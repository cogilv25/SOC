Hello, this is the fourth video in my video log for computabillity, I have changed the way the christofides algorithm is walking through the minimum spanning tree after adding the perfect matching of odd degree nodes. It is now written recursively and walks the minimum spanning tree in reverse order skipping nodes that have already been visited. I have solved the final step in the hungarian matching algorithm, this is again a recursive algorithm, although much more complicated, it recursively builds a perfect matching by adding pairs which only occur once in the matrix and where there are no pairs that occur only once, it reverts to brute force, by creating a sub problem for each possibillity. Unfortunately I am finding that certain problems, especially very large problems are getting stuck earlier in the hungarian algorithm, which suggests there is another bug and debugging this stage is extremely difficult due to these operations taking place on matrices with 10000 cells or more.


Christofides is now returning 1.5x accuracy on all problems
[demo] -ch burma14.tsp - 3332
att48 - 10628
eli76 - 538

eli101 either takes a long time or never returns. This is typical of larger datasets, however, using the random generation of matrices feature it is possible to occasionally get datasets of 200 to run and so I believe there are some specific circumstances which my implementation doesn't handle.

It seems to me that we are getting better than expected time complexity here but I haven't run the numbers yet and so therefore may be completely incorrect, however, at any rate we can see that we are getting fairly accurate results in significantly less time than with the brute force method, for example burma14 takes about 80 seconds with brute force but only20 micro seconds with christofides.

Ok, over the next week I will mostly be gathering statistics, writing my report, and refactoring the program, although if I can I will try to solve the problem with the hugarian algorithm. That's all for today, thank you and goodbye.