#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>   
#include <string.h>
#include <time.h>     

#ifdef USE_MY_MALLOC
#include "myMalloc.h"
#endif

void testSimpleAllocation()
{
    printf("Basic Malloc:\n");
    char* myStr = (char*)malloc(15);
    if (myStr == NULL)
    {
        printf("malloc error\n");
        return;
    }
    strcpy(myStr, "test my string");
    printf("%s\n", myStr);
    free(myStr);
    printf("passed\n|/|/|/|/|/|/|/|/|/|/|/|/|/|\n\n");
}

void testRealloc()
{
    printf("basic realloc:\n");
    char* initialStr = (char*)malloc(10);
    if (initialStr == NULL)
    {
        printf("realloc error\n");
        return;
    }
    strcpy(initialStr, "testing");
    printf("%s\n", initialStr);
    char* resizedStr = (char*)realloc(initialStr, 20);
    if (resizedStr == NULL)
    {
        printf("failed realoc\n");
        free(initialStr);
        return;
    }
    initialStr = resizedStr;
    strcat(initialStr, " string");
    printf("%s\n", initialStr);
    free(initialStr);
    printf("passed\n|/|/|/|/|/|/|/|/|/|/|/|/|/|\n\n");
}

void testMultipleAllocations()
{
    printf("Rerun malloc:\n");
    int* numbers = (int*)malloc(5 * sizeof(int));
    if (numbers == NULL)
    {
        printf("malloc failed\n");
        return;
    }
    for (int i = 0; i < 5; i++)
    {
        numbers[i] = i * i;
    }
    for (int i = 0; i < 5; i++)
    {
        printf("%d ", numbers[i]);
    }
    printf("\n");
    free(numbers);
    printf("passed\n|/|/|/|/|/|/|/|/|/|/|/|/|/|\n\n");
}

void testSmallAllocations()
{
    printf("small malloc:\n");
    char* pointers[64];
    for (int i = 0; i < 64; i++)
    {
        pointers[i] = (char*)malloc(16);
        if (pointers[i] == NULL)
        {
            printf("failed malloc\n");
            for (int j = 0; j < i; j++)
            {
                free(pointers[j]);
            }
            return;
        }
        memset(pointers[i], 'A' + (i % 26), 16);
    }
    for (int i = 0; i < 64; i++)
    {
        free(pointers[i]);
    }
    printf("passed\n|/|/|/|/|/|/|/|/|/|/|/|/|/|\n\n");
}

void testLargeAllocations()
{
    printf("large malloc:\n");
    int* largePtr = (int*)malloc(512 * 1024);
    if (largePtr == NULL)
    {
        printf("failed malloc\n");
        return;
    }
    size_t numInts = (512 * 1024) / sizeof(int);
    for (size_t i = 0; i < numInts; i++)
    {
        largePtr[i] = (int)i;
    }
    free(largePtr);
    printf("passed\n|/|/|/|/|/|/|/|/|/|/|/|/|/|\n\n");
}

void testSameSizeAllocations()
{
    printf("same size malloc:\n");
    int* pointers[50];
    for (int i = 0; i < 50; i++)
    {
        pointers[i] = (int*)malloc(128);
        if (pointers[i] == NULL)
        {
            printf("failed \n");
            for (int index = 0; index < i; index++)
            {
                free(pointers[index]);
            }
            return;
        }
        memset(pointers[i], i, 128);
    }
    for (int i = 0; i < 50; i++)
    {
        free(pointers[i]);
    }
    printf("passed\n|/|/|/|/|/|/|/|/|/|/|/|/|/|\n\n");
}

void testInterleavedAllocFree()
{
    printf("malloc interleaved & freed:\n");
    int* pointers[10];
    for (int i = 0; i < 10; i++)
    {
        pointers[i] = (int*)malloc(64);
        if (pointers[i] == NULL)
        {
            printf("Failed to allocate memory at index %d\n", i);
            for (int j = 0; j < i; j++)
            {
                free(pointers[j]);
            }
            return;
        }
        memset(pointers[i], i, 64);
    }
    for (int i = 0; i < 10; i += 2)
    {
        free(pointers[i]);
    }
    for (int i = 0; i < 5; i++)
    {
        pointers[i] = (int*)malloc(64);
        if (pointers[i] == NULL)
        {
            printf("Failed to allocate memory at index %d\n", i);
            for (int j = 1; j < 10; j += 2)
            {
                free(pointers[j]);
            }
            return;
        }
        memset(pointers[i], i + 10, 64);
    }
    for (int i = 0; i < 10; i++)
    {
        free(pointers[i]);
    }
    printf("passed\n|/|/|/|/|/|/|/|/|/|/|/|/|/|\n\n");
}

