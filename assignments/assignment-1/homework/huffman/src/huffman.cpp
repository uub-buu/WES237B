
#include "huffman.h"
#include <string.h> // Required for strlen
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <stdio.h>

// COMPLETE
// Counting Sort for Radix
void countingSort(asciiKey *arry, int size, int exp)
{

    const int COUNT_LENGTH = 10;

    int count[COUNT_LENGTH] = {0};
    for (int i = 0; i < size; i++)
    {
        int index = (arry[i].count / exp) % 10;
        count[index] += 1;
    }
    for (int i = 1; i < COUNT_LENGTH; i++)
    {
        count[i] += count[i - 1];
    }
    asciiKey output[size];
    for (int i = size - 1; i > -1; i--)
    {
        int digit = (arry[i].count / exp) % 10;

        output[count[digit] - 1] = arry[i];
        count[digit] -= 1;
    }

    memcpy(arry, output, size * sizeof(asciiKey));
}
// COMPLETE
// GetMax function to get max of a frequency table.
int getMax(asciiKey *arry, int size)
{
    int max = arry[0].count;
    for (int i = 0; i < size; i++)
    {
        if (max < arry[i].count)
        {
            max = arry[i].count;
        }
    }
    return max;
}
void radixSort(asciiKey arry[], unsigned int size)
{
    if (size == 0)
    {
        return;
    }

    int max_value = getMax(arry, size);

    int exp = 1;
    while ((max_value / exp) > 0){
        countingSort(arry, size, exp);
        exp*=10;
    }
}
/**
 * TODO Complete this function
 **/

int huffman_encode(const unsigned char *bufin,
                   unsigned int bufinlen,
                   unsigned char **pbufout,
                   unsigned int *pbufoutlen)
{
    // create initial frequency table that is is size of buffer to avoid overflow
    asciiKey tempFreqTable[bufinlen];
    unsigned int freqTableSize = 0;
    // calculate the frequency of each character and add to a table.
    for(int i = 0; bufin[i] != '\0'; i++){
        tempFreqTable[i].ascii = bufin[i];
        tempFreqTable[i].count+=1;
        freqTableSize++;
    }
    asciiKey freqTable[freqTableSize];
    // create a frequency table of only the characters within our buffer(im sure theres an easier way)
    memcpy(freqTable, tempFreqTable, freqTableSize*sizeof(asciiKey));
	// call radix sort function to sort array by frequency.
    radixSort(freqTable, freqTableSize);
	// print for debugging? 
    for(int i = 0; i < freqTableSize; i++){
        std::cout << freqTable[i].ascii;
    }
    std::cout << std::endl;
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

