// This application is able to compress and decompress strings or files
// using Huffman encoding. Additionally using flags you can show the
// intermediate tree and dictionary, or see statistics about the input
// and output such as compression ratio and shannon entropy. Running
// the program with no arguments will provide full usage guidance.

#define _CRT_SECURE_NO_WARNINGS // We are not worried about security here, this is a demo application.
// Should be 32 but to save a bit per code in the dictionary I made it 31
// TODO: Upgrade to 63 and use the extra bit per code.
#define MAX_CODE_BITS 31

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "timing.h"

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef unsigned long long uint64;


// TODO: right is not needed since it is always left + 1,
// TODO: remove union, originally intended to support multiple
//    input sizes rather than just uint8 or bytes, this was not
//    done, and I think it would be quite difficult and possibly
//    unproductive now having more knowledge of how the algorithm
//    works.
typedef struct TreeNode
{
	uint64 count;
	struct TreeNode* left;
	union
	{
		struct TreeNode* p;
		uint8 uint8Value;
		uint16 uint16Value;
		uint32 uint32Value;
		uint64 uint64Value;
	} right;
} TreeNode;

typedef struct
{
	uint64 map[256];
	uint64 count;
	uint16 uniqueCount;
} CountMap;

typedef struct
{
	TreeNode* root;
	CountMap* map;
} HuffmanTree;

typedef struct
{
	uint8 depth;
	uint32 code;
} HuffmanCode;

// Global unnamed struct instance to store statistics  
struct
{
	double compressionRatio;
	double shannonEntropy;
	double averageCodeLength;
	uint16 uniqueBytesUsed;
	uint64 bytesBeforeEncoding;
	uint64 bitsAfterEncoding;
	uint64 dictionaryBitLength;
	uint64 encodedDictionaryBits;
	uint64 bytesAfterDecoding;
	double timeTaken;
} stats = { 0 };

typedef enum
{
	CALLOC_FAILED = 0,
	FILE_NON_EXISTENT = 1,
	DEPTH_LIMIT_EXCEEDED = 2,
	UNIMPLEMENTED_ERROR = 3,
	MULTIPLE_INPUTS = 4,
	UNEXPECTED_ERROR = 5,
	NO_OUTPUT_FILE = 6,
	FILE_WRITE_FAILED = 7,
	WRITE_FILE_OPEN_FAILED = 8,
	CORRUPT_DICTIONARY = 9,
	DECODE_CLI_UNSUPPORTED = 10,
	CORRUPT_ENCODED_FILE = 11,
	NO_INPUT = 12
} ErrorCode;

typedef enum
{
	SEVERITY_WARNING = 0,
	SEVERITY_FATAL = 1
} ErrorSeverity;

const char* ErrorStart[] =
{
	"\x1b[33mWarning:",
	"\x1b[31mFatal Error:"
};

const char* ErrorMessages[] =
{
	"A call to calloc failed! this should only happen if the OS can't return a block the size requested. "
	"Either your memory is completely full, incredibly fragmented, or something odd has happened!",
	"Filepath entered does not point to a file.",
	"The maximum supported length of a generated huffman code has been exceeded!",
	"The feature you are trying to use is currently unimplemented!",
	"Only one input string is supported! Multiple were provided, if the input contains spaces, surround it with quotes... Continuing using last input..",
	"An infeasible error has occured, please try again, if this continues contact the creator of this program.",
	"Output flag was set but no file was provided!",
	"An error occured while writing to the output file",
	"An error occured preventing the output file from being overwritten / created!",
	"The dictionary of the file you are trying to decode is corrupt!",
	"Decoding messages from the command line is not supported..",
	"The file is corrupt and cannot be decoded!",
	"You must provide an input string or, when using the -f flag a filepath"
};

// Flags and command line argument state.
bool dFlag = false;
bool tFlag = false;
bool oFlag = false; const char* output;
bool fFlag = false;
bool rFlag = false;
bool sFlag = false;
bool bFlag = false;
bool nFlag = false;
const char* input = 0;

void printUsage()
{
	printf("usage: comp [-b] [-s] [-t] [-r] [-d] [-n] [-o <filepath>] [-f] <input>  \n\n");
	printf("<input> is interpreted as a string unless -f is provided.\n\n");
	printf("Flags: \n");
	printf("    -f Interpret <input> as a filepath and compress the file it points to.\n");
	printf("    -b Print the encoded message as a binary string.\n");
	printf("    -d Output the Huffman code dictionary internally generated by the program.\n");
	printf("    -s Generate and print additional statistics about the input, internal structures, and output\n");
	printf("    -t Output the Huffman tree internally generated to the console. (large trees may not display correctly)\n");
	printf("    -r Decode the input, as this does not produce a Huffman tree the t flag is ignored.\n");
	printf("    -o <filepath> Write the encoded / decoded message to the file pointed to by filepath.\n");
	printf("    -n Do not output the encoded message (Useful for gathering statistics).\n");
}

