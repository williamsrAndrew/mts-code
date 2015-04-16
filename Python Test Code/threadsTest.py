# import threading
import multiprocessing as mp
import matplotlib.pyplot as plt
import numpy as np
import time



def drawData(dataQ):
		plt.xlabel('Time (s)')
		plt.ylabel('Data')
		plt.ion()
		plt.show()
		t, data = [], []
		dataSet = []
		didGet = False
		while True:
			while not dataQ.empty():
				dataSet = dataQ.get()
				if dataSet == False:
					break
				t.append(dataSet[0])
				data.append(dataSet[1])
				didGet = True
			if didGet:
				plt.plot(t, data, 'b:')
				plt.draw()
				didGet = False
			time.sleep(1)
			if dataSet == False:
				break
		plt.show(block = True)
		return



def main():
	global dataset
	global end_run
	dataset = []
	end_run = False
	thread_sleep = 0.1
	cur_time = time.time()
	dataset.append([])
	dataset.append([])
	# dataset.append([])
	index = 0
	dataQ = mp.Queue()
	draw_job = mp.Process(target = drawData, args = (dataQ,))
	draw_job.start()

	while time.time() - cur_time < 10:
		dataQ.put([time.time() - cur_time, index])
		index += 1
		time.sleep(0.001)

	dataQ.put(False)
	# print (dataset)
	draw_job.join()

if __name__ == '__main__':
	main()








