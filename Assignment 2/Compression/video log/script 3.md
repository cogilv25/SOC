Hello, this is the third video in my video log for compression, I have implemented encoding dictionaries into the output and decoding of files, I chose to disallow decoding from the command line as this seems like a strange use case and encoding a dictionary only takes place if you are outputting to a file.

[demo] cli->file, file->cli, file->file, file->file

This example is actually shortened from the original to keep the encoded message below 1024 Bytes as we are using a buffer for both input and output and there is a bug I haven't quite found yet that causes the ouput message to be corrupted occasionally when we read more data into the input buffer.

[show by encoding the full book and then decoding]

Ok, that is all for this week, over the next week I will be implementing high precision timing, some more statistics, but mostly bug fixing and refactoring due to the code slowly descending into entropy and becoming more difficult to work on. That's all for today, thank you and goodbye.