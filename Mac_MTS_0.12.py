## Motor Test Stand: Python Code with GUI ##
## Andrew Williams, Tim McDonald, Paulo Mavungo, Sam Roark
## Revised 3/21/2015

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
		# self.handshake(hs_bit)

	def initUI(self):
		self.parent.title("Motor Test")
		self.style = Style()

		# frame = Frame(self, relief = RAISED, borderwidth = 2)
		# frame.pack(fill = BOTH, expand = 1)

		# Listbox that other functions can access
		global lb
		lb = Listbox(self)
		lb.pack(fill = BOTH, expand = 1)

		self.pack(fill = BOTH, expand = 1)

		closeButton = Button(self, text = 'Close', command = self.quit)
		closeButton.pack(side = RIGHT, padx = 5, pady = 5)
		test1Button = Button(self, text = 'Test 1', command = self.runTest1)
		test1Button.pack(side = RIGHT)
		test2Button = Button(self, text = 'Test 2', command = self.runTest2)
		test2Button.pack(side = RIGHT)
		test3Button = Button(self, text = 'Test 3', command = self.runTest3)
		test3Button.pack(side = RIGHT)
		test4Button = Button(self, text = 'Test 4', command = self.runTest4)
		test4Button.pack(side = RIGHT)

		clearButton = Button(self, text = 'Clear', command = self.clearTxt)
		clearButton.pack(side = LEFT)
		initButton = Button(self, text = 'Init Port', command = self.initPort)
		initButton.pack(side = LEFT)

	def runTest1(self):
		try:
			ser.write('1'.encode('utf-8'))
			lb.insert(END, 'Running test 1...')
			lb.yview(END)
			self.getData(100)
		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 1')
			lb.yview(END)

	def runTest2(self):
		try:
			ser.write('2'.encode('utf-8'))
			lb.insert(END, 'Running test 2...')
			lb.yview(END)
			self.getData(100)
		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 2')
			lb.yview(END)

	def runTest3(self):
		try:
			ser.write('3'.encode('utf-8'))
			lb.insert(END, 'Running test 3...')
			lb.yview(END)
			self.getData(100)
		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 3')
			lb.yview(END)

	def runTest4(self):
		try:
			ser.write('4'.encode('utf-8'))
			lb.insert(END, 'Running test 4...')
			lb.yview(END)
			self.getData(100)
		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 4')
			lb.yview(END)

	def clearTxt(self):
		lb.delete(0, END)

	# Find and initalize the port to run
	def initPort(self):
		# Initial port values
		port = 0
		ser.port = port
		lb.insert(END, "Trying {}...".format(ser.name))
		lb.yview(END)

		# Figure out which port to open
		while port < 10:
			try: 
				# In case the port wasn't properly closed before
				ser.close()
				# Open port
				ser.open()
				lb.insert(END, "Opened serial port on {}".format(ser.name))
				lb.yview(END)
				break
			except serial.SerialException:
				if port == 9:
					lb.insert(END, "Could not open a port, try again")
					lb.yview(END)
					break
				ser.close()
				port += 1
				ser.port = port
				lb.insert(END, "Trying {}...".format(ser.name))
				lb.yview(END)

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
	def getData(self, cycles):
		index = 0
		buf = bytearray() # use as buffer
		line = ser.read()

		# Flush buffer of handshake bits
		while line == hs_bit:
			line = ser.read()

		# Only iterate 20 times
		while index < cycles:
			# Check to see if read line is comma or newline
			if line != b',' and line != b'\n':
				buf.append(line)

			# If line is comma or newline, convert hex data to dec and reset data
			elif line == b',':
				try:
					# Convert bytes to usable int
					data = int.from_bytes(buf, byteorder = 'big')

					'''
					Debug Statement
					'''
					lb.insert(END, data)

					buf = bytearray() # Clear buffer

				except ValueError:
					print(line)
			# Read next line
			line = ser.read(1)
			index += 1

	# Quit out of window
	def quit(self):
		self.closePort()
		# time.sleep(1)
		Frame.quit(self)

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
	root.geometry("600x400")
	app = Motor(root)
	root.mainloop()
	# closePort()


if __name__ == '__main__':
	main()