bool printErrorMessageIf(bool condition, const char* message, ErrorSeverity severity)
{
	if (condition)
		printf("%s\x1b[0m %s\n", ErrorStart[severity], message);

	return condition;
}

// Prints the error associated with the error code if the condition
//     is true and returns the condition. Cleans up error reporting.
bool printErrorIf(bool condition, ErrorCode error, ErrorSeverity severity)
{
	return printErrorMessageIf(condition, ErrorMessages[error], severity);
}

// Basically an Assert with re-usable error codes rather than strings.
void fatalErrorIf(bool condition, ErrorCode error)
{
	if (printErrorIf(condition, error, SEVERITY_FATAL))
	{
		printf("\nFor usage help provide no input.\n");
		exit(-1);
	}
}

HuffmanTree createHuffmanTree(CountMap* map)
{
	// Maximum nodes = n * 2
	TreeNode* nodes = calloc(map->uniqueCount * 2, sizeof(TreeNode));
	fatalErrorIf(nodes == NULL, CALLOC_FAILED);

	uint16 count = 0;
	uint8 symbol = 0;
	for (; count < map->uniqueCount; ++symbol)
	{
		// Find next symbol
		for (; symbol < 256; ++symbol)
			if (map->map[symbol] > 0) break;

		// Find insertion point
		uint8 i = count;
		for (; i > 0 && map->map[symbol] < nodes[i - 1].count; --i)
			memcpy(nodes + i, nodes + i - 1, sizeof(TreeNode));

		// Insert element
		nodes[i].count = map->map[symbol];
		nodes[i].left = 0;
		nodes[i].right.uint8Value = symbol;
		++count;
	}

	// Ok, so we are going to start using the nodes array from both ends.
	//    From the end of the array going backwards (right array) are the 
	//    sorted nodes from smallest to largest count and from the start
	//    of the array going forwards (left array) are the unsorted nodes 
	//    again from smallest to largest count. We can guarantee these will
	//    never overlap due to the binary tree at max having 2n nodes. Once
	//    the left array has 1 item we are done.
	uint16 rightTail = map->uniqueCount * 2 - 1;
	for (;count > 1; --count)
	{
		// Copy 2 elements to right array.
		memcpy(&nodes[rightTail--], nodes, sizeof(TreeNode));
		memcpy(&nodes[rightTail--], nodes + 1, sizeof(TreeNode));
		// Move left array left by 2 elements
		memcpy(nodes, nodes + 2, sizeof(TreeNode) * (count - 2));

		// Find insertion point
		uint64 newNodeCount = nodes[rightTail + 1].count + nodes[rightTail + 2].count;
		uint8 i = count - 2;
		for (; i > 0 && newNodeCount < nodes[i - 1].count; --i)
			memcpy(nodes + i, nodes + i - 1, sizeof(TreeNode));

		// Insert element
		nodes[i].count = newNodeCount;
		nodes[i].left = nodes + rightTail + 1;
		nodes[i].right.p = nodes + rightTail + 2;
	}

	HuffmanTree tree;
	tree.root = nodes;
	tree.map = map;
	return tree;
}

void destroyHuffmanTree(HuffmanTree* tree)
{
	free(tree->root);
}

CountMap createCountMap(const char* string)
{
	CountMap map = { 0 };

	if (!fFlag)
	{
		for (const uint8* p = string; *p != '\0'; ++p)
		{
			
			if(map.map[*p]++ == 0) // Count occurances of *p
				++map.uniqueCount; // Count unique uint8's in string
			++map.count;           // Count total uint8's in string
		}
	}
	else
	{
		FILE* f = fopen(string, "rb");

		// No microsoft, f couldn't be 0 because we are checking right here
		//     Visual Studio has really gone downhill the last ~10 years...
		fatalErrorIf(f == NULL, FILE_NON_EXISTENT);
		
		uint8 buffer[1024];
		size_t bytesRead = fread(buffer, 1, 1024, f);
		while (bytesRead > 0)
		{
			for (int i = 0; i < bytesRead; ++i)
			{
				if(map.map[buffer[i]]++ == 0)// Count occurances of buffer[i] 
					++map.uniqueCount;       // Count unique uint8's in string
				++map.count;                 // Count total uint8's in string
			}

			if (bytesRead != 1024) break;

			bytesRead = fread(buffer, 1, 1024, f);
		}
		fclose(f);
	}

	if (sFlag)
	{
		for (uint16 i = 0; i < 256; ++i)
		{
			if (map.map[i] > 0)
			{
				double p = (double)map.map[i] / (double)map.count;
				stats.shannonEntropy -= p * log2(p);
			}
		}

		stats.bytesBeforeEncoding = (double)map.count;
		stats.uniqueBytesUsed = map.uniqueCount;
	}

	return map;
}

void printHuffmanCode(HuffmanCode code)
{
	// When i goes below 0 it will be 255.
	for (uint8 i = code.depth - 1; i < code.depth; --i)
	{
		// Branchless, print the bits in a code from left
		//    to right.
		printf("%c", ((code.code >> i) & (uint32)1) + 48);
	}
}

