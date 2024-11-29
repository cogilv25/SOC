Hello, this is the third video in my video log for computabillity, I have a q flag which prevents the path being outputted and makes comparing the statistics of different options easier where larger datasets are used.

[demo] cb, cn, ch on burma14

I'm still having a lot of problems with the christofides algorithm, sometimes it is worse than the simple min spanning tree algorithm [eil51], sometimes it gets stuck somewhere and never finishes [demo]. I have 2 areas where I know there are issues that could be causing this, one is that the min spanning tree is not iterated through as a tree but rather as a set of connections, which may be the issue, the other is that I haven't been able to find a solution other than using brute force to reliably find the minimum perfect matching of odd degree nodes in the final step of the hungarian algorithm.

Ok, over the next week I am aiming to explore different ways of iterating through the min spanning tree to see if I can get an improvement there, and finding a solution to the problem with the final step of the hungarian algorithm. That's all for today, thank you and goodbye.