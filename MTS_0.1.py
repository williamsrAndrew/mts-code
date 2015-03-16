## Motor Test Stand: Python Code with GUI ##
## Andrew Williams, Tim McDonald, Paulo Mavungo, Sam Roark
## Revised 3/16/2015

import serial
import time
from Tkinter import Tk, BOTH, RIGHT, RAISED, Listbox, END, LEFT
from ttk import Frame, Button, Style

ser = serial.Serial(baudrate = 115200, timeout = 0.5)
hs_bit = b'p'		# Handshake bit


class Motor(Frame):

	def __init__(self, parent):
		Frame.__init__(self, parent)
		self.parent = parent
		self.initUI()

	def initUI(self):
		self.parent.title("Motor Test")
		self.style = Style()
		# self.style.theme_use("clam")

		global lb
		lb = Listbox(self)
		lb.pack(fill = BOTH, expand = 1)

		frame = Frame(self, relief = RAISED, borderwidth = 1)
		frame.pack(fill = BOTH, expand = 1)

		self.pack(fill = BOTH, expand = 1)

		closeButton = Button(self, text = 'Close', command = self.quit)
		closeButton.pack(side = RIGHT, padx = 5, pady = 5)
		test1Button = Button(self, text = 'Test 1', command = self.runTest1)
		test1Button.pack(side = RIGHT)
		test2Button = Button(self, text = 'Test 2', command = self.runTest2)
		test2Button.pack(side = RIGHT)

		clearButton = Button(self, text = 'Clear', command = self.clearTxt)
		clearButton.pack(side = LEFT)

	def runTest1(self):
		lb.insert(END, 'Test 1 Complete')

	def runTest2(self):
		lb.insert(END, 'Test 2 Complete')

	def clearTxt(self):
		lb.delete(0, END)

# Find and initalize the port to run
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

# Complete handshake with the TIVA
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

# Close open serial port
def closePort():
	# Properly close open serial port
	ser.close()
	print('Serial {} closed'.format(ser.name))

# Ask user for option to run
def getChoice():
	while True:
		option = str(input('Enter the profile that you want to run: '))
		if (option == '1' or option == '2' or option == '3' or option == '4'):
			ser.write(option.encode('utf-8'))
			break

# Read in and print data
def getData():
	index = 0
	data = []
	line = ser.read(1)

	# Flush buffer of handshake bits
	while line == hs_bit:
		line = ser.read(1)

	# Only iterate 20 times
	while index < 20:
		# Check to see if read line is comma or newline
		if line != b',' and line != b'\n':
			data.append(line)

		# If line is comma or newline, convert hex data to dec and reset data
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

# def main():
# 	initPort()
# 	handshake(hs_bit)
# 	getChoice()
# 	getData()
# 	closePort()

def main():
	root = Tk()
	root.geometry("400x300")
	app = Motor(root)
	root.mainloop()

if __name__ == '__main__':
	main()
