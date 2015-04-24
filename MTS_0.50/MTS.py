## Motor Test Stand: Python Code with GUI ##
## Andrew Williams, Tim McDonald, Paulo Mavungo, Sam Roark
## Revised 4/23/2015

####### Import ########
import serial, time, tkinter
import multiprocessing as mp
from tkinter import Tk
import matplotlib.pyplot as plt
import MTS_GUI as gui

# Draw graphzzzzz
def drawGraph(dataQ, voltBool, currBool, thrustBool, torqueBool):
	# Number of graphs to draw
	graphNum = voltBool + currBool + thrustBool + torqueBool

	# Add things for the first subplot
	index = 1
	if voltBool:
		plt.subplot(graphNum, 1, index)
		plt.ylabel('Voltage')
		plt.ion()
		index += 1
	if currBool:
		plt.subplot(graphNum, 1, index)
		plt.ylabel('Current')
		plt.ion()
		index += 1
	if thrustBool:
		plt.subplot(graphNum, 1, index)
		plt.ylabel('Thrust')
		plt.ion()
		index += 1
	if torqueBool:
		plt.subplot(graphNum, 1, index)
		plt.ylabel('Torque')
		plt.ion()

	plt.xlabel('Time (ms)')
	plt.show()

	# Create new lists for time and incoming data
	t, data = [], []
	for i in range(0, graphNum):
		data.append([])

	# Create buffer for incoming data and boolean to terminate while loop
	dataSet = []
	didGet = False

	# Run the graph generation loop
	while True:
		while not dataQ.empty():
			dataSet = dataQ.get()
			if dataSet == False:
				break
			t.append(dataSet[0])
			if voltBool:
				data.append(dataset[3])
			if currBool:
				data.append(dataset[4])
			if thrustBool:
				data.append(dataset[5])
			if torqueBool:
				data.append(dataset[6])
			didGet = True
		if didGet:
			index = 1
			if voltBool:
				plt.subplot(graphNum, 1, index)
				plt.plot(t, data[index - 1], 'r.-')
				plt.draw()
				index += 1
			if currBool:
				plt.subplot(graphNum, 1, index)
				plt.plot(t, data[index - 1], 'b.-')
				plt.draw()
				index += 1
			if thrustBool:
				plt.subplot(graphNum, 1, index)
				plt.plot(t, data[index - 1], 'g.-')
				plt.draw()
				index += 1
			if torqueBool:
				plt.subplot(graphNum, 1, index)
				plt.plot(t, data[index - 1], 'y.-')
				plt.draw()

			didGet = False

		# Make sure to wait at least a second before going through again
		time.sleep(1)
		if dataSet == False:
			break
	plt.show(block = True)
	return


def main():
	root = Tk()
	root.geometry("700x400")
	app = gui.Motor(root)
	root.mainloop()


if __name__ == '__main__':
	main()