void testMemoryPattern()
{
    printf("memory pattern:\n");
    size_t dataSize = 1024;
    unsigned char* dataPtr = (unsigned char*)malloc(dataSize);
    if (dataPtr == NULL)
    {
        printf("failed\n");
        return;
    }
    for (size_t i = 0; i < dataSize; i++)
    {
        dataPtr[i] = (unsigned char)(i % 256);
    }
    for (size_t i = 0; i < dataSize; i++)
    {
        if (dataPtr[i] != (unsigned char)(i % 256))
        {
            printf("mismatch at index %lu\n", (unsigned long)i);
            free(dataPtr);
            return;
        }
    }
    free(dataPtr);
    printf("passed\n|/|/|/|/|/|/|/|/|/|/|/|/|/|\n\n");
}

void testRandomAllocFree()
{
    printf("random malloc & free:\n");
    srand((unsigned int)time(NULL));
    void* pointers[1000];
    int allocCount = 0;

    for (int i = 0; i < 1000; i++)
    {
        int action = rand() % 2;
        if (action == 0 && allocCount < 1000)
        {
            size_t size = rand() % 256 + 1;
            pointers[allocCount] = malloc(size);
            if (pointers[allocCount] == NULL)
            {
                printf("failed\n");
                continue;
            }
            memset(pointers[allocCount], rand() % 256, size);
            allocCount++;
        }
        else if (allocCount > 0)
        {
            int index = rand() % allocCount;
            free(pointers[index]);
            pointers[index] = pointers[allocCount - 1];
            allocCount--;
        }
    }
    for (int i = 0; i < allocCount; i++)
    {
        free(pointers[i]);
    }
    printf("passed\n|/|/|/|/|/|/|/|/|/|/|/|/|/|\n\n");
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        testSimpleAllocation();
        testRealloc();
        testMultipleAllocations();
        testSmallAllocations();
        testLargeAllocations();
        testSameSizeAllocations();
        testInterleavedAllocFree();
        testMemoryPattern();
        testRandomAllocFree();
    }
    else if (argc == 3 && strcmp(argv[1], "-t") == 0)
    {
        int testNum = atoi(argv[2]);
        if (testNum == 1)
        {
            testSimpleAllocation();
        }
        else if (testNum == 2)
        {
            testRealloc();
        }
        else if (testNum == 3)
        {
            testMultipleAllocations();
        }
        else if (testNum == 4)
        {
            testSmallAllocations();
        }
        else if (testNum == 5)
        {
            testLargeAllocations();
        }
        else if (testNum == 6)
        {
            testSameSizeAllocations();
        }
        else if (testNum == 7)
        {
            testInterleavedAllocFree();
        }
        else if (testNum == 8)
        {
            testMemoryPattern();
        }
        else if (testNum == 9)
        {
            testRandomAllocFree();
        }
        else
        {
            printf("Invalid test number\n");
            printf("Usage: %s [-t test_number]\n", argv[0]);
            printf("Test Numbers:\n");
            printf("basic malloc\n");
            printf("basic realloc\n");
            printf("Rerun malloc\n");
            printf("small malloc\n");
            printf("large malloc\n");
            printf("same size malloc\n");
            printf("malloc interleaved & freed\n");
            printf("memory pattern\n");
            printf("random malloc & free\n");
        }
    }
    else
    {
        printf("Usage: %s [-t test_number]\n", argv[0]);
        printf("Test Numbers:\n");
        printf("basic malloc\n");
        printf("basic realloc\n");
        printf("Rerun malloc\n");
        printf("small malloc\n");
        printf("large malloc\n");
        printf("same size malloc\n");
        printf("malloc interleaved & freed\n");
        printf("memory pattern\n");
        printf("random malloc & free\n");
    }
    return 0;
}