void _recurseHuffmanTree(HuffmanCode* map, TreeNode* n, uint8 depth, uint32 code, uint32 lines, bool parse, bool print)
{
	// We have set a hard limit on huffman tree depth of 32 here..
	//    I believe this is a reasonable maximum consideriong that although the worst
	//    case is 255 bits anything beyond 32 bits is unlikely requiring
	//    over 4GB of a repeating character.
	fatalErrorIf(depth > MAX_CODE_BITS, DEPTH_LIMIT_EXCEEDED);
	HuffmanCode hCode = { depth, code };

	if (print)
	{
		if (depth > 0)
		{
			for (uint8 i = depth - 1; i > 0; --i)
			{
				if ((lines & ((uint32)1 << i)) != 0)
					printf(" %c ", 179);
				else
					printf("   ");
			}

			// Mod2 but we want to flip it so that it's 1 when it would
			// have been 0 and vice versa as it makes the logic easier.
			uint32 mod = (code % 2 + 1) % 2;

			// Branchless programming, we are changing what character we
			//    draw in the table based on the value of mod, where if
			//    mod == 0 we are at the start of a right branch.
			uint8 c = 192 + mod * 3;

			// Again, the next line will be 1 depth deeper, if it's a left
			//    branch we need to draw an up-down line otherwise a space.
			//    Essentialy lines is a bitmap that tells us what to draw.
			lines = (lines + mod) << 1;
			printf(" %c ", c);
		}
		if(n->left != 0)
			printf(" %llu\n", n->count);
		else
		{
			printf("%llu = (Symbol: %u, Code: ", n->count, n->right.uint8Value);
			printHuffmanCode(hCode);
			printf(")\n");
		}
	}
	
	if (n->left == 0)
	{
		if(parse)
			map[n->right.uint8Value] = hCode;
	}
	else
	{
		_recurseHuffmanTree(map, n->left, ++depth, code << 1, lines, parse, print);
		_recurseHuffmanTree(map, n->right.p, depth, (code << 1) + 1, lines, parse, print);
	}
		
}


void parseHuffmanTree(HuffmanCode * map, HuffmanTree* tree)
{
	_recurseHuffmanTree(map, tree->root, 0, 0, 0, true, false);
}

void printHuffmanTree(HuffmanCode* map, HuffmanTree* tree)
{
	_recurseHuffmanTree(map, tree->root, 0, 0, 0, false, true);
}

void insertCodeIntoBuffer(uint8** buffer, uint64* bitCount, uint8* bufferBit, HuffmanCode code)
{
	// Endianness makes this a little bit of a pain, ideally we would
	//    insert more than 1bit at a time but, it's very fiddly.
	// This feels really inefficient but I wouldn't be surprised
	//    if the compiler can optimize this a lot.
	// TODO: Implement logically more optimal solution and evaluate
	//    any performance gain or loss.
	for (uint8 bit = code.depth; bit > 0; --bit)
	{
		// Append a single bit of the code to the buffer
		**buffer = (**buffer << 1) + ((code.code >> (bit - 1)) & (uint32)1);
		++(*bufferBit);
		++(*bitCount);

		if (*bufferBit < 8) continue;

		// If we reach the end of a byte move to the next byte in memory
		++(*buffer);
		*bufferBit = 0;
	}
}

// Will not read past pIn + pInSize. Decodes a single HuffmanCode from the
//    input pointer and writes it's byte to the output pointer. Assumes pOut
//    points to a valid address to write to. Moves the input pointer forward
//    and modifies the bufferBit as appropriate.
void decodeCode(uint8 ** pOut, uint8 **pIn, HuffmanCode* codeMap, uint64 pInSize, uint8* bufferBit, uint64* bitCount)
{
	// The way the codeMap is laid out in memory is great for encoding
	//    but more challenging for decoding.. There will never be time
	//    but I think having 2 copies of the codeMap would be ideal..
	//    one ordered by the byte value as it is now, making encoding
	//    easy, and one ordered by code length allowing us to check all
	//    the codes in order from smallest to largest code depth making
	//    decoding more efficient. I guess this is a big TODO: <- That

	// PInSizeBits is the maximum number of bits we will read from pIn
	uint64 pInSizeBits = pInSize * 8 - (*bufferBit);
	bool codeFound = false;
	// Code cache stores the code values we calculate for each depth
	uint32 codeCache[MAX_CODE_BITS + 1] = { 0 };
	bool codeCached[MAX_CODE_BITS + 1] = { 0 };

	for (uint16 i = 0; i < 256; ++i)
	{
		// If the code is too big to fit in the remaining
		//    input buffer skip it.
		if (codeMap[i].depth > pInSizeBits) continue;
		if (codeMap[i].depth == 0) continue;
		uint8 depth = codeMap[i].depth;
		uint32 code = codeMap[i].code;

		// If we haven't calculated the code value of the input at
		//    depth then we calculate that now.
		if (!codeCached[depth])
		{
			// Calculate the code at every depth to this depth
			uint8* p = *pIn;
			uint8 bit = *bufferBit;

			// Find smallest code already calculated
			uint8 s = depth;
			for (; s > 0; --s)
				if (codeCached[s]) break;

			// Move the position forward as required.
			bit += s;
			p += bit / 8;
			bit %= 8;

			// Calculate depths to depth
			for (uint8 j = s + 1; j <= depth; ++j)
			{
				codeCache[j] = (codeCache[j - 1] << 1) + (((*p) >> (7 - bit++)) & 1);
				codeCached[j] = true;
				p += (bit >> 3) & 1;
				bit %= 8;
			}
		}
		
		if (codeMap[i].code == codeCache[depth])
		{
			**pOut = (uint8)i;
			codeFound = true;
			*bufferBit += depth;
			*pIn += (*bufferBit) / 8;
			*bufferBit %= 8;
			*bitCount += depth;
			break;
		}
	}

	fatalErrorIf(!codeFound, CORRUPT_ENCODED_FILE);
}

