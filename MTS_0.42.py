## Motor Test Stand: Python Code with GUI ##
## Andrew Williams, Tim McDonald, Paulo Mavungo, Sam Roark
## Revised 3/28/2015

####### Import ########
import serial
import time
import tkinter
from tkinter import Tk, BOTH, RIGHT, RAISED, Listbox, END, LEFT, TOP, BOTTOM, X, StringVar, OptionMenu, Scale, HORIZONTAL, Label, PhotoImage
from tkinter.ttk import Frame, Button, Style
# import numpy as np
# import threading

####### Init variables #######
ser = serial.Serial(baudrate = 115200, timeout = 0.5)
hs_byte = b'p'		# Handshake byte
termination_byte = b'~'
inits = 0
# global end_run
# end_run = False



class Motor(Frame):

	def __init__(self, parent):
		Frame.__init__(self, parent)
		self.pickedOption = StringVar(parent)
		self.parent = parent
		self.initUI()

	def initUI(self):
		self.parent.title("Motor Test")
		self.style = Style()

		
		# Start at top, have dropdown and sliders
		top_frame = Frame(self)
		top_frame.pack(fill = BOTH, expand = 0)
		self.sliderLabel = Label(top_frame, text = "Choose test\nand options", justify = LEFT)
		self.sliderLabel.pack(side = LEFT, padx = 3, pady = 5)

		# Create dropdown menu
		optionList = ('1', '2', '3', '4')
		self.pickedOption.set(optionList[0])
		self.pickedOption.trace("w", self.pickChange)
		
		options = OptionMenu(top_frame, self.pickedOption, *optionList)
		options.pack(side = LEFT, padx = 3, pady = 5)

		# Make sliders
		self.slider1 = Scale(top_frame, from_=0, to_=60, orient = HORIZONTAL, label = "Startup Time")
		self.slider1.pack(side = LEFT, padx = 3, pady = 5)
		self.slider2 = Scale(top_frame, from_=0, to_=60, orient = HORIZONTAL, label = "Hold Time")
		self.slider2.pack(side = LEFT, padx = 3, pady = 5)
		self.slider3 = Scale(top_frame, from_=0, to_=60, orient = HORIZONTAL, label = "Cool-Down Time")
		self.slider3.pack(side = LEFT, padx = 3, pady = 5)
		self.slider4 = Scale(top_frame, from_=1, to_=95, orient = HORIZONTAL, label = "Max Power %")
		self.slider4.pack(side = LEFT, padx = 3, pady = 5)
		# TU = PhotoImage(file = "tulogo.gif")
		# TUimage = Label(top_frame, image = TU)
		# TUimage.pack(side = RIGHT, padx = 3)
		


		# Next down is a Listbox that other functions can access
		global lb
		lb = Listbox(self)
		lb.pack(fill = BOTH, expand = 1)

		self.pack(fill = BOTH, expand = 1)

		# Next have Buttons
		closeButton = Button(self, text = 'Close', command = self.quit)
		closeButton.pack(side = RIGHT, padx = 5, pady = 5)
		self.testButton = Button(self, text = 'Run Test', command = self.runTest)
		self.testButton.pack(side = RIGHT)
		clearButton = Button(self, text = 'Clear', command = self.clearTxt)
		clearButton.pack(side = LEFT)
		initButton = Button(self, text = 'Init Port', command = self.initPort)
		initButton.pack(side = LEFT)

	def runTest(self):

		if ser.name == None:
			lb.insert(END, 'Open comunication on a port to run a test')
			self.updateView()
			return
		try:
			testNum = self.pickedOption.get()
			lb.insert(END, 'Running test {}...'.format(testNum))
			self.updateView()
			ser.write(testNum.encode('utf-8'))

			if testNum == '1':
				time.sleep(0.001)
				ser.write(b'{}'.format(self.slider1.get()))
				time.sleep(0.001)
				ser.write(b'{}'.format(self.slider2.get()))
				time.sleep(0.001)
				ser.write((b'{}'.format(self.slider3.get()))
				time.sleep(0.001)
				ser.write((b'{}'.format(self.slider4.get()))
			elif testNum == '2':
				time.sleep(0.001)
				ser.write((b'{}'.format(self.slider1.get()))
				time.sleep(0.001)
				ser.write((b'{}'.format(self.slider2.get()))
				time.sleep(0.001)
				ser.write((b'{}'.format(self.slider3.get()))

			# Timer
			curTime = time.time()
			dataset = self.getData()
			self.updateView()

			# Create the file
			self.makeFile(dataset)
			lb.insert(END, 'Test {} complete, file created in "Test Results" folder'.format(testNum))
			self.updateView()

		except serial.serialutil.SerialException:
			lb.insert(END, 'Could not run test 4')
			self.updateView()

	# Run when the optionsMenu changes
	def pickChange(self, *args):
		# print('{}'.format(self.pickedOption.get()))
		option = self.pickedOption.get()
		if option == '1':
			self.slider1.config(from_=0, to_=60, orient = HORIZONTAL, label = "Startup Time")
			self.slider1.pack(side = LEFT, padx = 3, pady = 5)
			self.slider2.config(from_=0, to_=60, orient = HORIZONTAL, label = "Hold Time")
			self.slider2.pack(side = LEFT, padx = 3, pady = 5)
			self.slider3.config(from_=0, to_=60, orient = HORIZONTAL, label = "Cool-Down Time")
			self.slider3.pack(side = LEFT, padx = 3, pady = 5)
			self.slider4.config(from_=1, to_=95, orient = HORIZONTAL, label = "Max Power %")
			self.slider4.pack(side = LEFT, padx = 3, pady = 5)
			return
		elif option == '2':
			self.slider1.config(from_=0, to_=60, orient = HORIZONTAL, label = "Period")
			self.slider1.pack(side = LEFT, padx = 3, pady = 5)
			self.slider2.config(from_=1, to_=95, orient = HORIZONTAL, label = "Max Power %")
			self.slider2.pack(side = LEFT, padx = 3, pady = 5)
			self.slider3.config(from_=0, to_=60, orient = HORIZONTAL, label = "Run Time")
			self.slider3.pack(side = LEFT, padx = 3, pady = 5)
			# self.slider4.config(from_=0, to_=95, orient = HORIZONTAL, label = "Max Power %")
			self.slider4.pack_forget()
			return
		elif option == '3':
			# self.slider1.config(from_=0, to_=255, orient = HORIZONTAL, label = "Period")
			self.slider1.pack_forget()
			# self.slider2.config(from_=0, to_=95, orient = HORIZONTAL, label = "Max Power %")
			self.slider2.pack_forget()
			# self.slider3.config(from_=0, to_=255, orient = HORIZONTAL, label = "Run Time")
			self.slider3.pack_forget()
			# self.slider4.config(from_=0, to_=95, orient = HORIZONTAL, label = "Max Power %")
			self.slider4.pack_forget()
			return
		elif option == '4':
			# self.slider1.config(from_=0, to_=255, orient = HORIZONTAL, label = "Period")
			self.slider1.pack_forget()
			# self.slider2.config(from_=0, to_=95, orient = HORIZONTAL, label = "Max Power %")
			self.slider2.pack_forget()
			# self.slider3.config(from_=0, to_=255, orient = HORIZONTAL, label = "Run Time")
			self.slider3.pack_forget()
			# self.slider4.config(from_=0, to_=95, orient = HORIZONTAL, label = "Max Power %")
			self.slider4.pack_forget()
			return


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
		while port < 21:
			try: 
				# In case the port wasn't properly closed before
				ser.close()
				# Open port
				ser.open()
				lb.insert(END, "Opened serial port on {}".format(ser.name))
				self.updateView()
				break
			except serial.serialutil.SerialException:
				if port == 20:
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

		# self.testButton.config(text = 'Stop Test', command = self.stopRun)
		buf = [] # use as buffer
		byte = ser.read() # Read in first byte
		ender = 0
		index = 0
		loops = 0
		# global end_run
		# end_run = False

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
					break
			else: ender = 0

			if index  == 3 or index == 4 or index == 8 or index == 10 or index == 12 or index == 14 :
				buf.append(byte)
				try:
					# Convert bytes to usable int
					data = int.from_bytes(b''.join(buf), byteorder = 'big')
					dataset[datasetNum].append(data)

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

					# listLine = '' # Clear listLine
					buf = [] # Clear buffer

					index = 0
					datasetNum = 0
					loops += 1

					# Print something every so many cycles
					if loops % 200 == 0:
						lb.insert(END, 'Running, {} cycles have completed'.format(loops))
						self.updateView()

					# If end_run is true, end the run
					# if end_run:
					# 	break


				except ValueError:
					print(byte)

			else:
				buf.append(byte)
				index += 1

			# Read next byte
			byte = ser.read()
			
		# Reset end_run and test button
		# end_run = False
		# self.testButton.config(text = 'Run Test', command = self.runTest)
		return dataset

	# Create file from the collected data
	def makeFile(self, dataset):
		# The incoming dataset consists of raw data which needs to be
		# processed before it is written to the file
		filetime = time.localtime()
		f = open('Test Results\\test_{}_{}_{}_{}{}{}.csv'.format(filetime[1], filetime[2], filetime[0], filetime[3], filetime[4], filetime[5]), 'w')

		# Write the title line to the file so we know where each data set is
		f.write('Time, Input, RPM, Voltage, Current, Thrust, Torque\n')

		# Append data to file
		dataLine =''
		for i in range(0, len(dataset[0])):
			# Append values to dataLine
			dataLine += str(dataset[0][i]) + ','	# Time - absolute
			dataLine += str(dataset[1][i]) + ','	# Input - absolute
			dataLine += str(dataset[2][i]) + ','	# RPM - absolute
			dataLine += str(self.bitToVolt(dataset[3][i])) + ','	# Voltage - convert
			dataLine += str(self.bitToCurr(dataset[4][i])) + ','	# Current - convert
			dataLine += str(self.bitToThrust(dataset[5][i])) + ','	# Thrust - convert
			dataLine += str(self.bitToTorque(dataset[6][i])) + '\n'	# Torque - convert
			f.write(dataLine)
			dataLine = ''

		f.close()
		return


	# Stop running the getData program
	def stopRun(self):
		end_run = True
		return

	# Convert incoming voltage to actual voltage
	def bitToVolt(self, value):
		# Slope 0.001831099125
		# Intercept -36.61463847
		return (0.001831099125 * value) - 36.61463847

	# Convert incoming voltage to current
	def bitToCurr(self, value):
		# Slope 0.002599283332
		# Intercept -52.08362823
		return (0.002599283332 * value) - 52.08362823

	# Convert incoming voltage to thrust
	def bitToThrust(self, value):
		# Slope 0.1443259722
		# Intercept -2905.095166
		return (0.1443259722 * value) - 2905.095166

	# Convert incoming voltage to torque
	def bitToTorque(self, value):
		# Slope = 0.4144183355
		# Intercept = -7893.891642
		return (0.4144183355 * value) - 7893.891642

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
	root.geometry("700x400")
	app = Motor(root)
	root.mainloop()
	closePort()


if __name__ == '__main__':
	main()
