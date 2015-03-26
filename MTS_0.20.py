## Motor Test Stand: Python Code with GUI ##
## Andrew Williams, Tim McDonald, Paulo Mavungo, Sam Roark
## Revised 3/21/2015

####### Import ########
import serial
import time
from tkinter import Tk, BOTH, RIGHT, RAISED, Listbox, END, LEFT
from tkinter.ttk import Frame, Button, Style

####### Init variables #######
ser = serial.Serial(baudrate = 115200, timeout = 0.5)
hs_bit = b'p'		# Handshake bit
inits = 0


class Motor(Frame):

	def __init__(self, parent):
		Frame.__init__(self, parent)
		self.parent = parent
		self.initUI()
		# self.initPort()
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
		# hsButton = Button(self, text = 'Handshake', command = self.handshake)
		# hsButton.pack(side = LEFT)

	def runTest1(self):
		if ser.name == None:
			lb.insert(END, 'Open comunication on a port to run a test')
			self.updateView()
			return
		try:
			ser.write('1'.encode('utf-8'))
			lb.insert(END, 'Running test 1...')
			self.updateView()
			self.getData(100)
		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 1')
			self.updateView()

	def runTest2(self):
		if ser.name == None:
			lb.insert(END, 'Open comunication on a port to run a test')
			self.updateView()
			return
		try:
			ser.write('2'.encode('utf-8'))
			lb.insert(END, 'Running test 2...')
			self.updateView()
			self.getData(100)
		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 2')
			self.updateView()

	def runTest3(self):
		if ser.name == None:
			lb.insert(END, 'Open comunication on a port to run a test')
			self.updateView()
			return
		try:
			ser.write('3'.encode('utf-8'))
			lb.insert(END, 'Running test 3...')
			self.updateView()
			self.getData(100)
		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 3')
			self.updateView()

	def runTest4(self):
		# Timer
		curTime = time.time()

		if ser.name == None:
			lb.insert(END, 'Open comunication on a port to run a test')
			self.updateView()
			return
		try:
			ser.write('4'.encode('utf-8'))
			lb.insert(END, 'Running test 4...')
			self.updateView()
			bits_read = self.getData(1000)
			print("Time: {}\nBits read: {}\nBits per second: {}".format(time.time() - curTime, bits_read, bits_read /(time.time() - curTime) ))
		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 4')
			self.updateView()

	def clearTxt(self):
		lb.delete(0, END)

	# Find and initalize the port to run
	def initPort(self):
		if ser.port != None:
			lb.insert(END, 'Comms already open on port {}'.format(ser.name))
			self.updateView()
			return
		# Initial port values
		port = 0
		ser.port = port
		lb.insert(END, "Trying {}...".format(ser.name))
		self.updateView()

		# Figure out which port to open
		while port < 10:
			try: 
				# In case the port wasn't properly closed before
				ser.close()
				# Open port
				ser.open()
				lb.insert(END, "Opened serial port on {}".format(ser.name))
				self.updateView()
				break
			except serial.serialutil.SerialException:
				if port == 9:
					lb.insert(END, "Could not open a port, try again")
					self.updateView()
					ser.port = None
					return
				ser.close()
				port += 1
				ser.port = port
				lb.insert(END, "Trying {}...".format(ser.name))
				self.updateView()
		# Perform handshake with TIVA
		self.handshake()

	def handshake(self):
		# Perform handshake with TIVA
		while True:
			try:
				send = hs_bit
				# Wait until there is something I can read
				while ser.inWaiting() == 0:
					ser.write(send)
					# print('Wrote handshake')
					time.sleep(0.1) # sleep for 100 ms
					continue
				bit = ser.read(1)
				lb.insert(END, 'Received text: {}'.format(bit))
				if (bit == send):
					lb.insert(END, 'Handshake complete')
					break
			except serial.SerialTimeoutException:
				lb.insert(END, 'Nothing received...')

	# Read in and print data
	def getData(self, cycles):
		index = 0
		total_bits = 0
		buf = [] # use as buffer
		listLine = ''
		bit = ser.read()
		total_bits += 1

		# Flush buffer of handshake bits
		while bit == hs_bit:
			bit = ser.read()
			total_bits += 1

		# Only iterate 20 times
		while index < cycles:
			# Check to see if read bit is comma or newline
			if bit != b',' and bit != b'\n':
				buf.append(bit)

			# If bit is comma or newline, convert hex data to dec and reset data
			elif bit == b',':
				try:
					# Convert bytes to usable int
					data = int.from_bytes(b''.join(buf), byteorder = 'big')

					'''
					Debug Statement
					'''
					listLine += str(data)
					listLine += ', '
					# lb.insert(END, data)
					# self.updateView()

					buf = [] # Clear buffer

				except ValueError:
					print(bit)
			# If bit is newline, do that same as comma except go to next line in file
			elif bit == b'\n':
				try:
					# Convert bytes to usable int
					data = int.from_bytes(b''.join(buf), byteorder = 'big')

					'''
					Debug Statement
					'''
					listLine += str(data)
					lb.insert(END, listLine)
					self.updateView()

					listLine = '' # Clear listLine
					buf = [] # Clear buffer
				except ValueError:
					print(bit)

			# Read next bit
			bit = ser.read()
			total_bits += 1
			index += 1
		return total_bits


	# Update the listbox 
	def updateView(self):
		lb.yview(END)
		lb.update_idletasks()

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
	closePort()


if __name__ == '__main__':
	main()
