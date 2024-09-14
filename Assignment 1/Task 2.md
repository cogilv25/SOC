# Introduction
Password hashing is extensively used in authentication systems protecting systems and user data from unauthorized access. A cryptographic hash function transforms given passwords into fixed-size sequences of bits which are often represented as hexadecimal numbers for brevity. The time and space complexity of the set of hashes for a given password scheme varies greatly depending on the number of possible passwords defined by the password scheme as we will explore here.

1. A password is hashed by using a cryptographic hashing algorithm which takes a string as input and produces a fixed-size sequence of bits. When a password is created only the hash of the password is stored and then when authentication is performed the hash of a given password can be calculated and compared to the stored hash. This is more secure than storing the password because attackers are unable to steal the password and acquiring the hash is less useful as it would then need to be cracked which is practically infeasible where a secure hashing algorithm has been used. The most important characteristics of cryptographic hashing functions are as follows:
	1. Determinism - The same input must always produce the same output.
	2. Avalanche Effect - A small change to the input must result in a significant change to the output.
	3. Preimage Resistance - It must be practically impossible to find an input that produces a specific output hash.
2. Attackers can mount an array of attacks on password-protected systems, for example:
	1. Brute-Force Attack - Every possible combination of characters that could make up a password are systematically tried until a match is found, this is usually incredibly computationally expensive due to the sheer number of combinations possible.
	2. Dictionary Attack - A list of common passwords or words gathered about a specific target is used to construct passwords and test them against the authentication system until a match is found.
	3. Password Recovery Attack - If a recovery mechanism can be compromised then an attacker can simply recover the victims password to gain access.
	If an attacker has a list of stolen password hashes then they are able to perform offline attacks on this list by hashing guessed passwords and comparing the generated hash to the stolen one which is faster than having to interact with the login page, potentially by a great value where powerful hardware with a  hash rate is used..
3. A salt is a random value added to a password before it is hashed, the salt needs to be stored alongside the hash so that the same hash can be calculated during future authentication. The main purpose of salting a password is to prevent attackers from precomputing a dictionary of known passwords into a list of hashes known as a rainbow table which could then be directly compared to a list of stolen hashes without having to compute the hashes in real-time, when salted, the salt for each user has to be added to each known password in the list then hashed before it can be compared to the stolen hashes. This will slow down attackers but does not actually prevent them from compromising any accounts where they are able to guess the password using a dictionary attack or any other method.
4, 5 & 6. Following are the explanations for each calculation succeeded by the final table.
- a. 
	- Based on today's date 11/09/2024 the oldest 65 year old would be turning 66 tomorrow and so would have been born on 12/09/1958 and the youngest 15 year old was born 11/09/2009. There are 18628 days between these dates inclusively (13 29th of February's) using 2 significant figures this equates to 19000 days. 
	- Time to compute dictionary of passwords: 18628 x 5 µs = 0.093s
	- Dictionary storage space requirement: 18628 x 54 bytes = 980 KB
- b. 
	- There are 26 lower case letters and 10 digits which gives 26 x 26 x 26 x 26 x 10 x 10 x 10 x 10 or 26^4 x 10^4 which gives a grand total of 4,569,760,000 possible passwords or 4.6 million
	- Time to compute dictionary of passwords: 4,569,760,000 x 5 µs = 6.3 hours
	- Dictionary storage space requirement: 4,569,760,000 x 54 bytes = 220 GB
- c. 
	- 12 digits makes the total number of possible passwords 10^12 which equals 1,000,000,000,000 or 1 trillion. 
	- Time to compute dictionary of passwords: 1,000,000,000,000 x 5 µs = 58 days
	- Dictionary storage space requirement: 1,000,000,000,000 x 54 bytes = 49 TB
- d. 
	- 12 lower case characters gives a total of 26^12 possible passwords which equals 95,428,956,661,682,176 or 95 quadrillion.
	- Time to compute dictionary of passwords: 95,428,956,661,682,176 x 5 µs = 15,000 years
	- Dictionary storage space requirement: 95,428,956,661,682,176 x 54 bytes = 4.5 EB (Exabytes)
- e. 
	- All case-sensitive alpha numeric characters gives us a-z, A-Z and 0-9 or 62 possible characters and thus we have 62^12 possible passwords which equals 3,226,266,762,397,899,821,056 or 3.2 sextillion.
	- Time to compute dictionary of passwords: 3,226,266,762,397,899,821,056 x 5 µs = 510 million years
	- Dictionary storage space requirement: 3,226,266,762,397,899,821,056 x 54 bytes = 150 ZB (Zettabytes)
- f. 
	- There is some ambiguity in what counts as a printable ASCII character but excluding the debatable characters there are 95 possible characters giving us 95^12 possible passwords which equals 540,360,087,662,636,962,890,625 or 540 sextillion.
	- Time to compute dictionary of passwords: 540,360,087,662,636,962,890,625 x 5 µs = 86 billion years.
	- Dictionary storage space requirement: 540,360,087,662,636,962,890,625 x 54 bytes = 24 YB (Yottabytes)

| Password protocol                                     | Possible passwords | Temporal for a SHA1 calculation and disk write | Storage space requirements of password/hash table |
| ----------------------------------------------------- | ------------------ | ---------------------------------------------- | ------------------------------------------------- |
| 8-digit passwords representing birth dates (ddmmyyyy) | 19000              | 0.093 seconds                                  | 980 KB                                            |
| 4 lower case letters followed by 4 digits             | 4.6 million        | 6.3 hours                                      | 220 GB                                            |
| 12-character numeric only                             | 1 trillion         | 58 days                                        | 49 TB                                             |
| 12-character lower-case letters only                  | 95 quadrillion     | 15,000 years                                   | 4.5 EB                                            |
| 12-character case-sensitive alphanumeric              | 3.2 sextillion     | 510 million years                              | 150 ZB                                            |
| 12-character any printable ASCII characters           | 540 sextillion     | 86 billion years                               | 24 YB                                             |

# Conclusion
The calculations show quite clearly that for a given password protocol any increase in it's complexity will produce exponentially greater quantities of possible passwords and thus make brute-force attacks as well as dictionary attacks take significantly longer, additionally, the storage requirements to pre-compute a full hash table quickly become inconceivable. Due to this it makes sense for users to create passwords that are both long and use as diverse a set of characters as available and it stands to reason that attackers should focus on a smaller subset of potential passwords while, conversely focusing on as great a population of targets as possible.