#include "huffman.h"
#include <string.h> // Required for strlen
#include <math.h>
// COMPLETE
//Counting Sort for Radix
bool countingSort(int arry[], int size, int exp)
{
	const int COUNT_LENGTH = 10;
	if (size == 0)
	{
		return false;
	}
	int count[COUNT_LENGTH];
	for (int i = 0; i < size; i++)
	{
		int index = (arry[i] / exp) % 10;
		count[index] += 1;
	}
	// compute prefix
	for (int i = 0; i < COUNT_LENGTH; i++)
	{
		count[i] += count[i - 1];
	}

	int output[size];
	for (int i = size - 1; i > -1; i--)
	{
		int digit = (arry[i] / exp) % 10;
		output[count[digit] - 1] = arry[i];
		count[digit] -= 1;
	}
	return output;
}
// COMPLETE 
// GetMax function to get max of an array. 
int getMax(int arry[], int size)
{
	int max = arry[0];
	for (int i = 0; i < size; i++)
	{
		if (max < arry[i])
		{
			max = arry[i];
		}
	}
	return max;
}
/**
 * TODO Complete this function
 **/

int huffman_encode(const unsigned char *bufin,
				   unsigned int bufinlen,
				   unsigned char **pbufout,
				   unsigned int *pbufoutlen)
{
	// 1. calculate freq of each character
	// 2. organize by most frequent(radix sort)
	// d = (num  // exp) % 0;
	// 3. start with least frequent  on right two leafnodes
	// 4. parent of these nodes will be addition of both frequencies
	// 5. move to the left most least frequent leaf nodes
	// 6. take the parent of the rights and least frequent left node on the right and add
	// 7. this is their parent
	// move to the left for most frequent node and add left
	// need to compare frequency with aded frequencies as well. most frequent is on the right least on the left

	return 0;
}

/**
 * TODO Complete this function
 **/
int huffman_decode(const unsigned char *bufin,
				   unsigned int bufinlen,
				   unsigned char **pbufout,
				   unsigned int *pbufoutlen)
{
	return 0;
}
