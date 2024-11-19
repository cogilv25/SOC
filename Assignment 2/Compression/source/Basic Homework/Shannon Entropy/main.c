#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#define uint unsigned int

// We are just supporting standard ascii characters here.
#define MAX_DICTIONARY_LENGTH 128


// The dictionary parsing is hopefully robust, I haven't extensively
//    tested it, but, it can deal with monstrosities like this:
char* testInput = "[   [   'a', \"   110   \"  ],[   'b'  , \"000   111   \"]  ,['t',\"011111\"]  ][asfasfa sfakkljlakshf  ";

// Expected format [['a',"1001"], ['b', "1010"] ...]
//    single quotes must contain 1 character, double quotes
//    must only contain 1 or 0, whitespace is ignored.
double getAverageCodeLength(const char* dictionary, const uint* countMap)
{
	// Note: We are going to ignore \0
	if (dictionary[0] != '[') return -1;
	const char* p = dictionary + 1;
	char c;
	unsigned int count = 0;
	double average = 0.0;
	bool endOfEntry = false;
	for (const char* p = dictionary + 1; *p != '\0'; ++p)
	{
		// Skip whitespace
		if(*p == ' ' || *p == '\r' || *p == '\n' || *p == '\t') continue;

		// There must be a , seperator between entries or a ]
		//    to indicate the end of the dictionary.
		if (endOfEntry)
		{
			if (*p == ']') break;

			if(*p != ',')
				return -1;

			endOfEntry = false;
			continue;
		}

		if (*p != '[') return -1;
		{
			// Inside a dictionary entry
			c = 0;
			bool entryComplete = false;
			for (++p; *(p-1) != ']'; ++p)
			{
				// Invalid dictionary
				if(*p == '\0') return -1;

				// Skip whitespace
				if(*p == ' ' || *p == '\r' || *p == '\n' || *p == '\t') continue;

				// Once the entry is complete there should only be whitespace
				//    until ]
				if (entryComplete) return -1;

				if(c == 0)
				{
					// We are expecting a single character surrounded by '
					if(p[0] != '\'') return -1;
					if(p[1] == '\'' || p[1] == '\0') return -1;
					if(p[2] != '\'') return -1;
					c = p[1];
				}
				// We are expecting a comma (possibly surrounded by whitespace)
				//    and then a sequence of 1's and 0's surrounded by " anything
				//    else is invalid.
				bool commaPassed = false;
				bool entryComplete = false;
				for(p += 3; !(entryComplete && *p==']'); ++p)
				{
					// Skip whitespace
					if(*p == ' ' || *p == '\r' || *p == '\n' || *p == '\t') continue;
					// Find Comma
					if(!commaPassed)
					{
						if(*p != ',')
							return -1;

						commaPassed = true;
						continue;
					}
					// Find Double Quote
					if(*p != '"') return -1;
					for (++p; *p != '"'; ++p)
					{
						// Skip whitespace
						if(*p == ' ' || *p == '\r' || *p == '\n' || *p == '\t') continue;
						if(*p != '0' && *p != '1') return -1;
						average += countMap[c];
					}
					count += countMap[c];
					entryComplete = true;
				}
				break;
			}
			endOfEntry = true;
		}
	}
	return average / (double)count;
}

void getCharacterCountMapAndCount(uint* countMap, uint* count, const char* string)
{
	*count = 0;
	for (const char* p = string; *p != '\0'; ++p)
	{
		if (*p < 0)
		{
			// This shouldn't happen so we will just exit
			printf("Invalid string provided!\n");
			return;
		}
		++countMap[*p];
		++*count;
	}
}

double getShannonEntropy(uint count, const uint* countMap)
{
	double entropy = 0.0;
	for (int i = 1; i < 128; ++i)
	{
		if (countMap[i] > 0)
		{
			double p = (double)countMap[i] / (double)count;
			entropy += p * log2(p);
		}
	}

	return -entropy;
}

// Due to the command line removing quotes this program is just
//    going to require internal modification as I only need to
//    run it once..
int main(int argc, char** argv)
{
	// Input because bash removes escaped quotes, which sorta defeats
	//    the point of escaping them
	char* testData[2] = {0};
	testData[0] = "["
		"[' ',\"10    \"],['o',\"0000  \"],['e',\"0001  \"],['h',\"0010  \"],"
		"['r',\"0011  \"],['t',\"01000 \"],['u',\"01001 \"],['a',\"010100\"],"
		"['b',\"010101\"],['c',\"010110\"],['d',\"010111\"],['f',\"011000\"],"
		"['g',\"011001\"],['i',\"011010\"],['j',\"011011\"],['k',\"011100\"],"
		"['l',\"011101\"],['m',\"011110\"],['n',\"011111\"],['p',\"11000 \"],"
		"['q',\"11001 \"],['s',\"11010 \"],['v',\"11011 \"],['w',\"11100 \"],"
		"['x',\"11101 \"],['y',\"11110 \"],['z',\"11111 \"]"
		"]";
	testData[1] = "the quick brown fox jumps over the lazy dog";


#ifndef _DEBUG
	printf("This program must be edited within Visual Studio or some other software to change the input\n");
#endif

	//                                                        __
	// I should have just extended my original program.....  /  \
	//                                                       .  .
	// I'm too far in now...

	// This is all a little odd but it works... The extension task
	//    will make all this redundant anyway...
	uint charCountMap[128];
	uint count;
	memset(charCountMap, 0, sizeof(uint) * 128);

	getCharacterCountMapAndCount(charCountMap, &count, testData[1]);
	double avg = getAverageCodeLength(testData[0], charCountMap);
	double ent = getShannonEntropy(count, charCountMap);

	printf("The average code length in bits of the dictionary provided is: %.3f bits\n", avg);
	printf("Shannon entropy for the string provided is:                    %.3f bits\n", ent);
}