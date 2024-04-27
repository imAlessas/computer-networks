# python modules
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
import os

# user defined modules
from utilities import *






# median function for advanced coding
def median(a, b, c):
    
    v = [a, b, c]
    v.sort()

    return v[1]



# codes an image based on the guidelines directives
def advanced_coding(img):
    # blank image 
    predicted_img = np.zeros_like(img)
    
    # extracts the height and the width from the image
    height, width = img.shape[0], img.shape[1]

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
    

    #######    Setup    #######
    print_task(0, task_color="purple")

    img_file_name = "peppers"
    img_extension = ".pgm"
    current_dir = os.getcwd()

    # path to reach the img
    path_to_img = os.path.join(current_dir, "multimedia", "hw-1", "script", "imgs") + "/"

    # loads the colored image
    gray_img = mpimg.imread(path_to_img +  img_file_name + img_extension) 

    # extracts the luminance if RGB
    if gray_img[0][0].size > 1:
        gray_img = np.dot(gray_img[..., :3], [1, 1, 1]) / 3

    plot_figure(gray_img, 'Grayscale image', colorbar = False)


    
    #######    Task 1    #######
    # Perform "advanced" predictive coding: first, the predictor 𝑝 is constructed by scanning the image
    print_task(1, task_color="purple", number_color="red")
    
    # performs advanced coding
    predicted_img = advanced_coding(gray_img)

    # calculates the prediction error
    adv_coding_error_img = gray_img - predicted_img

    # plots error
    plot_figure(np.abs(adv_coding_error_img), 'Advanced coding prediction error', 'seismic')





    # exp_golomb_bpp = exp_golomb_bitrate(np.reshape(adv_coding_error_img, img_size))
    
    # print(f"The bitrate of the advanced coding is {exp_golomb_bpp:.4f}\n")








    ### Show all the figures ###

    plt.show()