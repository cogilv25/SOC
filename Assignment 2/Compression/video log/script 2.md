Hello, this is the second video in my video log for compression, although I had intended to support maximum length codes, I have decided to limit these to 32bits. I discovered that to generate a code of length n the count for the most occuring byte divided by the count for the least occuring byte must be at least 2 to the power of n + 1. I don't expect you to do the maths in your head but this means even for a 32 bit code we need over 4GB of one character for every byte of another, as such I have decided using 32bit codes will be good enough for most reasonable uses and it would be easy to increase this if needed as I would only need to change the types of the variables involved.

[demo] tree, out, stats, show file, book, `wc -c test.txt`

# Notes
total bits = 1 + 2 + 3 + 4 .. n
           = (n^2 + n) / 2
           = (255^2 + 255) / 2
           = 32640 bits
           = 4080 bytes

Due to the nature of binary trees one of the conditions to generate a code with length n is:

a / b must be at least (2 ^ n) + 1


where a is a count of the most frequent byte, 

and b is a count of the least frequent byte 

64 bits = ~18 quintillion 
32 bits = ~4 billion