void printBuffer(uint8* buffer, uint64 count)
{
	if (bFlag)
	{
		HuffmanCode code = { 0 };
		code.depth = 8;
		for (uint64 i = 0; i < count; ++i)
		{
			code.code = buffer[i];
			printHuffmanCode(code);
		}
	}
	else
	{
		for (uint64 i = 0; i < count; ++i)
			printf("%c", buffer[i]);
	}
}

void printBufferBits(uint8* buffer, uint64 count)
{
	uint64 bytes = count / 8;
	uint8 bits = count % 8;
	uint8 lastCode = buffer[bytes] >> (8 - bits);

	printBuffer(buffer, bytes);

	HuffmanCode last = { bits, lastCode};
	printHuffmanCode(last);
}

// Returns a huffman code of any remaining bits
HuffmanCode encodeDictionary(HuffmanCode* codeMap, FILE* file)
{
	// Worst case would be 2048 Bytes where average bit
	//    length is 8 plus 32 bytes for a bit mask, 3 bits for the
	//    size of the last byte, leaving 5 bits for any flags.
	// 
	// TODO: I think we could sort the codes in this function to
	//    move them around so we don't need to send as much 
	//    information, however, this would invalidate the Huffman 
	//    tree so maybe we should be doing it when creating the tree..
	stats.encodedDictionaryBits = 33 * 8 + stats.dictionaryBitLength;

	uint8 buffer[2081] = { 0 };


	uint8* bitmapLocation = buffer + 1;
	uint8* dictionaryLocation = bitmapLocation + 32;

	uint8 bitmapBit = 0;
	uint8 dictionaryBit = 0;
	uint64 dictionaryBitCount = 0;

	// We could use another insertCodeIntoBuffer call but the bitmap
	//    is so simple I just did it inline
	HuffmanCode depthCode = { 5, 0 };
	for (uint16 i = 0; i < 256; ++i)
	{
		if (codeMap[i].depth > 0)
		{
			*bitmapLocation = (*bitmapLocation << 1) + 1;
			depthCode.code = codeMap[i].depth;
			insertCodeIntoBuffer(&dictionaryLocation, &dictionaryBitCount, &dictionaryBit, depthCode);
			insertCodeIntoBuffer(&dictionaryLocation, &dictionaryBitCount, &dictionaryBit, codeMap[i]);
		}
		else
		{
			*bitmapLocation = *bitmapLocation << 1;
		}
		if (++bitmapBit == 8)
		{
			++bitmapLocation;
			bitmapBit = 0;
		}
	}

	// Calculate the leftover bits at the end of the full file
	//    we don't add 33 bytes since this doen't affect the outcome.
	uint8 lastByteSize = (dictionaryBitCount + stats.bitsAfterEncoding) % 8;
	// Insert into bits 6-8 of the buffer. 
	buffer[0] += lastByteSize;

	// Write the file
	uint64 bufferSize = (dictionaryBitCount / 8) + 33; // Ignore any leftover bits.
	if(!nFlag) // Skip writing if nFlag set
		fatalErrorIf(bufferSize != fwrite(buffer, sizeof(uint8), bufferSize, file), FILE_WRITE_FAILED);

	stats.encodedDictionaryBits = bufferSize * 8 + dictionaryBit;

	HuffmanCode leftover = { dictionaryBitCount % 8, *dictionaryLocation };
	return leftover;
}

