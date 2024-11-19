#include <stdio.h>
#include <string.h>
#define uint unsigned int


int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Please provide a single string to count the occurance of characters.\n");
		return -1;
	}

	// We will ignore anything less than 0 and 0 itself
	uint occurances[126];
	memset(occurances, 0, sizeof(uint) * 126);
	
	for (char*p = argv[1]; *p != '\0'; ++p)
	{
		if (*p < 0)
		{
			// This shouldn't happen so we will just exit
			printf("Invalid string provided!\n");
			return -1;
		}
		// +1 because we are ignoring 0
		++occurances[*p + 1];
	}

	for (char i = 0; i < 126; ++i)
		if (occurances[i] > 0)
		{
			if (i < 34)
				printf("(%d): %u\n", i-1, occurances[i]);
			else
				printf("   %c: %u\n", i-1, occurances[i]);
		}

	return 0;
}