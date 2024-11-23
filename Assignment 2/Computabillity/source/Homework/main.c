// Technically there are some issues in here, mostly due to the use of malloc and ignoring it failing,
//     however, this is essentially a one-use program and serves it's purpose.
//
//  TODO: Use an arena since we know the upper memory limit of the application at startup.. or potentially 
//        dump everything on the stack, I'm unsure of the limit however, I think it's configurable
//        per compiler which could reduce the portability of the code.

#define _CRT_SECURE_NO_WARNINGS // Noone will use this and I don't feel like jumping through Microsoft's hoops
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define N_FILES 21
#define MAX_BUFFER_SIZE (UINT64)1 << (N_FILES-1)
#define MAX_INT_CHARACTER_LENGTH 32 // The max for a 64 bit int is 19 characters, so 32 is overkill!

typedef struct
{
	__int64* data;
	int count;
} IntBuffer;

typedef struct {
	// For each algorithm we measure a time for sorting the dataset given:
	// a random order, a pre-sorted order, and a reverse order.
	double bubbleTimeTaken[3];
	double insertionTimeTaken[3];
	double mergeTimeTaken[3];

	// For debugging / error checking
	bool bubbleStatus[3];
	bool insertionStatus[3];
	bool mergeStatus[3];

	int count;
} DatasetStats;

enum SORT_COMPARATOR
{
	BUBBLE,
	INSERT
};

enum DIRECTION
{
	ASC = false,
	DSC = true
};

enum TIMER_RESOLUTION
{
	SECOND = 1,
	MILLI = 1000,
	MICRO = 1000000,
	NANO = 1000000000
};

// WIN32 High Frequency Timer
UINT64 timerFQ = 0;

void initHFT()
{
	LARGE_INTEGER t;
	QueryPerformanceFrequency(&t);
	timerFQ = t.QuadPart;
}

double hFTNow(enum TIMER_RESOLUTION r)
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return (t.QuadPart * r) / (double)timerFQ;
}

UINT64 hFTNowDiscrete(enum TIMER_RESOLUTION r)
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return (t.QuadPart * r) / timerFQ;
}


void freeBuffer(IntBuffer buffer)
{
	free(buffer.data);
}

__int64 convertStringToInt(const char* str)
{
	__int64 value = 0;
	__int64 mul = 1;

	for (int i = 0; i < MAX_INT_CHARACTER_LENGTH; ++i)
	{
		if (str[i] == '\0') break; // Detect the end of the nul-terminated string
		if (str[i] == '-' && value == 0) mul = -1; // Detect negative numbers
		if (str[i] < 48 || str[i] > 57) continue; // Ignore nun-numeric chars
		value *= 10;
		value += str[i] - 48;
	}
	return value * mul;
}

bool compareInt64s(__int64 a, __int64 b, enum DIRECTION dir)
{
	if (dir) return a < b;
	else return a > b;
}

bool loadIntBufferFromFile(const char* filename, IntBuffer * buf)
{
	FILE* file = fopen(filename, "r");
	if (file == NULL) return false;

	char buffer[MAX_INT_CHARACTER_LENGTH];
	for (int i = 0; fgets(buffer, MAX_INT_CHARACTER_LENGTH, file); ++i)
	{
		buf->data[i] = convertStringToInt(buffer);
		++buf->count;
	}
	return true;
}

void printBuffer(IntBuffer buf)
{
	printf("\nBuffer:\n");
	for (int i = 0; i < buf.count; ++i)
		printf("%d: %lld\n", i, buf.data[i]);
}


void bubbleSort(IntBuffer* buf, enum DIRECTION dir)
{
	bool sorted = false;
	while (!sorted)
	{
		sorted = true;
		for (int i = 1; i < buf->count; ++i)
		{
			if (compareInt64s(buf->data[i], buf->data[i - 1], dir))
			{
				__int64 t = buf->data[i];
				buf->data[i] = buf->data[i - 1];
				buf->data[i - 1] = t;
				sorted = false;
			}
		}
	}
}

