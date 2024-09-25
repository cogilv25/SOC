# Introduction
Hashing transforms an arbitrary length sequence of bits into a hash (fixed length sequence of bits) which can be later compared against a second hash for authentication, data integrity verification or data retrieval purposes.

For a hashing function to be considered cryptographically secure it must be infeasible to:
- Find an input that produces a given hash.
- Find 2 inputs that produce the same hash.
- Find a different input that produces the same hash as a given input.

This report will analyse the role of computer science in this field as well as evaluate recent studies going more in depth on some of their key findings.
# The Role of Computer Science in Developing and Maintaining Robust Hashing Algorithms

The importance of hashing has been recognized since the early stages of computer science [1,11:3] and it plays a pivotal role in the field of hashing algorithms with many important responsibilities.

Computer scientists must design and implement new algorithms as old algorithms are shown to have vulnerabilities for example with MD-5 which was first broken in 2004 [2] further improvements were made to attacks with a collision being found in less than a minute on a consumer notebook in 2006 [3] and more recently SHA-1 which had theoretical collision attacks faster than brute-force as early as 2005 but proved to computationally expensive but with slow improvements being made over time a practical attack was first performed in 2017 being able to "craft two colliding PDF documents containing arbitrary distinct images" [4]. This clearly shows the importance of continually creating new algorithms to protect the users' data which is reliant on these fundamental algorithms remaining secure.

High-performance hashing algorithms are required to prevent excessive wait times for users processing large sets of data but performance can not be at the expense of highly secure algorithms. Due to the ever increasing processing power available to the average user algorithms naturally become more performant over time, however, this also naturally diminishes the security of hashing algorithms as attackers can perform hashes faster as well and thus attacks become easier. This naturally facilitates the need for gradual improvements to and replacements of algorithms, which can be easily seen in the case of SHA first published in 1993, revised in 1995 producing SHA-1, SHA-2 was subsequently proposed in 2001[5 p2] and finally SHA-3 announced in 2012 after a competition running since 2008 ended with the Keccak algorithm winning and becoming the new SHA-3 standard[5 p13].

As more secure hashing standards are created and older algorithms are compromised it is the responsibility of computing professionals to adopt these new standards and migrate old systems with potentially large amounts of data and intricate compatibility concerns. There seems to be a reluctance in the industry to move to new standards where there is no strong incentive, examples of this include the move from IPv4 to Ipv6 taking over 20 years, and the transition from SHA-1 to SHA-2 to SHA-3 still ongoing with very slow adoption of SHA-3 [6 p1-2]. A practical example is the migration of hashes in a database where they can not be simply converted due to the one way nature of the hashing algorithm but instead require a progressive transition as users login providing their passwords which can then be used to generate the new hash.

/* 

As more secure algorithms are released and older algorithms are compromised it is the responsibility of computing professionals to adopt these new standards, potentially migrating large amounts of data to the new algorithm while maintaining data integrity in live systems. [!!!]

Adversarial testing and responsible disclosure is an important practice in that it allows flaws or vulnerabilities to be found and patched hopefully before any malicious actor has the chance to capitalise on the situation and certainly into the future where previous exploitations are discovered. [!!!]

*/

# The Emerging Threat of Quantum Algorithms
Recent studies have discussed the growing risks considering quantum computers with one paper stating "Recent groundbreaking achievements in quantum computing mean that in the nearest future, even more effective attacks on hashing algorithms will be possible." [RG, 1], and another corroborating with "Quantum computers threaten traditional hash functions." [RS , a197]. Within the quantum setting the best algorithm for a generic or "brute-force" attack is the BHT algorithm presented in 1998 by Brassard et al. [9] but this requires an environment with an exponentially large qRAM [10 p239] and current quantum computers have very limited qRAM with IBM hoping to have a system with a mere 200 qubits by 2029 and 2000 qubits by 2033 [11]. A generic attack can be performed with the CNS algorithm with linear memory complexity but at the cost of increasing time complexity [12]. There is currently active research on reducing the time complexity of quantum based attacks, it has been shown this can be reduced as compared to a quantum generic attack [10 p257] and over time it is only likely to be improved upon bringing us ever-closer to genuine threats from the quantum computing space.

# Contemporary Algorithms
Another area of discussion is the need to find novel hash functions [RS, a197] as aside from the quantum threats a "steady increases in computing power has already made many cryptographic attacks that were previously considered only theoretical now practical for malicious users" [RG, 1]. Ullah and Pun present a novel algorithm that uses self-learning neurons and a forwarding and backward propagation process during the implementation phase to generate collision free hash values with initial experimentation showing that the proposed algorithm is faster and more collision resistant than existing state-of-the-art algorithms [13]. Another recent new algorithm is Nik-512 which uses a reduced number of rounds compared to alternatives with each round being more computationally expensive. During experimentation Nik-512 able to out-perform other hashing algorithms in terms of the avalanche effect, indicating increased collision resistance [RG].
# Conclusion
