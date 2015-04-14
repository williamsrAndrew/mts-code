## Motor Test Stand: Python Code with GUI ##
## Andrew Williams, Tim McDonald, Paulo Mavungo, Sam Roark
## Revised 3/28/2015

####### Import ########
import serial
import time
from tkinter import Tk, BOTH, RIGHT, RAISED, Listbox, END, LEFT
from tkinter.ttk import Frame, Button, Style
import numpy as np

####### Init variables #######
ser = serial.Serial(baudrate = 115200, timeout = 0.5)
hs_byte = b'p'		# Handshake byte
termination_byte = b'~'
inits = 0


class Motor(Frame):

	def __init__(self, parent):
		Frame.__init__(self, parent)
		self.parent = parent
		self.initUI()

	def initUI(self):
		self.parent.title("Motor Test")
		self.style = Style()
		# Listbox that other functions can access
		global lb
		lb = Listbox(self)
		lb.pack(fill = BOTH, expand = 1)

		self.pack(fill = BOTH, expand = 1)

		# Buttons
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
		# Check if comms has been initialized
		if ser.name == None:
			lb.insert(END, 'Open comunication on a port to run a test')
			self.updateView()
			return
		try:
			# Write a 1 to run test 1
			ser.write('1'.encode('utf-8'))
			lb.insert(END, 'Running test 1...')
			self.updateView()
			self.getData()
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
			self.getData()
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
			self.getData()
		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 3')
			self.updateView()

	def runTest4(self):

		if ser.name == None:
			lb.insert(END, 'Open comunication on a port to run a test')
			self.updateView()
			return
		try:
			lb.insert(END, 'Running test 4...')
			self.updateView()
			ser.write('4'.encode('utf-8'))

			# Timer
			curTime = time.time()
			dataset = self.getData()
			self.updateView()

			# Create the file
			self.makeFile(dataset)
			lb.insert(END, 'Test 4 complete, file created in "Test Results" folder')
			self.updateView()

		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 4')
			self.updateView()

	# Clear the text from the listbox
	def clearTxt(self):
		lb.delete(0, END)
		return

	# Find and initalize the port to run
	def initPort(self):
		if ser.port != None:
			lb.insert(END, 'Comms already open on port {}, closing port'.format(ser.name))
			self.updateView()
			ser.close()
		# Initial port values
		port = 0
		ser.port = port
		lb.insert(END, "Trying {}...".format(ser.name))
		self.updateView()

		# Figure out which port to open
		while port < 15:
			try: 
				# In case the port wasn't properly closed before
				ser.close()
				# Open port
				ser.open()
				lb.insert(END, "Opened serial port on {}".format(ser.name))
				self.updateView()
				break
			except serial.serialutil.SerialException:
				if port == 14:
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
		lb.insert(END, "If blue light on TIVA is still blinking, press Init Port again")
		self.updateView()

	def handshake(self):
		# Perform handshake with TIVA
		while True:
			try:
				send = hs_byte
				# Wait until there is something I can read
				#while ser.inWaiting() == 0:
				for i in range(0,51):
					ser.write(send)
					time.sleep(0.1) # sleep for 100 ms

					if ser.inWaiting() >= 1:
						break

					if i == 50:
						lb.insert(END, 'No handshake byte received...')
						self.updateView()
						return
					elif i % 10 == 0:
						lb.insert(END, '.')
						self.updateView()
					continue

				byte = ser.read()
				lb.insert(END, 'Received text: {}'.format(byte))
				self.updateView()
				if (byte == send):
					lb.insert(END, 'Handshake complete')
					self.updateView()
					break
			except serial.SerialTimeoutException:
				lb.insert(END, 'Nothing received...')
				self.updateView()

	# Read in and print data
	def getData(self):

		# filetime = time.localtime()
		# f = open('Test Results\\test_{}_{}_{}_{}{}{}.csv'.format(filetime[1], filetime[2], filetime[0], filetime[3], filetime[4], filetime[5]), 'w')

		# # Write the title line to the file so we know where each data set is
		# line1 = 'Time, Input, RPM, Voltage, Current, Thrust, Torque\n'
		# lb.insert(END, line1)
		# self.updateView()
		# f.write(line1)

		# total_bytes = 0
		buf = [] # use as buffer
		# listLine = ''
		byte = ser.read() # Read in first byte
		# total_bytes += 1 # Debug
		ender = 0
		index = 0
		loops = 0

		# Create list for data
		dataset = []
		for i in range(0,7):
			dataset.append([])
		datasetNum = 0

		# Flush buffer of handshake bytes
		while byte == hs_byte:
			byte = ser.read()
			# total_bytes += 1 # Debug

		while True:
			# Check if byte is an ender byte
			if byte == termination_byte:
				ender += 1
				if ender == 4:
					lb.insert(END, 'Complete, now creating file...')
					self.updateView()
					break
			else: ender = 0

			if index  == 3 or index == 4 or index == 8 or index == 10 or index == 12 or index == 14 :
				buf.append(byte)
				try:
					# Convert bytes to usable int
					data = int.from_bytes(b''.join(buf), byteorder = 'big')
					dataset[datasetNum].append(data)

					# '''
					# Debug Statement
					# '''
					# listLine += str(data)
					# listLine += ', '

					buf = [] # Clear buffer
					index += 1
					datasetNum += 1

				except ValueError:
					print(byte)

			elif index == 16:
				buf.append(byte)
				try:
					# Convert bytes to usable int
					data = int.from_bytes(b''.join(buf), byteorder = 'big')
					dataset[datasetNum].append(data)
					if loops % 100 == 0:
						lb.insert(END, 'working: elapsed time # {}'.format(loops/100))
						self.updateView()

					# '''
					# Debug Statement
					# '''
					# listLine += str(data)
					# lb.insert(END, listLine)
					# self.updateView()
					# f.write(listLine)
					# f.write('\n')

					# listLine = '' # Clear listLine
					buf = [] # Clear buffer

					index = 0
					datasetNum = 0
					loops += 1

				except ValueError:
					print(byte)

			else:
				buf.append(byte)
				index += 1

			# Read next byte
			byte = ser.read()


			# print(byte)
			# total_bytes += 1
		# f.close()
		return dataset

	# Create file from the collected data
	def makeFile(self, dataset):
		# The incoming dataset consists of raw data which needs to be
		# processed before it is written to the file
		filetime = time.localtime()
		f = open('Test Results\\test_{}_{}_{}_{}{}{}.csv'.format(filetime[1], filetime[2], filetime[0], filetime[3], filetime[4], filetime[5]), 'w')

		# Write the title line to the file so we know where each data set is
		f.write('Time, Input, RPM, Thrust, Torque, Voltage, Current\n')

		# Append data to file
		dataLine =''
		for i in range(0, len(dataset[0])):
			# Append values to dataLine
			dataLine += str(dataset[0][i]) + ','	# Time - absolute
			dataLine += str(dataset[1][i]) + ','	# Input - absolute
			dataLine += str(dataset[2][i]) + ','	# RPM - absolute
			dataLine += str(self.voltToThrust(self.voltageValue(dataset[3][i]))) + ','	# Thrust - convert
			dataLine += str(self.voltToTorque(self.voltageValue(dataset[4][i]))) + ','	# Torque - convert
			dataLine += str(self.voltToVolt(self.voltageValue(dataset[5][i]))) + ','	# Voltage - convert
			dataLine += str(self.voltToCurr(self.voltageValue(dataset[6][i]))) + '\n'	# Current - convert
			# Write compiled data to file
			f.write(dataLine)
			dataLine = '' # Clear line
		f.close()
		return


	# Convert incoming decimal values to voltages
	def voltageValue(self, value):
		# Each +1 bit = +0.002 V
		# Offset of -0.0036
		return ((0.002 * value) - 0.0036)

	# Convert incoming voltage to actual voltage
	def voltToVolt(self, voltage):
		# Obtained from empirical data
		return 14.86 * voltage - 36.408

	# Convert incoming voltage to current
	def voltToCurr(self, voltage):
		# Obtained from empirical data
		return 18.835 * voltage - 47.46

	# Convert incoming voltage to thrust
	def voltToThrust(self, voltage):
		# Slope obtained from data sheet
		# Offset obtained empirically
		return 0.514112 * (voltage - 2.48)

	# Convert incoming voltage to torque
	def voltToTorque(self, voltage):
		# Putting 5V on the sensor
		# 2.1821 mV/V
		# Slope obtained from data sheet
		# Offset obtained empirically
		return 0.000546 * (voltage - 2.34)

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
		if ser.port != None:
			ser.flush()
			ser.close()
			lb.insert(END, 'Serial {} closed'.format(ser.name))
		

def closePort():
	# Properly close open serial port
	if ser.port != None:
			ser.flush()
			ser.close()
			lb.insert(END, 'Serial {} closed'.format(ser.name))

def main():
	root = Tk()
	root.geometry("600x400")
	app = Motor(root)
	root.mainloop()
	closePort()


if __name__ == '__main__':
	main()
