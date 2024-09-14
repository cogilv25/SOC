# Introduction
Hashing transforms an arbitrary length sequence of bits into a hash (fixed length sequence of bits) which can be later compared against a second hash for authentication, data integrity verification or data retrieval purposes.

For a hashing function to be considered cryptographically secure it must be infeasible to:
- Find an input that produces a given hash.
- Find 2 inputs that produce the same hash.
- Find a different input that produces the same hash as a given input.

This report will analyse the role of computer science in this field as well as evaluate recent studies going more in depth on some of their key findings.
# The Role of Computer Science in Developing and Maintaining Robust Hashing Algorithms

The importance of hashing has been recognized since the early stages of computer science [1,11:3] and it plays a pivotal role in the field of hashing algorithms with many important responsibilities.

Computer scientists must design and implement new algorithms as old algorithms are shown to have vulnerabilities for example with MD-5 [] which had blah blah and more recently SHA-1 [] which had pdf thingy. This clearly shows the importance of continually creating new algorithms to protect the data of users which is protected by these fundamental algorithms.

Performance improvements are sometimes discovered for hashing algorithms which reduces wait times for users although can also have a negative effect on security allowing attackers to compute hashes faster as well. Additionally security patches are released where a small change can be made to protect against specific attacks without requiring a significant change to the inner workings of the algorithm.[!!!]

As more secure algorithms are released and older algorithms are compromised it is the responsibility of computing professionals to adopt these new standards, potentially migrating large amounts of data to the new algorithm while maintaining data integrity in live systems. [!!!]

Adversarial testing and responsible disclosure is an important practice in that it allows flaws or vulnerabilities to be found and patched hopefully before any malicious actor has the chance to capitalise on the situation and certainly into the future where previous exploitations are discovered. [!!!]

# Key Findings of Recent Studies

# The Emerging Threat of Quantum Algorithms

# Contemporary Algorithms

# Conclusion