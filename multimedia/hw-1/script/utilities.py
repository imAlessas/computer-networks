import math
import matplotlib.pyplot as plt



# plot_figure function
def plot_figure(figure, title, style='gray', colorbar=True):
    """
    Function to plot a figure with specified style and title.

    Parameters:
    figure (array): The figure to be plotted.
    title (str): The title of the plot.
    style (str, optional): The style of the plot. Default is 'gray'.
    colorbar (bool, optional): Whether to display a colorbar. Default is True.
    """
    plt.figure()
    plt.imshow(figure, cmap=style)
    plt.axis('image')
    plt.axis('off')
    plt.title(title)

    if colorbar:
        plt.colorbar()

def print_task(n, task_color = "white", number_color= "white"):
    
    color = {
    "black": "\033[30m",
    "red": "\033[31m",
    "green": "\033[32m",
    "orange": "\033[33m",
    "blue": "\033[34m",
    "purple": "\033[35m",
    "cyan": "\033[36m",
    "white": "\033[37m"
    }

    print(f"\n{color[task_color]}Task {color[number_color]}{n}\033[0m")






# Exponential Golomb utilites

def exp_golomb_count(error_vector):
    """
    Calculate the bitrate based on the exponential Golomb code for a given error vector.

    Parameters:
    error_vector (list): List of error symbols.

    Returns:
    float: Bitrate per pixel.
    """
    bit_count = 0
    for symbol in error_vector:
        bit_count += len(exp_golomb_signed(int(symbol)))

    return bit_count

def exp_golomb_signed(n):
    """
    Computes the Exp-Golomb code for signed integers.
    
    Parameters:
    n (int): Signed integer to be encoded.
    
    Returns:
    (str): Binary representation of the Exp-Golomb code.
    """
    if n > 0:
        return exp_golomb_unsigned(2 * n - 1)
        
    return exp_golomb_unsigned(-2 * n)
    
def exp_golomb_unsigned(n):
    """
    Computes the Exp-Golomb code for non-negative integers.
    
    Parameters:
    n (int): Non-negative integer to be encoded.
    
    Returns:
    (str): Binary representation of the Exp-Golomb code.
    """

    # Handle the case where N is zero
    if n == 0:
        return '1'
    
    # returns the coded string of bits  →  zeros( ⌊ log2(n + 1) ⌋ ) + dec2bin(n + 1)
    return '0' * int( math.floor( math.log2(n + 1) ) ) + format(n + 1, 'b')
