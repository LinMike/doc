import matplotlib.pyplot as plt
import numpy as np

#plt.figure(figsize=(6,6), dpi=180)
plt.figure(1)
ax1=plt.subplot(211)
plt.scatter([1,3,5],[2,4,5],marker='v', s=50,color='r')
ax2=plt.subplot(212)
plt.plot([2,4,6], [7,9,15])

plt.figure(2)
x=np.arange(4)
y=np.array([15,20,18,25])
plt.bar(x,y)
plt.title('panel 2')
plt.figure(1)
plt.subplot(212)
plt.plot([7,8],[11,18])
ax1.set_title('figure 1')
ax2.set_title('figure 2')
plt.tight_layout()
plt.show()
