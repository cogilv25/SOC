Hello, this is the first video in my video log for computability, I have chosen the travelling salesman pathway and for the first week I have concentrated on reading input from files using the TSPLIB format and implementing a brute force solving algorithm. I chose to use C++ for the application as it has performance close to that of C with high level abstractions which I believe will be useful.

TSPLIB supports multiple types of problem, we ignore problems wich are not Travelling Salesman Problems but otherwise we are close to full specification compliance with the 2 known exceptions being we don't support explicit matrices and  we cannot parse some 3D problems.

[demo] 1,2,3, burma

Over the next week I am aiming to: 
- implement the Nearest Neighbour and Christofide's algorithms,
- implement high precision timing, 
- add cli input instead of directly modifying the source code, and finally, 
- refactor my TSPMatrix class in order to support the 2 unsupported cases when parsing TSPLIB files.

That's all I have today,
Thank you and goodbye