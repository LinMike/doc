import numpy as np

#Robot 1
#X = [286, 314, 333, 351]
#X = [201, 220, 233, 246]
#Y = [600, 700, 800, 900]
#Robot 2
#Y = [185, 200, 215, 230]
#X = [262, 284, 307, 329]
#Y = [200, 220, 240, 260]
#X = [625, 720, 830, 955]
X = []
Y = []

num =raw_input("input dist data:\n")
X = num.split(' ')
X = map(eval, X)
num = raw_input("input speed (Y) data:\n")
Y = num.split(' ')
Y = map(eval, Y)
print X
print Y
# for num in range(0, 3):
#     nDist = int(raw_input("input dist (X) data:\n"))
#     X.append(nDist)
# for num in range(0, 3):
#     nDist = int(raw_input("input speed (Y) data:\n"))
#     Y.append(nDist)

k1 = np.polyfit(X, Y, 1)
b1 = np.poly1d(k1);
print k1
print b1
