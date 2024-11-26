Hello, this is the second video in my video log for computabillity, I have implemented support for 3D and explicit problems using the TSPLIB format, this only leaves some edge cases which I will have to add support for as I find them. I have implemented the command line interface for the program which I will now demonstrate.

[demo] 
- blank,
- nearest neighbour, generating random, matrix,
- timing on 13509,
- a280 min spanning tree,
- burma14 christofides 

caveat is we are using brute force to calculate the perfect matching of odd degree nodes. n! I think this could be n^2 + n / 2 based on the number of combinations of pairs, although I think I must be wrong based on fact that the current state of the art according to my reading is n^3, either way I think I will leave this for now and maybe come back to it once I have a reasonably fast and well known implementation of the algorithm.

I have learned from this week to not underestimate the complexity of these algorithms and as such I am going to focus on implementing a nearest neighbour heuristic as well as the Hungarian Minimum Matching algorithm for the matching process over the next week.