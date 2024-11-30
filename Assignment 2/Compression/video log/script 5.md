Hello, this is the fifth and final video in my video log for compression, this week I have mostly been generating statistics and writing my report, with the only visible change to the program being the inclusion of some statistics to show what the size would have been using a fixed-length encoding.

With carefully crafted inputs you can find cases where a fixed length encoding would be an improvement,

[]

We are using a simple but not particularly space efficient dictionary encoding scheme, I would like to test out different dictionary encoding schemes and use the 5 flag bits I reserved at the start of the file to multiplex between them but it would be foolish to attempt this with only a few hours left until submission so that will have to be an exercise for the future.

[report]

as you can see both encoding and decoding have a linear time complexity although decoding does become a problem well before encoding due to the angle of the line.

As we have spoken about before the algorithm does not perform well with small files, the number at the bottom would be the number of bytes in the original message the green section is the dictionary and then the orange section is the actual message.

pre-compressed files do not compress well as we would expect since the data has a shannon entropy close to 8.

one of the things I found interesting is that the algorithm works quite well on executable files and further more we can find large blocks of 0's if we look through the file that could be further compressed, I think it would be an interesting exercise to create an executable that decompresses itself in memory while continuing to run. 

Finally to no great surprise plain text has the highest compression ratio since a small subset of available bytes are typically used and their is a great disparity in the occurence of different characters in natural languages.


I have quite enjoyed making this application and learning about huffman codes, I wish I had some more time to explore the other avenues I have mentioned but I'm fairly happy with where I got to all things considered,

that's all for now, thank you and goodbye.