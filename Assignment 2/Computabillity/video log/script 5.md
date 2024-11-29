Hello, this is the fifth and final video in my video log for computabillity, this week I have mostly been generating statistics and been writing up my report, there have been a few changes to the structure of the program but nothing that can really be shown and I have not been able to remove the bug in the Hungarian algorithm, it has been predictably very difficult to find.

Here we can see a comparison of the different algorithms on randomly generated datasets of increasing size. Nearest neighbour as we know is clearly the fastest algorithm, with the Minimum Spanning Tree and Christofides' using Nearest Neighbour being very close to one another and, finally the full christofides algorithm which is the slowest on this scale, however it is unclear if that would continue to infinity based on this sample size, and, unfortunately, the algorithm becomes too unstable at this point to reliably collect any more data.

This is the brute force algorithm which as we know takes n! time.

Using a larger dataset, this time with real data, we can see that the time complexity of the nearest neighbour algorithm sits somewhere between n log n, and n squared.

it also performed better than expected on accuracy only going over 1.5 on one occasion, however, this cannot be relied upon as it gaurantees no level of accuracy.

both the minimum spanning tree algorithm.

and the

christofides algorithm, using the nearest neighbour heuristic have a time complexity between n ^ 2.5 and n^ 2.7

the accuracy is again surprisingly good here.

the time complexity of the christofides algorithm using the hungarian minimum matching algorithm appears to have a time complexity between n log n and n squared, but,

as we can see it is unable to process many of the datasets due to the bug still present.

...

I am dissapointed that I didn't overcome the Christofides algorithm but I think it was a great learning opportunity and I may even try to solidify my understanding in my spare time, that's all for now, thank you and goodbye.