void insertionSort(IntBuffer* buf, enum DIRECTION dir)
{
	for (int i = 1; i < buf->count; ++i)
	{
		for (int j = i - 1; j >= 0; --j)
		{
			if (compareInt64s(buf->data[i], buf->data[j], dir))
			{
				if (j > 0) continue;
				else
				{
					__int64 t = buf->data[j];
					buf->data[j] = buf->data[i];

					for (; j < i; ++j)
					{
						buf->data[i] = buf->data[j + 1];
						buf->data[j + 1] = t;
						t = buf->data[i];
					}
					break;
				}
			}
			else
			{
				++j;
				__int64 t = buf->data[j];
				buf->data[j] = buf->data[i];

				for (; j < i; ++j)
				{
					buf->data[i] = buf->data[j + 1];
					buf->data[j + 1] = t;
					t = buf->data[i];
				}
				break;
			}
		}
	}
}
//   TODO: !IMPORTANT Retest odd-sized arrays
void mergeSort(IntBuffer* buf, enum DIRECTION dir)
{
	// This is a bit annoying in C.. it may be easier to emulate the way
	//   the algorithm works by just doing pairs one at a time until there
	//   are no more pairs and then merge all the pairs to quads,
	//   then the quads to octets, etc until done. Same algorithm just 
	//   without moving things about in memory so much.
	//   If we were multi-threading it might be worth doing it as intended...
	//   Even then it's probably better to just create multiple IntBuffers
	//   pointing to different parts of the full array and then do the final
	//   merge on the main thread one the others finish.


	bool isOdd = (buf->count % 2 == 1);
	// First we sort all pairs
	for (int i = 0; i < buf->count / 2; ++i)
	{
		// In odd length lists we need to sort the last element...
		if (isOdd && i * 2 + 3 == buf->count)
		{
			// These are for readability, they are the indexes
			//    of the three ints in the buffer
			int i1 = i * 2, i2 = i1 + 1, i3 = i2 + 1;
			__int64 t;
			if (compareInt64s(buf->data[i3], buf->data[i2], dir))
			{
				if (compareInt64s(buf->data[i3], buf->data[i1], dir))
				{
					t = buf->data[i1];
					buf->data[i1] = buf->data[i3];
					if (compareInt64s(t, buf->data[i2], dir))
					{
						buf->data[i3] = buf->data[i2];
						buf->data[i2] = t;
					}
					else
					{
						buf->data[i3] = t;
					}
				}
				else
				{
					t = buf->data[i2];
					buf->data[i2] = buf->data[i3];
					if (compareInt64s(t, buf->data[i1], dir))
					{
						buf->data[i3] = buf->data[i1];
						buf->data[i1] = t;
					}
					else
					{
						buf->data[i3] = t;
					}
				}
			}
			else if(compareInt64s(buf->data[i2], buf->data[i1], dir))
			{
				t = buf->data[i1];
				buf->data[i1] = buf->data[i2];
				buf->data[i2] = t;
			}
		}
		// For normal pairs we just swap them if they are in the wrong order
		else if (compareInt64s(buf->data[i * 2 + 1], buf->data[i * 2], dir))
		{
			__int64 t = buf->data[i * 2];
			buf->data[i * 2] = buf->data[i * 2 + 1];
			buf->data[i * 2 + 1] = t;
		}
	}

	// We are done if there is nothing to merge
	if (buf->count < 4)
		return;

	// We will use the term tuple to refer to any pair, quad, octet, etc
	//    from here out.

	// A buffer which we will use as the second buffer in a double buffer
	//    (the first being buf) to merge our tuples. We could
	//    do everything in the one buffer but it would involve moving
	//    elements around and thus no longer be merge sort.
	__int64* buf2 = malloc(sizeof(__int64) * buf->count);
	// The double buffer
	__int64* dblBuf[2];
	dblBuf[0] = buf->data;
	dblBuf[1] = buf2;
	// Now we merge 2 n-sized tuples to 1 m-sized tuple where m = n * 2 
	//    and n is multiplied by 2 each round until n >= count - 1. The
	//    -1 is to account for odd-sized buffers...
	for (int n = 2; n < buf->count - 1; n *= 2)
	{
		int oSBC = 0; // Odd-Sized Buffer Compensation (little hacky)
		for (int i = 0; i < (buf->count / n) / 2; ++i)
		{
			// Increase the range of k on the last iteration of i if the
			//    buffer is odd-sized to include it in the second tuple.
			if (isOdd && (i + 2) * n * 2 > buf->count)
				oSBC = 1;

			int count = 0;
			// These just make the logic easier to follow as we don't need
			//    "i * n * 2" everywhere.
			__int64 *tup1 = dblBuf[0] + i * n * 2;
			__int64 *tup2 = tup1 + n;
			for (int j = 0; j < n; ++j)
			{
				for (int k = count - j; k < n + oSBC; ++k)
				{
					if (compareInt64s(tup1[j], tup2[k], dir))
					{
						dblBuf[1][i * n * 2 + count++] = tup1[j];

						// If we're at the last element of tuple1 we can 
						//    just insert the rest of tuple2.
						if(j + 1 == n)
						{
							for (; k < n + oSBC; ++k)
							{
								dblBuf[1][i * n * 2 + count++] = tup2[k];
							}
						}
							break;
					}
					else
					{
						dblBuf[1][i * n * 2 + count++] = tup2[k];

						// If we're at the last element of tuple2 we can 
						//    just insert the rest of tuple1.
						if (count - j >= n + oSBC)
						{
							for (; j < n; ++j)
								dblBuf[1][i * n * 2 + count++] = tup1[j];
							break;
						}
					}
				}
			}
		}
		// Swap the buffers
		__int64* t = dblBuf[0];
		dblBuf[0] = dblBuf[1];
		dblBuf[1] = t;
	}

	// dblBuf[0] is the sorted array, dblBuf[1] is the
	//    array from the previous step. Which buffer
	//    buf->data points to depends on the number
	//    of steps performed, we just assign it to
	//    the correct one and free the other. This is
	//    not production code where we need to worry
	//    about the passed in buffer being on the stack
	//    or used somewhere else, etc, etc.
	buf->data = dblBuf[0];
	free(dblBuf[1]);
}

