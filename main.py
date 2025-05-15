import sensor, image, time, math, pyb
from image import SEARCH_EX, SEARCH_DS
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time=2000)
clock = time.clock()
COMM_MODE = 'BINARY'
templates = {1: image.Image("1.pgm"), 2: image.Image("2.pgm"),
			 3: image.Image("3.pgm"), 4: image.Image("4.pgm"),
			 5: image.Image("5.pgm"), 6: image.Image("6.pgm"),
			 7: image.Image("7.pgm"), 8: image.Image("8.pgm")}
color_thresholds = [
	(0, 0, 4, 4, 0, 0),
	(46, 60, -65, -22, -68, 73),
	(39, 50, 11, 33, 26, 49)
]
color_names = ["Black", "Green", "Orange"]
CUBE_WIDTH_CM = 5.0
FOCAL_LENGTH = 1600
uart = pyb.UART(3, 9600)
def calculate_distance(pixel_width):
	return (CUBE_WIDTH_CM * FOCAL_LENGTH) / pixel_width
def find_cube(blobs):
	for blob in blobs:
		if 0.7 < blob.w()/blob.h() < 1.3 and blob.area() > 500:
			img_center_x = sensor.width() // 2
			img_center_y = sensor.height() // 2
			center_x = blob.cx() - img_center_x
			center_y = img_center_y - blob.cy()
			angle = blob.rotation() if hasattr(blob, 'rotation') else 0
			return {
				'blob': blob,
				'center_x': center_x,
				'center_y': center_y,
				'angle': angle,
				'distance': calculate_distance(blob.w())
			}
	return None
def send_data(mode, value1, value2=0, value3=0, value4=0):
	if COMM_MODE == 'ASCII':
		if mode == 'T':
			packet = "T%d\n" % value1
		elif mode == 'C':
			packet = "C%d%04d\n" % (value1, int(value2*10))
		elif mode == 'P':
			packet = "P%d%c%03d%c%03d%c%03d\n" % (
				value1,
				'+' if value2 >=0 else '-', abs(value2),
				'+' if value3 >=0 else '-', abs(value3),
				'+' if value4 >=0 else '-', abs(value4))
		uart.write(packet)
		print("ASCII Sent:", packet.strip())
	else:
		if mode == 'P':
			x = int(value2)
			y = int(value3)
			openmv_data = bytearray([
				0xA3, 0xB3,
				value1 & 0xFF,
				(x >> 8) & 0xFF, x & 0xFF,
				(y >> 8) & 0xFF, y & 0xFF,
				0xC3
			])
			uart.write(openmv_data)
			print("BIN Sent:", openmv_data)
def print_cube_info(color_name, cube_info):
	print("=== Cube Info ===")
	print("Color:", color_name)
	print("Center (X,Y): (%d, %d)" % (cube_info['center_x'], cube_info['center_y']))
	print("Distance: %.1f cm" % cube_info['distance'])
	print("Angle: %.1f°" % cube_info['angle'])
while True:
	clock.tick()
	img = sensor.snapshot()
	template_detected = False
	for tid, template in templates.items():
		r = img.find_template(template, 0.70, step=4, search=SEARCH_EX)
		if r:
			img.draw_rectangle(r, color=(255, 0, 0))
			template_detected = True
			print("Detected Template: ID=%d" % tid)
			send_data('T', tid)
			break
	if not template_detected:
		detected_cubes = []
		for i, threshold in enumerate(color_thresholds):
			blobs = img.find_blobs([threshold],
								 pixels_threshold=200,
								 area_threshold=200,
								 merge=True,
								 margin=10)
			cube_info = find_cube(blobs)
			if cube_info:
				detected_cubes.append((i, cube_info))
				blob = cube_info['blob']
				img.draw_rectangle(blob.rect(), color=(0, 255, 0))
				img.draw_cross(blob.cx(), blob.cy(), color=(0, 255, 0))
				img.draw_string(blob.x(), blob.y()-10,
							   "%s %.1fcm" % (color_names[i], cube_info['distance']),
							   color=(0, 255, 0))
				angle_rad = math.radians(cube_info['angle'])
				end_x = int(blob.cx() + 30 * math.cos(angle_rad))
				end_y = int(blob.cy() + 30 * math.sin(angle_rad))
				img.draw_line(blob.cx(), blob.cy(), end_x, end_y, color=(255, 0, 0))
		if detected_cubes:
			color_id, cube_info = max(detected_cubes, key=lambda x: x[1]['blob'].area())
			print_cube_info(color_names[color_id], cube_info)
			send_data('C', color_id, cube_info['distance'])
			send_data('P', color_id,
					 cube_info['center_x'],
					 cube_info['center_y'],
					 cube_info['angle'])
	img.draw_string(0, 0, "FPS:%.1f Mode:%s" % (clock.fps(), COMM_MODE),
				   color=(255, 255, 255))