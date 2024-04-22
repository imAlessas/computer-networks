from exp_golomb_unsigned import exp_golomb_unsigned

def exp_golomb_signed(N):
    """
    Computes the Exp-Golomb code for signed integers.
    
    Parameters:
    N (int): Signed integer to be encoded.
    
    Returns:
    bits (str): Binary representation of the Exp-Golomb code.
    """
    if N > 0:
        bits = exp_golomb_unsigned(2*N - 1)
    else:
        bits = exp_golomb_unsigned(-2*N)
    
    return bits
