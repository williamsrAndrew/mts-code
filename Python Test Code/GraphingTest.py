import matplotlib.pyplot as plt 
import matplotlib.animation as animation
import numpy as np

def data_gen():
	t = data_gen.t
	count = 0
	for i in range(0,1000):
		t += 0.05
		yield t, np.sin(t * np.pi * 2) * np.exp(-t/ 10)
data_gen.t = 0

fig, ax = plt.subplots()
line, = ax.plot([], [], lw=2)
ax.set_ylim(-1.1, 1.1)
ax.set_xlim(0,5)
ax.grid()

xdata, ydata = [], []

def run(data):
	t, y = data
	xdata.append(t)
	ydata.append(y)
	xmin, xmax = ax.get_xlim()
	if t >= xmax:
		ax.set_xlim(xmin, 2*xmax)
		ax.figure.canvas.draw()
	line.set_data(xdata, ydata)
	return line,

ani = animation.FuncAnimation(fig, run, data_gen, blit = False, interval = 10, repeat = False)
plt.show
