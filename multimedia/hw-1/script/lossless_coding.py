import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
import os
import cv2


#######    Task 1    #######

## Display the images

# Prepare to load the image
img_file_name = "spiderman.jpg"
current_dir = os.getcwd()

img_path = os.path.join(current_dir, "multimedia", "hw-1", "script", "imgs", img_file_name) # construct the full path to the image file

# loads the colored image
img = mpimg.imread(img_path) 

# extracts the luminance
gray_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

# creates a figure with two subplots
fig, axs = plt.subplots(1, 2, figsize=(12, 6))

# displays the colored image in the first subplot
axs[0].imshow(img, cmap='gray')
axs[0].set_title('Colored image')
axs[0].axis('off')

# displays the grayscale image in the second subplot
axs[1].imshow(gray_img, cmap='gray')
axs[1].set_title('Grayscale image')
axs[1].axis('off')

# plt.show()



## Compute the entropy

# flatten the transposed matrix to read pixels row by row
rasterScan = np.transpose(gray_img).flatten()

# count the occurrences of each pixel value
occurrencies = np.histogram(rasterScan, bins=range(256))[0]

# calculate the relative frequencies
p = occurrencies / np.sum(occurrencies)

# remove zero-values of probability
p = p[p > 0]

# compute and display the entropy
HX = - np.sum(p * np.log2(p))
print(f"\nThe entropy of {img_file_name} is {HX:.3f} bpp\n")



#######    Task 2    #######
