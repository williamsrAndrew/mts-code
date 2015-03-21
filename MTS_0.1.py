## Motor Test Stand: Python Code with GUI ##
## Andrew Williams, Tim McDonald, Paulo Mavungo, Sam Roark
## Revised 3/16/2015

import serial
import time
from tkinter import Tk, BOTH, RIGHT, RAISED, Listbox, END, LEFT
from tkinter.ttk import Frame, Button, Style

ser = serial.Serial(baudrate = 115200, timeout = 0.5)
hs_bit = b'p'		# Handshake bit


class Motor(Frame):

	def __init__(self, parent):
		Frame.__init__(self, parent)
		self.parent = parent
		self.initUI()
		self.initPort()
		self.handshake(hs_bit)

	def initUI(self):
		self.parent.title("Motor Test")
		self.style = Style()

		# Listbox that other functions can access
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
		ser.write('1'.encode('utf-8'))
		lb.insert(END, 'Running test 1...')
		self.getData()

	def runTest2(self):
		ser.write('2'.encode('utf-8'))
		lb.insert(END, 'Running test 2...')
		self.getData()

	def clearTxt(self):
		lb.delete(0, END)

	# Find and initalize the port to run
	def initPort(self):
		# Initial port values
		port = 0
		ser.port = port
		lb.insert(END, "Trying {}...".format(ser.name))

		# Figure out which port to open
		while port < 9:
			try: 
				# In case the port wasn't properly closed before
				ser.close()
				# Open port
				ser.open()
				lb.insert(END, "Opened serial port on {}".format(ser.name))
				break
			except serial.SerialException:
				ser.close()
				port += 1
				ser.port = port
				lb.insert(END, "Trying {}...".format(ser.name))
	# Complete handshake with the TIVA
	def handshake(self, n):
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
				lb.insert(END, 'Received text: {}'.format(line))
				if (line == send):
					lb.insert(END, 'Handshake complete')
					break
			except ser.SerialTimeoutException:
				lb.insert(END, 'Nothing received...')

	# Read in and print data
	def getData(self):
		index = 0
		buf = [] # use as buffer
		line = ser.read(1)

		# Flush buffer of handshake bits
		while line == hs_bit:
			line = ser.read(1)

		# Only iterate 20 times
		while index < 20:
			# Check to see if read line is comma or newline
			if line != b',' and line != b'\n':
				buf.append(line)

			# If line is comma or newline, convert hex data to dec and reset data
			elif line == b',':
				try:
					# Convert bytes to usable int
					x = int.from_bytes(buf[0] + buf[1], byteorder = 'big')
					# print(x, " : ",data[0] + data[1])
					lb.insert(END, x)
					buf = [] # Clear buffer
				except ValueError:
					print(line)
			# Read next line
			line = ser.read(1)
			index += 1

	# Quit out of window
	def quit(self):
		self.closePort()
		root.destroy()

	# Close open serial port
	def closePort(self):
		# Properly close open serial port
		ser.close()
		lb.insert(END, 'Serial {} closed'.format(ser.name))

def closePort():
	# Properly close open serial port
	ser.close()
	print('Serial {} closed'.format(ser.name))


def main():
	root = Tk()
	root.geometry("500x500")
	app = Motor(root)
	root.mainloop()
	closePort()


if __name__ == '__main__':
	main()
