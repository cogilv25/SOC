#define _CRT_SECURE_NO_WARNINGS // Noone will use this and I don't feel like jumping through Microsoft's hoops
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_BUFFER_SIZE 16384
#define MAX_INT_CHARACTER_LENGTH 32 // The max for a 64 bit int is 19, so 32 should be fine!

typedef struct
{
	__int64* data;
	int count;
} intBuffer;

enum SORT_COMPARATOR
{
	BUBBLE,
	INSERT
};

enum DIRECTION
{
	ASC = 1,
	DSC = -1
};

void freeBuffer(intBuffer buffer)
{
	free(buffer.data);
}

__int64 convertStringToInt(const char* str)
{
	__int64 value = 0;
	__int64 mul = 1;
	__int64 base = 1; // We don't need this strictly speaking but it makes it a bit easier
	for (int i = 0; i < MAX_INT_CHARACTER_LENGTH; ++i)
	{
		if (str[i] == '\0') break; // Detect the end of the nul-terminated string
		if (str[i] == '-' && value == 0) mul = -1; // Detect negative numbers
		if (str[i] < 48 || str[i] > 57) continue; // Ignore nun-numeric chars
		value += base * (str[i] - 48);
		base *= 10;
	}
	return value * mul;
}

intBuffer loadIntBufferFromFile(const char* filename)
{
	intBuffer iBuf = {0};
	iBuf.data = (__int64*) calloc(MAX_BUFFER_SIZE, sizeof(__int64));

	FILE* file = fopen(filename, "r");
	char buffer[MAX_INT_CHARACTER_LENGTH];
	for (int i = 0; fgets(buffer, MAX_INT_CHARACTER_LENGTH, file); ++i)
	{
		iBuf.data[i] = convertStringToInt(buffer);
		++iBuf.count;
	}
	return iBuf;
}

void printBuffer(intBuffer buf)
{
	for (int i = 0; i < buf.count; ++i)
		printf("%d: %lld\n", i, buf.data[i]);
}


void bubbleSort(intBuffer buf, enum DIRECTION dir)
{
	bool sorted = false;
	while (!sorted)
	{
		sorted = true;
		for (int i = 1; i < buf.count; ++i)
		{
			if ((buf.data[i] - buf.data[i - 1]) * dir < 0)
			{
				int t = buf.data[i];
				buf.data[i] = buf.data[i - 1];
				buf.data[i - 1] = t;
				sorted = false;
			}
		}
	}
}

void insertionSort(intBuffer buf, enum DIRECTION dir)
{
	for (int i = 1; i < buf.count; ++i)
	{
		for (int j = i - 1; j >= 0; --j)
		{
			if ((buf.data[i] - buf.data[j]) * dir < 0)
			{
				if (j > 0) continue;
				else
				{
					int t = buf.data[j];
					buf.data[j] = buf.data[i];

					for (; j < i; ++j)
					{
						buf.data[i] = buf.data[j + 1];
						buf.data[j + 1] = t;
						t = buf.data[i];
					}
					break;
				}
			}
			else
			{
				++j;
				int t = buf.data[j];
				buf.data[j] = buf.data[i];

				for (; j < i; ++j)
				{
					buf.data[i] = buf.data[j + 1];
					buf.data[j + 1] = t;
					t = buf.data[i];
				}
				break;
			}
		}
	}
}

void mergeSort(intBuffer buf, void(*subSortFunction)(intBuffer, enum DIRECTION), enum DIRECTION dir)
{
	subSortFunction(buf, dir);
}

int main()
{
	const char* fileToLoad = "data/14.dat";
	intBuffer buf = loadIntBufferFromFile(fileToLoad);
	mergeSort(buf, insertionSort, ASC);
	printBuffer(buf);
	freeBuffer(buf);
	return 0;
}