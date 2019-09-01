import numpy as np
import matplotlib.pyplot as plt
#from skimage import io
import cv2
# https://blog.csdn.net/u010468553/article/details/79456447

lutSize = 64
size = int(np.sqrt(lutSize))
gap = int(256/lutSize)
nodes = np.linspace(0, 256, lutSize,dtype='int')
nodes[nodes > 255] = 255
print(nodes)
lutImage_net = np.zeros((lutSize*size, lutSize*size, 3)).astype(np.int)
lut_cols, lut_rows, lut_bands = lutImage_net.shape
for i in range(lut_rows):
    for j in range(lut_cols):
        blue_rows = i/lutSize
        blue_cols = j/lutSize
        blue_idx = blue_rows*size+blue_cols
        red_idx = j % lutSize
        green_idx = i % lutSize
        lutImage_net[i, j, :] = 2*np.array([int(red_idx*256/lutSize), int(green_idx*256/lutSize), int(blue_idx*256/lutSize)])
plt.figure("lutImage_net")
plt.imshow(lutImage_net)
lutImage = np.zeros((lutSize*lutSize*lutSize, 3)).astype(np.int)
for b1 in range(size):
    for g in range(lutSize):
        for b2 in range(size):
            for r in range(lutSize):
                lutImage[(b1*lutSize+g)*lutSize*size+r+b2*lutSize, :] = 2*np.array([nodes[r], nodes[g], nodes[b1*size+b2]])
lutImage = lutImage.reshape(lutSize*size, lutSize*size, 3)
plt.figure("lutImage")
plt.imshow(lutImage)
#plt.show()#
input = cv2.imread("../data/24color.jpg")
inputImg = cv2.cvtColor(input, cv2.COLOR_BGR2RGB)
rows, cols, bands = inputImg.shape
outputImage_net = np.zeros((rows*cols, 3)).astype(int)
outputImage = np.zeros((rows*cols, 3)).astype(int)
inputImg2 = inputImg.reshape(rows*cols, 3)
count = 0
for color in inputImg2:
    blue_rows = int(color[2] / gap / size )   #0~255 -> 0~63 -> 0~7
    blue_cols = int(color[2] / gap % size  ) #0~255 -> 0~63 -> 0~7
    green_rows = int(color[1] / gap)       #0~255 -> 0~63
    red_cols = int(color[0] / gap )        #0~255 -> 0~63
    ans_rows = int(blue_rows * lutSize + green_rows)
    ans_cols = int(blue_cols * lutSize + red_cols)
    outputImage_net[count, :] = lutImage_net[ans_rows, ans_cols, :]
    outputImage[count, :] = lutImage[ans_rows, ans_cols, :]
    count = count + 1
outputImage_net = outputImage_net.reshape(rows, cols, bands)
outputImage = outputImage.reshape(rows, cols, bands)
plt.figure("input")
plt.imshow(inputImg)
plt.figure("ouput_net")
plt.imshow(outputImage_net)
plt.figure("ouput")
plt.imshow(outputImage)
plt.show()
