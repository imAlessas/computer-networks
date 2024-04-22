import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
import os
import cv2


#######    Task 1    #######

## Display the images

# Prepare to load the image
img_file_name = "spiderman"
img_extension = ".jpg"
current_dir = os.getcwd()

# path to reach the img
path_to_img = os.path.join(current_dir, "multimedia", "hw-1", "script", "imgs") + "/"

# loads the colored image
img = mpimg.imread(path_to_img +  img_file_name + img_extension) 

# extracts the luminance
gray_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

# creates a figure with two subplots
# fig, axs = plt.subplots(1, 2, figsize=(12, 6))

# displays the colored image in the first subplot
# axs[0].imshow(img, cmap='gray')
# axs[0].set_title('Colored image')
# axs[0].axis('off')

# displays the grayscale image in the second subplot
# axs[1].imshow(gray_img, cmap='gray')
# axs[1].set_title('Grayscale image')
# axs[1].axis('off')

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
print(f"\nThe entropy of {img_file_name}{img_extension} is {HX:.3f} bpp")
print(f"The compression ratio of {img_file_name}{img_extension} is {8/HX:.4f}\n")





#######    Task 2    #######

# zip the image
cmd = f"zip {path_to_img}{img_file_name}.zip {path_to_img}{img_file_name}.jpg"
os.system(cmd)

# get the zip bytes
img_stats = os.stat(f"{path_to_img}{img_file_name}.jpg")
zip_bytes = img_stats.st_size

# get img size
height, width, _ = img.shape

# get the birate
zip_bitrate = zip_bytes * 8 / (height * width) 

print(f"\nThe bitrate of {img_file_name}.zip is {zip_bitrate:.3f} bpp\n")