// This function will populate the codeMap provided with the dictionary it finds in the file,
//    it will also advance the bufferLocation past the dictionary, loading more of the file
//    into the buffer if needed. The return value is the number of bits used in the last byte
//    of the file. It is assumed that the buffer has 1024 bytes of capacity, that bufferLocation
//    points to the first element in the buffer, and that codeMap is 0 initialized.
uint8 decodeDictionary(HuffmanCode* codeMap, uint8** bufferLocation, uint8* bufferBit, uint16* bufferCount, uint64* bitCount, FILE* file)
{
	// This only stands true so long as we are not using the first 5 flag bits...
	fatalErrorIf(**bufferLocation > 7, CORRUPT_DICTIONARY);
	fatalErrorIf(*bufferCount < 33, CORRUPT_DICTIONARY);

	uint8* bufferStart = *bufferLocation;

	// The first byte contains 5 currently unused flag bits followed by 
	//    a 3 bit number. TODO: Test... cast shouldn't be needed... could just be vs being dumb
	uint8 finalByteBits = ((*(*bufferLocation)++) << 5) >> 5;
	;

	// The next 32 bytes or 256 bits are a bitmask telling us what byte values have codes.
	// We just set the depths of the codes to 1 if they are included and count them.
	uint16 codeCount = 0;
	codeMap[0].depth = **bufferLocation >> 7; // 1 or 0
	*bufferBit = 1;
	for (uint8 i = 1; i > 0; ++i) // When the uint8 overflows i == 0
	{
		// Who needs if statements anyway
		uint8 exists = ((**bufferLocation) >> (7 - (*bufferBit)++)) & 1;
		codeCount += exists;
		codeMap[i].depth = exists;
		*bufferLocation += ((*bufferBit) >> 3) & 1;
		*bufferBit %= 8;
	}

	uint8 currentCode = 0;
	bool eofFound = (*bufferCount) == 1024 ? false : true;
	// The number of bits to represent the number of bits in a code
	//    where the maximum code depth is MAX_CODE_BITS
	uint16 maxCodeDepthBits = ((uint16)(log2(MAX_CODE_BITS) + 1.0));
	// The maximum size of an entry in the dictionary: n + ceil(log2(n)).
	//    where n is the maximum allowed code depth.
	uint16 maxEntryBits = maxCodeDepthBits + MAX_CODE_BITS;
	// Now read the codes into the codeMap
	for (uint16 i = 0; i < codeCount; ++i)
	{
		// Find next valid code location in codeMap
		while (codeMap[currentCode].depth == 0)
			++currentCode;
		
		// Load more of the file into the buffer if needed.
		if (!eofFound && *bufferLocation + maxEntryBits >= bufferStart + *bufferCount)
		{
			// Copy remaining contents to start of buffer.
			uint16 remaining = ((bufferStart + *bufferCount) - bufferLocation);
			memcpy(bufferStart, *bufferLocation, remaining);
			
			// Read to buffer after remaining content
			*bufferCount = fread(bufferStart + remaining, sizeof(uint8), 1024 - remaining, file) + remaining;
			*bufferLocation = bufferStart;
			
			if (*bufferCount < 1024)
				eofFound = true;
		}

		// There should be at least 1 bit after the depth.
		fatalErrorIf(*bufferLocation + (((*bufferBit) + maxCodeDepthBits + 7) / 8) >= bufferStart + *bufferCount, CORRUPT_DICTIONARY);

		// Read the code into the codeMap
		// Read Depth
		codeMap[currentCode].depth = 0;
		for (uint8 i = 0; i < maxCodeDepthBits; ++i)
		{
			codeMap[currentCode].depth = codeMap[currentCode].depth << 1;
			codeMap[currentCode].depth += ((**bufferLocation) >> (7 - (*bufferBit)++)) & 1;
			*bufferLocation += ((*bufferBit) >> 3) & 1;
			*bufferBit %= 8;
		}

		// There must be at least as many bits left as the depth of the code
		fatalErrorIf((*bufferLocation) + (((*bufferBit) + codeMap[currentCode].depth + 7) / 8) >= bufferStart + *bufferCount, CORRUPT_DICTIONARY);

		// Read Code
		for (uint16 i = 0; i < codeMap[currentCode].depth; ++i)
		{
			codeMap[currentCode].code = codeMap[currentCode].code << 1;
			codeMap[currentCode].code += ((**bufferLocation) >> (7 - (*bufferBit)++)) & 1;
			*bufferLocation += ((*bufferBit) >> 3) & 1;
			*bufferBit %= 8;
		}

		*bitCount += codeMap[currentCode].depth + maxCodeDepthBits;
		++currentCode;
	}

	return finalByteBits;
}

 
// I would like to break this problem down a bit more and
//    refactor this function but realistically there's not
//    enough time to debug it afterwards even if I managed
//    to refactor it in time! I don't want to split it into
//    seperate encode/decode functions as there is so much
//    shared methodology but I think that viewing the problem
//    from a different angle would be fruitful.

