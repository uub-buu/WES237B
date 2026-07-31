

#include "huffman.h"
#include <string.h> // Required for strlen
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <queue>
#include <unordered_map>
#include <bitset>
// COMPLETE
// Counting Sort for Radix

std::unordered_map<unsigned char, bitcode> codebook;

bitcode getCode(unsigned char ascii)
{
    return codebook[ascii];
}
void countingSort(asciiKey arry[], int size, int exp)
{

    const int COUNT_LENGTH = 10;

    int count[COUNT_LENGTH] = {0};
    for (int i = 0; i < size; i++)
    {
        int index = (arry[i].count / exp) % 10;
        count[index]++;
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
        count[digit]--;
    }

    memcpy(arry, output, size * sizeof(asciiKey));
}
// COMPLETE
// GetMax function to get max of a frequency table.
int getMax(asciiKey arry[], int size)
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
    while ((max_value / exp) > 0)
    {
        countingSort(arry, size, exp);
        exp *= 10;
    }
}

void generateCodebook(huffnode *node, bitcode code)
{
    // temp bit buffer
    uint8_t bits;
    if (node->data.ascii != '\0')
    {
        codebook.insert({node->data.ascii, code});
        std::cout << node->data.ascii << ": ";
        std::cout << std::bitset<4>(code.bits) << " bitcount=" << (int)code.bitCount << std::endl;
    }
    if (node->leftNode != nullptr)
    {
        bits = (code.bits << 1) + 0;
        generateCodebook(node->leftNode, bitcode{bits, code.bitCount + 1});
    }

    if (node->rightNode != nullptr)
    {
        bits = (code.bits << 1) + 1;
        generateCodebook(node->rightNode, bitcode{bits, code.bitCount + 1});
    }
}

huffnode *huffmanTree(asciiKey table[], int size)
{
    std::priority_queue<huffnode> treeHeap;

    huffnode *left;
    huffnode *right;
    for (int i = 0; i < size; i++)
    {
        treeHeap.push(huffnode{table[i], nullptr, nullptr});
    }

    while (treeHeap.size() > 1)
    {
        // pop the lowest two node
        left = new huffnode(treeHeap.top());
        std::cout << left->data.ascii << ":" << left->data.count << std::endl;
        treeHeap.pop();
        right = new huffnode(treeHeap.top());
        std::cout << right->data.ascii << ":" << right->data.count << std::endl;

        treeHeap.pop();

        asciiKey parentData = {'\0', left->data.count + right->data.count};
        huffnode parent{parentData, left, right};
        std::cout << parent.data.ascii << ":" << parent.data.count << std::endl;

        treeHeap.push(parent);
    }

    huffnode *root = new huffnode(treeHeap.top());

    return root;
}

/**
 * TODO Complete this function
 **/
int huffman_encode(const unsigned char *bufin,
                   unsigned int bufinlen,
                   unsigned char **pbufout,
                   unsigned int *pbufoutlen)
{
    // capture all possible ascii characters
    asciiKey asciiTable[128] = {0};
    unsigned int frequencySize = 0;
    for (unsigned int i = 0; i < bufinlen; i++)
    {
        unsigned int index = static_cast<unsigned int>(bufin[i]);
        if (bufin[i] != '\0')
        {
        }
        asciiTable[index].ascii = bufin[i];
        if (asciiTable[index].count == 0)
        {
            frequencySize++;
        }
        asciiTable[index].count++;
    }
    // filter any 0 values out.
    asciiKey frequencyTable[frequencySize];
    for (unsigned int i = 0, j = 0; i < 128 && j < frequencySize; i++)
    {
        if (asciiTable[i].count != 0)
        {
            frequencyTable[j] = asciiTable[i];
            j++;
        }
    }
    radixSort(frequencyTable, frequencySize);

    huffnode *root = huffmanTree(frequencyTable, frequencySize);
    generateCodebook(root, bitcode{0, 0});
    // generate new output file size.
    unsigned int outputBits = 0;
    for (const auto &data : frequencyTable)
    {
        outputBits += data.count * codebook[data.ascii].bitCount;
    }
    // find total bytes needed for our output
    unsigned int outputBytes = (outputBits + 7) / 8;

    *pbufoutlen = outputBytes;
    // create our new output
    *pbufout = new unsigned char[*pbufoutlen]();
    int bitsfilled = 0;
    uint8_t currentByte = 0;
    // need to go through each character in the file.
    for (int i = 0, byteIndex = 0; i < bufinlen; i++)
    {
        uint8_t bitcode = codebook[bufin[i]].bits;
        int bitCount = codebook[bufin[i]].bitCount;

        uint8_t extractedBit = 0;
        for (int bits = bitCount - 1; bits >= 0; --bits)
        {
            // add bit to our psuedo buffer
            extractedBit = (bitcode >> bits) & 1;
            currentByte = (currentByte << 1) | extractedBit;
            bitsfilled++;
            if (bitsfilled == 8)
            {
                (*pbufout)[byteIndex++] = currentByte;

                currentByte = 0;
                bitsfilled = 0;
            }
        }

        if (bitsfilled > 0)
        {
            currentByte = currentByte << (8 - bitsfilled);
            (*pbufout)[byteIndex] = currentByte; // byteIndex should be at the very end
        }
    }
    for (int i = 0; i < outputBytes; i++)
    {

        std::cout << "compressed bits is: " << std::bitset<8>((*pbufout)[i])<< std::endl;
    }
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
