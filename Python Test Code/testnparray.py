import time

curTime = time.time()

data = []
for i in range(0,7):
	data.append([])

for i in range(0,100000):
	# if i == 999900:
	# 	curTime = time.time()
	for j in range(0,7):
		data[j].append(i)

print (time.time() - curTime)