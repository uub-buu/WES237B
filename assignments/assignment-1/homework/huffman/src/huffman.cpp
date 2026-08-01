

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
huffnode *root;
unsigned int fileSize;
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
        //std::cout << node->data.ascii << ": ";
        //std::cout << std::bitset<16>(code.bits) << " bitcount=" << (int)code.bitCount << std::endl;
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
        treeHeap.pop();
        right = new huffnode(treeHeap.top());
        treeHeap.pop();

        asciiKey parentData = {'\0', left->data.count + right->data.count};
        huffnode parent{parentData, left, right};

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
    fileSize = bufinlen;
    // capture all possible ascii characters
    asciiKey asciiTable[128] = {0};
    unsigned int frequencySize = 0;
    for (unsigned int i = 0; i < bufinlen; i++)
    {
        unsigned int index = static_cast<unsigned int>(bufin[i]);
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

    root = huffmanTree(frequencyTable, frequencySize);

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
            uint8_t updatedByte = (currentByte << 1) | extractedBit;
            currentByte = updatedByte;

            bitsfilled++;
            if (bitsfilled == 8)
            {
                (*pbufout)[byteIndex] = currentByte;
                byteIndex++;
                currentByte = 0;
                bitsfilled = 0;
            }
        }
    }
    if (bitsfilled > 0)
    {
        currentByte = currentByte << (8 - bitsfilled);
        (*pbufout)[*pbufoutlen - 1] = currentByte; // byteIndex should be at the very end
    }

    return 0;
}
// maybe we return true?
bool decode(huffnode *parent, huffnode *child, uint8_t bit)
{
    // check left and right node until leaf node is encountered(points to nullptr)
    //  if node bit matches then what? need to return it I guess.
    //  temp bit buffer
    huffnode *ptr = child;
    uint8_t trackedBit = 2;
    // setting to some arbitrary value
    // prevent matching before node position is determined

    // check left first
    if (ptr == parent->leftNode)
    {
        trackedBit = 0;
    }

    if (ptr == parent->rightNode)
    {
        trackedBit = 1;
    }

    if (bit == trackedBit)
    {
        return true;
    }
    else
    {
        return false;
    }
}
/**
 * TODO Complete this function
 **/
int huffman_decode(const unsigned char *bufin,
                   unsigned int bufinlen,
                   unsigned char **pbufout,
                   unsigned int *pbufoutlen)
{
    uint8_t currentByte = 0;
    huffnode *ptr = root;
    // ideally we should append the coded file with the original file size in the first byte
    // this would allow us to decode without needing to know the file size
    // we also need appended the code file with some variation of the huffman tree
    // all this would be done later on
    *pbufoutlen = fileSize;
    *pbufout = new unsigned char[*pbufoutlen]();
    unsigned int originalIndex = 0;
    // going through our coded file 1 byte at a time
    int bitShift;
    for (int i = 0; i < bufinlen; i++)
    {
        bitShift = 7;
        uint8_t extracted = 0;
        currentByte = bufin[i]; // get the first byte in the coded file
        while (bitShift >= 0)
        {
            if (ptr->data.ascii != '\0')
            {

                (*pbufout)[originalIndex++] = ptr->data.ascii;
                //std::cout << "byte " << originalIndex << ": " << (*pbufout)[originalIndex++] << std::endl;
                ptr = root;
            }
            else
            {
                extracted = (currentByte >> bitShift--) & 1;
                bool bitMatched = decode(ptr, ptr->leftNode, extracted);
                if (bitMatched)
                {
                    ptr = ptr->leftNode;
                }
                else
                {
                    ptr = ptr->rightNode;
                }
                if (originalIndex == (*pbufoutlen) - 1 && ptr->data.ascii != '\0')
                {

                    (*pbufout)[originalIndex] = ptr->data.ascii;
                    ptr = root;
                }
            }
        }
    }

    return 0;
}
