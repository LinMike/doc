import matplotlib.pyplot as plt
import numpy as np
import thread
import ctypes
import threading
import cv2

def showimg(file_path):
	img = cv2.imread(file_path)
	cv2.imshow('img', img)
	cv2.waitKey(0)
	print 'load test image'#, img

def drawData(a, b, c):
# 	a = [[1,2,3,4,5,6],[1,2,3,4,5,6]]
# 	b = [[1,2,3,4,5,6],[1,2,3,4,5,6]]
# 	c = [[1,2,3,4,5,6],[1,2,3,4,5,6]]
	print 'run draw data function'
	print 'a = ', a
	print 'b = ', b
	print 'c = ', c
# 	print 'python function current thread id = ', threading.current_thread().name
# 	plt.figure(figsize=(6,6), dpi=180)
	print 'before'
	plt.figure(1)
 	ax1=plt.subplot(211)
    	
#  	plt.scatter([1,3,5],[2,4,5],marker='v', s=50,color='r')# 	
	plt.scatter(a[0],a[1],marker='v', s=50,color='r')
	print 'done'
	ax2=plt.subplot(212)
	plt.plot(b[0], b[1])
    
	plt.figure(2)
	#x=np.arange(4)
	#y=np.array([15,20,18,25])
	plt.bar(c[0],c[1])
	plt.title('panel 2')
# 	plt.figure(1)
	#plt.subplot(212)
	#plt.plot([7,8],[11,18])
	ax1.set_title('figure 1')
	ax2.set_title('figure 2')
	plt.tight_layout()
	print 'show figures'
	plt.show()

def Hello(name):
# 	print 'python function current thread id = ', threading.current_thread().name
	print 'hello ', name
 	
def Add(a, b):
	return a+b
	
# if __name__ == '__main__':
# 	showimg()
# 	a = [[1,2,3,4,5,6],[1,2,3,4,5,6]]
# 	b = [[1,2,3,4,5,6],[1,2,3,4,5,6]]
# 	c = [[1,2,3,4,5,6],[1,2,3,4,5,6]]
# 	print 'run draw data function'
# 	print 'a = ', a
# 	print 'b = ', b
# 	print 'c = ', c
# # 	print 'python function current thread id = ', threading.current_thread().name
# # 	plt.figure(figsize=(6,6), dpi=180)
# 	print 'before'
# 	plt.figure(1)
#  	ax1=plt.subplot(211)
#        	
# #  	plt.scatter([1,3,5],[2,4,5],marker='v', s=50,color='r')# 	
# 	plt.scatter(a[0],a[1],marker='v', s=50,color='r')
# 	print 'done'
# 	ax2=plt.subplot(212)
# 	plt.plot(b[0], b[1])
#        
# 	plt.figure(2)
# 	#x=np.arange(4)
# 	#y=np.array([15,20,18,25])
# 	plt.bar(c[0],c[1])
# 	plt.title('panel 2')
# # 	plt.figure(1)
# 	#plt.subplot(212)
# 	#plt.plot([7,8],[11,18])
# 	ax1.set_title('figure 1')
# 	ax2.set_title('figure 2')
# 	plt.tight_layout()
# 	print 'show figures'
# 	plt.show()
# # 	drawData()
# # 	print 'main function'
# # 	showimg()
# # 	th = threading.Thread(target=drawData)
# # 	th.start()
# # 	th.join()
# 	print 'python main current thread id = ', threading.current_thread().name
	
	
# 	drawData()
# 	drawData([[1,2,3,4,5,6],[1,2,3,4,5,6]], [[1,2,3,4,5,6],[1,2,3,4,5,6]], [[1,2,3,4,5,6],[1,2,3,4,5,6]])