// This function performs both encoding and decoding as well
//    as printing the result to the console or a file 
//    1024 bytes at a time. As such it is quite abstract.
void transformInput(HuffmanCode* codeMap, uint64 characterCount)
{
	uint8 bufferBit = 0;
	uint64 bitsCount = 0;
	uint64 count = 0;
	FILE* inFile = 0;
	uint8 inFileBuffer[1024];
	uint16 inFileBufferCount = 0;
	FILE* outFile = 0;
	uint8 outBuffer[1032] = { 0 };
	uint16 outBufferCount = 0;
	uint8* pIn = input;
	uint8* pOut = outBuffer;
	uint8 finalByteSize = 0;

	if (fFlag)
	{
		inFile = fopen(input, "rb");
		fatalErrorIf(inFile == NULL, FILE_NON_EXISTENT);
		pIn = inFileBuffer;
		inFileBufferCount = fread(inFileBuffer, sizeof(uint8), 1024, inFile);
		if (rFlag)
		{
			finalByteSize = decodeDictionary(codeMap, &pIn, &bufferBit, &inFileBufferCount, &bitsCount, inFile);
			stats.dictionaryBitLength = bitsCount + 33 *8;
			bitsCount = 0;
		}
	}

	if (oFlag)
	{
		// No need to open the file if we aren't writing
		if (!nFlag)
		{
			outFile = fopen(output, "wb");
			fatalErrorIf(outFile == NULL, WRITE_FILE_OPEN_FAILED);
		}
		if (!rFlag)
		{
			HuffmanCode leftover = encodeDictionary(codeMap, outFile);
			insertCodeIntoBuffer(&pOut, &bitsCount, &bufferBit, leftover);
		}
	}


	// Since decoding requires fFlag we don't need to worry about
	//    decoding from the command line.
	bool eofFound = (inFileBufferCount < 1024);
	for (; count < characterCount || rFlag; rFlag ? ++pOut : ++pIn) // <- not allowed an if... ternary works I guess
	{
		if (rFlag)
		{
			if(eofFound)
			{
				if (pIn >= inFileBuffer + inFileBufferCount)
					break;
			}
			else if (pIn + 4 >= inFileBuffer + inFileBufferCount)
			{
				// Should always be 4..?
				uint16 remaining = inFileBuffer + inFileBufferCount - pIn;
				memcpy(inFileBuffer, pIn, remaining);
				inFileBufferCount = fread(inFileBuffer + remaining, sizeof(uint8), 1024 - remaining, inFile) + remaining;
				pIn = inFileBuffer;
				if (inFileBufferCount < 1024)
					eofFound = true;
			}
		}
		else if (fFlag && pIn >= (inFileBuffer + inFileBufferCount))
		{
			inFileBufferCount = fread(inFileBuffer, sizeof(uint8), 1024, inFile);
			pIn = inFileBuffer;

			// I think this can only happen if the file changed between
			//    us previously reading it and now since we know how
			//    many bytes we should be reading..?
			fatalErrorIf(inFileBufferCount == 0, UNEXPECTED_ERROR);
		}
		if (pOut - 1024 > outBuffer)
		{
			// Only if nFlag not set
			if(!nFlag) if (oFlag)
				fatalErrorIf(1024 != fwrite(outBuffer, sizeof(uint8), 1024, outFile), FILE_WRITE_FAILED);
			else // for second if
				printBuffer(outBuffer, 1024);
			// Copy bits written past 1024 bytes to the start of the buffer and
			//    move pOut back by 1024 bytes.
			memcpy(outBuffer, outBuffer + 1024, sizeof(uint8) * 4);
			pOut -= 1024;
		}
		if (rFlag)
		{
			// If we are on the last byte and we have read finalByteSize bits we're done.
			if (pIn + 1 >= inFileBuffer + inFileBufferCount && bufferBit >= finalByteSize)
				break;
			decodeCode(&pOut, &pIn, codeMap, (inFileBuffer + inFileBufferCount) - pIn, &bufferBit, &bitsCount);
		}
		else
		{
			insertCodeIntoBuffer(&pOut, &bitsCount, &bufferBit, codeMap[*pIn]);
		}

		count++;
	}

	if (rFlag)
	{
		stats.bitsAfterEncoding = bitsCount;
		bitsCount = count * 8;
		stats.bytesAfterDecoding = count;
	}

	if (fFlag)
		fclose(inFile);

	// If we have a partial byte left over we fill the remaining space with
	//    0's so we don't use whatever happens to be in memory when outputing
	if (!rFlag && bufferBit > 0)
	{
		HuffmanCode c = { 8 - bufferBit, 0 };
		uint64 t = 0; // We don't want to increment the bitCount
		insertCodeIntoBuffer(&pOut, &t, &bufferBit, c);
	}


	// Write remaining bytes to output if nFlag not set
	if(!nFlag) if (oFlag)
	{
		uint64 remainingBytes = ((bitsCount + 7) / 8) % 1024;
		fatalErrorIf(remainingBytes != fwrite(outBuffer, sizeof(uint8), remainingBytes, outFile), FILE_WRITE_FAILED);
		fclose(outFile);
	}
	else // for second if
	{
		printBufferBits(outBuffer, bitsCount % (1024 * 8));
	}
}

void computeAverageCodeLength(CountMap map, HuffmanCode* codeMap)
{
	double total = map.map[0] * codeMap[0].depth;
	stats.dictionaryBitLength = codeMap[0].depth;
	for (uint8 i = 1; i > 0; ++i) // When the uint8 overflows i == 0
	{
		total += map.map[i] * codeMap[i].depth;
		stats.dictionaryBitLength = codeMap[0].depth;
	}

	stats.bitsAfterEncoding = total;
	stats.averageCodeLength = (double)total / (double)map.count;
}


