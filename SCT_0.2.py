## Motor Test Stand: Python Test Code ##
## Andrew Williams, Tim McDonald, Paulo Mavungo, Sam Roark
## Revised 3/15/2015

import serial
import time

ser = serial.Serial(baudrate = 115200, timeout = 0.5)
hs_bit = b'x'		# Handshake bit


def initPort():
	# Initial port values
	port = 0
	ser.port = port
	print("Trying {}...".format(ser.name))

	# Figure out which port to open
	while port < 9:
		try: 
			# In case the port wasn't properly closed before
			ser.close()
			# Open port
			ser.open()
			print("Opened serial port on {}".format(ser.name))
			break
		except serial.SerialException:
			ser.close()
			port += 1
			ser.port = port
			print("Trying {}...".format(ser.name))

def handshake(n):
	# Send and wait for handshake before proceeding
	while True:
		try:
			send = n
			# Wait until there is something I can read
			while ser.inWaiting() == 0:
				ser.write(send)
				# print('Wrote handshake')
				time.sleep(0.1) # sleep for 100 ms
				continue
			line = ser.read(1)
			print('Received text: {}'.format(line))
			if (line == send):
				print('Handshake complete')
				break
		except ser.SerialTimeoutException:
			print('Nothing received...')

def closePort():
	# Properly close open serial port
	ser.close()
	print('Serial {} closed'.format(ser.name))

def main():
	initPort()
	handshake(hs_bit)

	while True:
		option = str(input('Enter the profile that you want to run: '))
		if (option == '1' or option == '2' or option == '3' or option == '4'):
			ser.write(option.encode('utf-8'))
			break

	index = 0
	data = []
	line = ser.read(1)
	while line == hs_bit:
		line = ser.read(1)
	while index < 20:

		# Check to see if read line is comma or newline
		if line != b',' and line != b'\n':
			data.append(line)
			# print("got these weird chars")

		else:
			try:
				# Convert bytes to usable int
				x = int.from_bytes(data[0] + data[1], byteorder = 'big')
				print(x, " : ",data[0] + data[1])
				data = []
			except ValueError:
				print(line)
		# Read next line
		line = ser.read(1)
		index += 1
	closePort()



if __name__ == '__main__':
	main()
