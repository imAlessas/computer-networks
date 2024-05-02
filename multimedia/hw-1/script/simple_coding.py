# python modules
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
import os

# user defined modules
from utilities import *




if __name__ == "__main__":
    

    #######    Task 1    #######
    # Loading an image in grayscale or in color, but in the latter case, it is necessary to extract the luminance component, which can be approximated as the   average of the RGB components.
    print_task(1, task_color="purple", number_color="green")

    # Prepare to load the image
    img_file_name = "spiderman"
    img_extension = ".jpg"
    current_dir = os.getcwd()

    # path to reach the img
    path_to_img = os.path.join(current_dir, "multimedia", "hw-1", "script", "imgs") + "/"

    # loads the colored image
    gray_img = mpimg.imread(path_to_img +  img_file_name + img_extension).astype(np.int16)

    # extracts the luminance if RGB
    if gray_img[0][0].size > 1:
        gray_img = np.dot(gray_img[..., :3], [1, 1, 1]) / 3

    # plots images
    plot_figure(gray_img, 'Grayscale image', colorbar = False)
    


    ######    Task 2    #######
    # Let 𝑓(𝑛, 𝑚) be the grayscale image: estimate its entropy, expressing it in bits per pixel.
    print_task(2, task_color="purple", number_color="green")

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
    print(f"The entropy of {img_file_name}{img_extension} is {HX:.3f} bpp")



    ######    Task 3    #######
    # Use an application like zip in Windows or gzip in Linux and calculate the resulting bitrate (file size in bits divided by the number of pixels).
    print_task(3, task_color="purple", number_color="green")

    # change the current working directory to the directory containing the image
    os.chdir(path_to_img)

    # zip the image
    cmd = f"zip {img_file_name}.zip {img_file_name}{img_extension}"
    os.system(cmd)
    
    # get the zip bytes
    zip_bytes = os.stat(f"{img_file_name}.zip").st_size

    # get img size
    height, width = gray_img.shape
    img_size = width * height

    # get the birate
    zip_bitrate = zip_bytes * 8 / img_size 

    print(f"The bitrate of {img_file_name}.zip is {zip_bitrate:.3f} bpp")



    ######    Task 5    #######
    # Perform "simple" predictive coding
    print_task(5, task_color="purple", number_color="green")

    # calculate prediction error
    simple_coding_error = np.concatenate(([raster_scan[0] - 128], np.diff(raster_scan.astype(float))))

    # plot error graph
    simple_coding_error_img = np.transpose(np.reshape(np.abs(simple_coding_error), (width, height)))
    plot_figure(simple_coding_error_img, 'Simple coding prediction error', 'seismic')



    ######    Task 6    #######
    # Estimate the entropy of the prediction error 𝑦
    print_task(6, task_color="purple", number_color="green")

    # count the occurrences of each prediction error value
    occ, _ = np.histogram(simple_coding_error, bins = range(-255, 256))

    # calculate the relative frequencies and remove any probability == 0
    freqRel = occ / np.sum(occ)
    p = freqRel[freqRel > 0]

    # calculate the entropy
    HY = np.sum(p * np.log2(1 / p))
    
    print(f"The entropy of the simple prediction error of {img_file_name} is {HY:.3f} bpp")



    ######    Task 7    #######
    # Evaluate the number of bits required to encode the prediction error 𝑦 using Signed Exp-Golomb coding, and deduce the encoding bitrate.
    print_task(7, task_color="purple", number_color="green")

    exp_golomb_bit = exp_golomb_count(simple_coding_error) 

    exp_golomb_bpp = exp_golomb_bit / img_size

    print(f"The number of bits for the simple coding is {exp_golomb_bit}")
    print(f"The bitrate of the simple coding is {exp_golomb_bpp:.3f}\n")




    ### Show all the figures ###
    plt.show()
