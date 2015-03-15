import time
import random

filetime = time.localtime()

f = open('test_{}_{}_{}_{}{}{}.csv'.format(filetime[1], filetime[2], filetime[0], filetime[3], filetime[4], filetime[5]), 'w')

def genRand(n):
	nums = []
	for i in range(0,6):
		nums.append(random.random()*n)
	return nums


starttime = time.time()
for i in range(1,11):
	data = genRand(i)
	f.write(str(time.time() - starttime))
	f.write(',')
	for j in data:
		f.write(str(j))
		f.write(',')
	f.write('\n')
	time.sleep(0.01)

f.close()