// main controls the program flow by parsing arguments and
//     deciding what functions should run from there.
int main(int argc, char** argv)
{
// Useful for debugging in Visual Studio, if the configuration
//    is set to debug _DEBUG is defined and the input can be
//    modified here instead of on the command line.
#ifdef _DEBUG
	//argc = 4;
	//char* args[] = { "", "-b", "-o", "Hello", "-n" };
	//argc = 6;
	//char* args[] = { "", "-t", "-s", "-b", "-d", "the quick brown fox jumps over the lazy dog"};
	argc = 9;
	char* args[] = { "", "-t", "-s", "-o", "../data/test.txt", "-b", "-d", "-f", "../data/32_unique.txt" };
	//argc = 6;
	//char* args[] = { "", "-s", "-o", "../data/test.txt", "-f", "../data/Lorem.txt" };
	//argc = 5;
	//char* args[] = { "", "-s", "-o", "test.txt", "Hello"};
	//argc = 7;
	//char* args[] = { "", "-s", "-r", "-o", "../data/out.md", "-f", "../data/test.txt" };
	argv = args;
#endif
	// No arguments provided. Display usage message.
	if (argc == 1)
	{
		printf("No input was provided!\n\n");
		printUsage();
		return 0;
	}

	// Set the codepage so I can get the symbols I want in
	//    the console. On linux, it probably won't look right.
	// TODO: linux equivalent
#ifdef _WIN32
	SetConsoleCP(850);
	SetConsoleOutputCP(850);
#endif
	// Parse command line arguments.
	bool multiInputErrorPrinted = false;
	for (uint8 i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
			case 'f':
				fFlag = true;
				break;
			case 'd':
				dFlag = true;
				break;
			case 't':
				tFlag = true;
				break;
			case 'r':
				rFlag = true;
				break;
			case 's':
				sFlag = true;
				break;
			case 'b':
				bFlag = true;
				break;
			case 'n':
				nFlag = true;
				break;
			case 'o':
				oFlag = true;
				fatalErrorIf(++i >= argc, NO_OUTPUT_FILE);
				fatalErrorIf(argv[i][0] == '-', NO_OUTPUT_FILE);
				output = argv[i];
				break;
			default:
				fatalErrorIf(true, UNIMPLEMENTED_ERROR);
			}
		}
		else
		{
			// We only want to run the second if when the first is 
			//    true as it has side-effects
			if (!multiInputErrorPrinted) if(printErrorIf(input != 0, MULTIPLE_INPUTS, SEVERITY_WARNING))
			{
				multiInputErrorPrinted = true;
			}
			input = argv[i];
		}
	}

	printErrorMessageIf(nFlag && bFlag, "-b ignored because of -n", SEVERITY_WARNING);
	printErrorMessageIf(nFlag && oFlag, "-o <filepath> ignored because of -n", SEVERITY_WARNING);
	printErrorMessageIf(bFlag && oFlag, "-b flag only applies to console output, -o specified", SEVERITY_WARNING);

	fatalErrorIf(rFlag && !fFlag, DECODE_CLI_UNSUPPORTED);
	fatalErrorIf(input == 0, NO_INPUT);


	initHFT();
	double duration, start = hFTNow();

	// Perform requested actions.
	CountMap countMap = { 0 };
	HuffmanTree tree;
	HuffmanCode codeMap[256] = { 0 };

	if (!rFlag)
	{
		countMap = createCountMap(input);
		tree = createHuffmanTree(&countMap);

		parseHuffmanTree(codeMap, &tree);
		computeAverageCodeLength(countMap, codeMap);
	}
	
	// This is just for nice formating, it makes every section after the first section
	//    create an extra newline before printing.
	bool firstSection = true;

	if (!nFlag)
	{
		firstSection = false;
		if (oFlag)
			printf(rFlag ? "Decoding Message...\n" : "Encoding Message...\n");
		else
			printf(rFlag ? "Decoded Message: \n" : "Encoded Message: \n");
	}

	transformInput(codeMap, countMap.count);

	// Get time taken
	duration = hFTNow() - start;

	if(!nFlag)
		printf(oFlag ? "\n" : "\n\nDone.\n");


	if (tFlag && !rFlag)
	{
		firstSection ? firstSection = false :  printf("\n");

		printf("Huffman Tree:\n");
		printHuffmanTree(codeMap, &tree);
	}

	if (dFlag)
	{
		firstSection ? firstSection = false : printf("\n");

		printf("Dictionary:");
		for (uint32 i = 0; i < 256; ++i)
		{
			if (countMap.map[i] > 0)
			{
				printf("\n%3u: ", i);
				printHuffmanCode(codeMap[i]);
			}
		}
		printf("\n");
	}

	if (sFlag)
	{
		firstSection ? firstSection = false : printf("\n");


		printf("Statistics:\n");
		if (rFlag)
		{
			printf("----  Input  ----\n");
			uint64 totalBytes = (stats.bitsAfterEncoding + stats.dictionaryBitLength + 7) / 8;
			printf("File Size Before Decoding : %llu Bytes\n", totalBytes);
			
			printf("Dictionary Size           : %llu Bytes", stats.dictionaryBitLength / 8);
			(stats.dictionaryBitLength % 8 > 0) ? printf(" and %llu Bits\n", stats.dictionaryBitLength % 8) : printf("\n");
			
			printf("Digest Size               : %llu Bytes", stats.bitsAfterEncoding / 8);
			(stats.bitsAfterEncoding % 8 > 0) ? printf(" and %llu Bits\n", stats.bitsAfterEncoding % 8) : printf("\n");

			double compressionRatio = (double)totalBytes / (double)(stats.bytesAfterDecoding);
			printf("File Compression Ratio    : %.3f (%.1f%%)\n", compressionRatio, compressionRatio * 100.0);


			printf("\n---- Output  ----\n");
			printf("File Size                 : %llu Bytes\n", stats.bytesAfterDecoding);


			printf("\n---- General ----\n");
		}
		else
		{
			printf("----  Input  ----\n");
			printf("Size Before Encoding      : %llu Bytes\n", stats.bytesBeforeEncoding);

			printf("Shannon Entropy           : %.3f Bits\n", stats.shannonEntropy);


			printf("\n---- Output  ----\n");
			printf("Digest Size               : %llu Bytes", stats.bitsAfterEncoding / 8);
			(stats.bitsAfterEncoding % 8 > 0) ? printf(" and %llu Bits\n", stats.bitsAfterEncoding % 8) : printf("\n");
			double compressionRatio = (double)(stats.bitsAfterEncoding) / (double)(stats.bytesBeforeEncoding * 8);
			printf("Digest Compression Ratio  : %.3f (%.1f%%)\n", compressionRatio, compressionRatio * 100.0);

			if (oFlag)
			{
				printf("Dictionary Size           : %llu Bytes", stats.encodedDictionaryBits / 8);
				(stats.encodedDictionaryBits % 8 > 0) ? printf(" and %llu Bits\n", stats.encodedDictionaryBits % 8) : printf("\n");
				uint64 totalBytes = (stats.bitsAfterEncoding + stats.encodedDictionaryBits + 7) / 8;
				printf("Output File Size          : %llu Bytes\n", totalBytes);
				compressionRatio = (double)(totalBytes) / (double)stats.bytesBeforeEncoding;
				printf("File Compression Ratio    : %.3f (%.1f%%)\n", compressionRatio, compressionRatio * 100.0);
			}


			printf("\n---- Fixed-Length Encoding Alternative ----\n");
			uint8 fleBits = ceil(log2(stats.uniqueBytesUsed));
			uint64 fleMessageBits = stats.bytesBeforeEncoding * fleBits;
			printf("Digest Size               : %llu Bytes", fleMessageBits / 8);
			(fleMessageBits % 8 > 0) ? printf(" and %llu Bits\n", fleMessageBits % 8) : printf("\n");
			compressionRatio = (double)(fleMessageBits) / (double)(stats.bytesBeforeEncoding * 8);
			printf("Digest Compression Ratio  : %.3f (%.1f%%)\n", compressionRatio, compressionRatio * 100.0);
			if (oFlag)
			{
				// 5 bits for flags 3 bits to specify encoding used 16 bytes for bitmask
				//    finally dictionary entries will be fleBits * uniquechars
				uint64 fleDBits = (17 * 8) + fleBits * stats.uniqueBytesUsed;
				printf("Dictionary Size           : %llu Bytes", fleDBits / 8);
				(fleDBits % 8 > 0) ? printf(" and %llu Bits\n", fleDBits % 8) : printf("\n");
				uint64 fleTotalBytes = (fleMessageBits + fleDBits + 7) / 8;
				printf("Output File Size          : %llu Bytes\n", fleTotalBytes);
				compressionRatio = (double)(fleTotalBytes) / (double)stats.bytesBeforeEncoding;
				printf("File Compression Ratio    : %.3f (%.1f%%)\n", compressionRatio, compressionRatio * 100.0);

			}
			

			printf("\n---- General ----\n");
			printf("Average Code Length       : %.3f Bits\n", stats.averageCodeLength);
		}

		// Print the time taken in whatever unit makes the most sense
		printf("Time Taken                : ");
		if (duration >= 1)
		{
			if (duration < 61)
				printf("%.2f Seconds", duration);
			else
			{
				duration /= 60.0;
				uint64 mins = duration;
				duration = duration - mins;
				printf("%llu Minutes and %.2f Seconds", mins, duration);
			}
		}
		else if (duration >= 1e-03)
		{
			duration *= 1e+03;
			printf("%.2f Milli-Seconds", duration);
		}
		else if (duration >= 1e-06)
		{
			duration *= 1e+06;
			printf("%.2f Micro-Seconds", duration);
		}
		else
		{
			duration *= 1e+09;
			printf("%.2f Nano-Seconds", duration);
		}
		printf("\n");
	}


	if(!rFlag)
		destroyHuffmanTree(&tree);
}