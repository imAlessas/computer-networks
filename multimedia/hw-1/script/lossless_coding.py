# python modules
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
import os
import cv2

# user defined modules
from expgolomb import exp_golomb_signed





# plot_figure function
def plot_figure(figure, title, style = 'gray', colorbar = True):
    plt.figure()
    plt.imshow(figure, cmap = style)
    plt.axis('image')
    plt.axis('off')
    plt.title(title)

    if colorbar:
        plt.colorbar()





# bitrate calculation function based on the exponential golomb code
def exp_golomb_bitrate(error_vector):
    bit_count = 0
    for symbol in error_vector:
        bit_count += len(exp_golomb_signed(int(symbol)))

    return bit_count / img_size





# median function for advanced coding
def median(a, b, c):
    vector = [a, b, c]
    vector.remove(min(vector))
    vector.remove(max(vector))
    
    return vector[0]





# codes an image based on the guidelines directives
def advanced_coding(img):
    # blank image 
    predicted_img = np.zeros_like(img)

    # iterates through the rows (height)
    for row in range(height):

        # iterates through the cols (width)
        for col in range(width - 1):

            if row == 0 and col == 0:   # first pixel
                predicted_img[row][col] = img[row][col] - 128
            
            elif row == 1:              # first row
                predicted_img[row][col] = img[row][col - 1]
            
            elif col == 1:              # first col
                predicted_img[row][col] = img[row - 1][col]
            
            elif col == (width - 1):    # last col
                predicted_img[row][col] = median(img[row - 1][col], img[row][col - 1], img[row - 1][col - 1])

            else:                       # other cases
                predicted_img[row][col] = median(img[row - 1][col], img[row][col - 1], img[row - 1][col + 1])

    return predicted_img





# - - - - -  main - - - - -

if __name__ == "__main__":
    
    
    #######    Task 1    #######

    # Prepare to load the image
    img_file_name = "peppers"
    img_extension = ".pgm"
    current_dir = os.getcwd()

    # path to reach the img
    path_to_img = os.path.join(current_dir, "multimedia", "hw-1", "script", "imgs") + "/"

    # loads the colored image
    img = mpimg.imread(path_to_img +  img_file_name + img_extension) 

    # extracts the luminance
    # gray_img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    gray_img = img

    # plots images
    # plot_figure(img, 'Colored image', colorbar = False)
    plot_figure(gray_img, 'Grayscale image')



    # flatten the transposed matrix to read pixels row by row
    raster_scan = np.transpose(gray_img).flatten()

    # count the occurrences of each pixel value
    occurrencies = np.histogram(raster_scan, bins=range(256))[0]

    # calculate the relative frequencies
    rel_freq = occurrencies / np.sum(occurrencies)

    # remove zero-values of probability
    p = rel_freq[rel_freq > 0]

    # compute and display the entropy
    HX = np.sum(p * np.log2(1 / p))
    print(f"\nThe entropy of {img_file_name}{img_extension} is {HX:.3f} bpp")
    # print(f"The compression ratio of {img_file_name}{img_extension} is {8/HX:.4f}\n")





    #######    Task 2    #######

    # zip the image
    cmd = f"zip {path_to_img}{img_file_name}.zip {path_to_img}{img_file_name}{img_extension}"
    os.system(cmd)

    # get the zip bytes
    img_stats = os.stat(f"{path_to_img}{img_file_name}{img_extension}")
    zip_bytes = img_stats.st_size

    # get img size
    height, width = gray_img.shape
    img_size = width * height

    # get the birate
    zip_bitrate = zip_bytes * 8 / img_size 

    print(f"\nThe bitrate of {img_file_name}.zip is {zip_bitrate:.3f} bpp\n")





    #######    Task 4    #######

    # calculate prediction error
    simple_coding_error = np.concatenate(([raster_scan[0] - 128], np.diff(raster_scan.astype(float))))

    # plot error graph
    simple_coding_error_img = np.transpose(np.reshape(np.abs(simple_coding_error), (width, height)))
    plot_figure(simple_coding_error_img, 'Simple coding prediction error', 'seismic')

    # count the occurrences of each prediction error value
    occ, _ = np.histogram(simple_coding_error, bins = range(-255, 256))

    # calculate the relative frequencies and remove any probability == 0
    freqRel = occ / np.sum(occ)
    p = freqRel[freqRel > 0]

    # calculate the entropy
    HY = np.sum(p * np.log2(1 / p))
    
    print(f"The entropy of the prediction error of {img_file_name} is {HY:.3f} bpp")
    print(f"The compression ratio of {img_file_name} is {8/HY:.4f}\n")





    #######    Task 5    #######

    exp_golomb_bpp = exp_golomb_bitrate(simple_coding_error)    
    
    print(f"The bitrate of the simple coding is {exp_golomb_bpp:.4f}\n")





    #######    Task 6    #######

    predicted_img = advanced_coding(gray_img)

    adv_coding_error_img = gray_img - predicted_img

    # plots error
    plot_figure(np.abs(adv_coding_error_img), 'Advanced coding prediction error', 'seismic')
    
    # plots difference between error
    # plot_figure(np.abs(simple_coding_error_img - adv_coding_error_img), 'Difference between two techniques', 'seismic')





    #######    Task 7    #######

    exp_golomb_bpp = exp_golomb_bitrate(np.reshape(adv_coding_error_img, img_size))
    
    print(f"The bitrate of the advanced coding is {exp_golomb_bpp:.4f}\n")








    ### Show all the figures ###

    plt.show()