bool checkSorted(IntBuffer buf, enum DIRECTION dir)
{
	for (int i = 1; i < buf.count; ++i)
		if ((buf.data[i] - buf.data[i - 1]) * dir < 0)
			return false;

	return true;
}

__int64 generate64BitNumber()
{
	// We are going to generate uint64s then just reinterpret them as
	//    signed int64s to get the full range otherwise the math is
	//    a little tricky..

	// RAND_MAX is 16 bits long on my machine, to be safe we will
	//    rely on it being at least 8 bits. Meaning 8 rounds to
	//    generate a 64 bit integer.
	UINT64 value = 0;

	for (int i = 0; i < 8; ++i)
		value = (value << 8) + rand();

	// No reinterpret_cast in C so we cast the address to a signed pointer
	//    then dereference to get a signed 64 bit value.
	return *((__int64*)(&value));
}

void printBadInputMessage()
{
	printf("Valid inputs are gen or [filename]\n--------------------------------------------\n");
	printf("gen - generates 15 files of varying lengths containing random 64 bit integers in a readable format.\n\n");
	printf("[filename] - Loads the file and sorts it generating statistics about the 3 algorithms (bubble, insertion and merge).\n");
}

// Input should be micro-seconds
void printTimeInBestUnit(double time)
{
	if (time < 1000.0)
	{
		printf("%.1f micro-seconds\n", time);
		return;
	}

	time /= 1000.0;
	if (time < 1000.0)
	{
		printf("%.3f milli-seconds\n", time);
		return;
	}

	time /= 1000.0;
	if (time < 60.0)
	{
		printf("%.3f seconds\n", time);
		return;
	}

	time /= 60.0;
	printf("%.3f minutes\n", time);
}

