import math

def exp_golomb_unsigned(N):
    """
    Computes the Exp-Golomb code for non-negative integers.
    
    Parameters:
    N (int): Non-negative integer to be encoded.
    
    Returns:
    bits (str): Binary representation of the Exp-Golomb code.
    """
    if N > 0:
        # Calculate the number of trailing bits
        trailBits = format(N + 1, '0' + str(int(math.floor(math.log2(N + 1)))) + 'b')
        # Calculate the number of leading bits
        headBits = '0' * (len(trailBits) - 1)
        # Concatenate the leading and trailing bits
        bits = headBits + trailBits
    else:
        # Handle the case where N is zero
        bits = '1'
    
    return bits

