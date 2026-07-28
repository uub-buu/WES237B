
#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    unsigned char ascii;
    int count = 0;
} asciiKey;

void countingSort(unsigned int *arry, int size, int exp);
int getMax(const unsigned int * *arry, int size);
void radixSort(unsigned int * arry, unsigned int size);
/**
 * @param bufin       Array of characters to encode
 * @param bufinlen    Number of characters in the array
 * @param pbufout     Pointer to unallocated output array
 * @param pbufoutlen  Pointer to variable where to store output size
 *
 * @return error code (0 is no error)
 **/
int huffman_encode(const unsigned char *bufin,
		   uint32_t bufinlen,
		   unsigned char **pbufout,
		   uint32_t *pbufoutlen);


/**
 * @param bufin       Array of characters to decode
 * @param bufinlen    Number of characters in the array
 * @param pbufout     Pointer to unallocated output array
 * @param pbufoutlen  Pointer to variable where to store output size
 *
 * @return error code (0 is no error)
 **/
int huffman_decode(const unsigned char *bufin,
  		   uint32_t bufinlen,
		   unsigned char **bufout,
		   uint32_t *pbufoutlen);

#endif