int main(int argc, char ** argv)
{
	initHFT();
	
#ifdef _DEBUG
	char* test_argv[2] = { "", "data/handmade/14.dat" };
	argv = test_argv;
	argc = 2;
#endif

	if (argc != 2)
	{
		printBadInputMessage();
		return 1;
	}

	bool gen = false;
	const char* gen_str = "gen";
	for (int i = 0; i < 4; ++i)
	{
		if (argv[1][i] != gen_str[i]) break;
		if (i == 3) gen = true;
	}

	if (gen)
	{
		// We are making an assumption that we are
		// running on a little endian system.. on a big
		// endian system we would get the same seed for
		// 2^32 seconds.. But big endian systems are rare.
		int seed[2];
		time((time_t*)seed);
		srand(seed[0]);

		// We will reuse this buffer for each file.
		char filename[16];

		for (int i = 0; i < N_FILES; ++i)
		{
			sprintf(filename, "%02d.dat", i);
			FILE* file = fopen(filename, "w");

			fprintf(file, "%lld", generate64BitNumber());
			for (int j = 1; j < (1 << i); ++j)
				fprintf(file, "\n%lld", generate64BitNumber());

			fclose(file);
		}
	}
	else
	{
		IntBuffer datasets[3] = { 
			{calloc(MAX_BUFFER_SIZE, sizeof(__int64)), 0},
			{calloc(MAX_BUFFER_SIZE, sizeof(__int64)), 0},
			{calloc(MAX_BUFFER_SIZE, sizeof(__int64)), 0}
		};
		DatasetStats stats;
		enum TIMER_RESOLUTION tRes = MICRO;
		enum DIRECTION initialDirection = ASC;

		// Load the file
		if (!loadIntBufferFromFile(argv[1], datasets))
		{
			printf("FATAL ERROR: Could not load file!\n\n");
			printBadInputMessage();
			return 1;
		}

		// Clone buffer twice so we have one for each algorithm
		datasets[1].count = datasets[0].count;
		datasets[2].count = datasets[0].count;
		memcpy(datasets[1].data, datasets[0].data, datasets[0].count * sizeof(__int64));
		memcpy(datasets[2].data, datasets[0].data, datasets[0].count * sizeof(__int64));

		// Collect statistics for random order sorting
		//Bubble
		double start = hFTNow(tRes);
		bubbleSort(&datasets[0], initialDirection);
		double finish = hFTNow(tRes);
		stats.bubbleTimeTaken[0] = finish - start;
		stats.bubbleStatus[0] = checkSorted(datasets[0], initialDirection);

		//Insertion
		start = hFTNow(tRes);
		insertionSort(&datasets[1], initialDirection);
		finish = hFTNow(tRes);
		stats.insertionTimeTaken[0] = finish - start;
		stats.insertionStatus[0] = checkSorted(datasets[1], initialDirection);

		//Merge
		start = hFTNow(tRes);
		mergeSort(&datasets[2], initialDirection);
		finish = hFTNow(tRes);
		stats.mergeTimeTaken[0] = finish - start;
		stats.mergeStatus[0] = checkSorted(datasets[2], initialDirection);


		// Collect statistics for sorted order sorting
		//Bubble
		start = hFTNow(tRes);
		insertionSort(&datasets[0], initialDirection);
		finish = hFTNow(tRes);
		stats.bubbleTimeTaken[1] = finish - start;
		stats.bubbleStatus[1] = checkSorted(datasets[0], initialDirection);

		//Insertion
		start = hFTNow(tRes);
		insertionSort(&datasets[1], initialDirection);
		finish = hFTNow(tRes);
		stats.insertionTimeTaken[1] = finish - start;
		stats.insertionStatus[1] = checkSorted(datasets[1], initialDirection);

		//Merge
		start = hFTNow(tRes);
		mergeSort(&datasets[2], initialDirection);
		finish = hFTNow(tRes);
		stats.mergeTimeTaken[1] = finish - start;
		stats.mergeStatus[1] = checkSorted(datasets[2], initialDirection);


		// Collect statistics for reverse order sorting
		//Bubble
		start = hFTNow(tRes);
		bubbleSort(&datasets[0], !initialDirection);
		finish = hFTNow(tRes);
		stats.bubbleTimeTaken[2] = finish - start;
		stats.bubbleStatus[2] = checkSorted(datasets[0], !initialDirection);

		//Insertion
		start = hFTNow(tRes);
		insertionSort(&datasets[1], !initialDirection);
		finish = hFTNow(tRes);
		stats.insertionTimeTaken[2] = finish - start;
		stats.insertionStatus[2] = checkSorted(datasets[1], !initialDirection);

		//Merge
		start = hFTNow(tRes);
		mergeSort(&datasets[2], !initialDirection);
		finish = hFTNow(tRes);
		stats.mergeTimeTaken[2] = finish - start;
		stats.mergeStatus[2] = checkSorted(datasets[2], !initialDirection);
				
		stats.count = datasets[0].count;

		// Finally display our statistics

		printf("Number of integers in dataset: %d\n", stats.count);
		printf("--------------------------------------------------\n\n");

		// Bubble Sort
		printf("Bubble sort:\nRandom order: %s - ", stats.bubbleStatus[0] ? "Success" : "Failure");
		printTimeInBestUnit(stats.bubbleTimeTaken[0]);

		printf("Pre-Sorted order: %s - ", stats.bubbleStatus[1] ? "Success" : "Failure");
		printTimeInBestUnit(stats.bubbleTimeTaken[1]);

		printf("Reverse-Sorted order: %s - ", stats.bubbleStatus[2] ? "Success" : "Failure");
		printTimeInBestUnit(stats.bubbleTimeTaken[2]);
		printf("--------------------------------------------------\n\n");

		// Insertion Sort
		printf("Insertion sort:\nRandom order: %s - ", stats.insertionStatus[0] ? "Success" : "Failure");
		printTimeInBestUnit(stats.insertionTimeTaken[0]);

		printf("Pre-Sorted order: %s - ", stats.insertionStatus[1] ? "Success" : "Failure");
		printTimeInBestUnit(stats.insertionTimeTaken[1]);

		printf("Reverse-Sorted order: %s - ", stats.insertionStatus[2] ? "Success" : "Failure");
		printTimeInBestUnit(stats.insertionTimeTaken[2]);
		printf("--------------------------------------------------\n\n");

		// Merge Sort
		printf("Merge sort:\nRandom order: %s - ", stats.mergeStatus[0] ? "Success" : "Failure");
		printTimeInBestUnit(stats.mergeTimeTaken[0]);

		printf("Pre-Sorted order: %s - ", stats.mergeStatus[1] ? "Success" : "Failure");
		printTimeInBestUnit(stats.mergeTimeTaken[1]);

		printf("Reverse-Sorted order: %s - ", stats.mergeStatus[2] ? "Success" : "Failure");
		printTimeInBestUnit(stats.mergeTimeTaken[2]);
		printf("--------------------------------------------------\n\n");

		

		// Cleanup although honestly the OS would do it for us in this case
		freeBuffer(datasets[0]);
		freeBuffer(datasets[1]);
		freeBuffer(datasets[2]);
	}
	return 0;
}