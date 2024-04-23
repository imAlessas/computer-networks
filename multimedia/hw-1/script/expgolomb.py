import math

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



