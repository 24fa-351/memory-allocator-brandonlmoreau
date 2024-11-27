#include "myMalloc.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

typedef struct MemoryBlock
{
    size_t blockSize;
    int isAvailable;
    struct MemoryBlock* prevBlock;
    struct MemoryBlock* nextBlock;
} MemoryBlock;

static void initializeMemory();
#ifdef _WIN32
#else
static void* allocateMemory(ssize_t size);
#endif
static void addFreeBlock(MemoryBlock* block);
static void removeFreeBlockAt(size_t currentIndex);
static void siftDown(size_t currentIndex);

static size_t parentIndex(size_t currentIndex);
static size_t leftChild(size_t currentIndex);
static size_t rightChild(size_t currentIndex);
static MemoryBlock* findSuitableBlock(size_t blockSize);
static void mergeBlocks(MemoryBlock* block);

static MemoryBlock* freeMemoryList[1024];
static size_t listSize = 0;

#ifdef _WIN32

static void initializeMemory()
{
    static int isInitialized = 0;
    static void* memoryStart = NULL;

    if (!isInitialized)
    {
        memoryStart = VirtualAlloc(NULL, 1024 * 1024, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (memoryStart == NULL)
        {
            fprintf(stderr, "failed\n");
            return;
        }

        MemoryBlock* startBlock = (MemoryBlock*)memoryStart;
       
        startBlock->blockSize = (1024 * 1024) - sizeof(MemoryBlock);
        startBlock->isAvailable = 1;
    
        startBlock->prevBlock = NULL;
        startBlock->nextBlock = NULL;

        addFreeBlock(startBlock);
        isInitialized = 1;
    }
}

#else

static void* allocateMemory(ssize_t size)
{
    void* memoryPointer = sbrk(0);
   
    if (sbrk(size) == (void*)-1)
    {
        return (void*)-1;
    }
    return memoryPointer;
}

static void initializeMemory()
{
    static int isInitialized = 0;
    static void* memoryStart = NULL;

    if (!isInitialized)
    {
        memoryStart = allocateMemory(1024 * 1024);
        if (memoryStart == (void*)-1)
        {
            fprintf(stderr, "failed\n");
            return;
        }

        MemoryBlock* startBlock = (MemoryBlock*)memoryStart;
        startBlock->blockSize = (1024 * 1024) - sizeof(MemoryBlock);
     
        startBlock->isAvailable = 1;
        startBlock->prevBlock = NULL;
        startBlock->nextBlock = NULL;

        addFreeBlock(startBlock);
        isInitialized = 1;
    }
}

#endif

void* myMalloc(size_t blockSize)
{
    if (blockSize == 0)
    {
        return NULL;
    }
    initializeMemory();

    size_t adjustedSize = (blockSize + 7) & ~7;

    MemoryBlock* currentBlock = findSuitableBlock(adjustedSize);
    if (currentBlock == NULL)
    {
        fprintf(stderr, "error, no available %lu\n", (unsigned long)blockSize);
        return NULL;
    }

    if (currentBlock->blockSize >= adjustedSize + sizeof(MemoryBlock) + 8)
    {
        MemoryBlock* splitBlock = (MemoryBlock*)((char*)currentBlock + sizeof(MemoryBlock) + adjustedSize);
        splitBlock->blockSize = currentBlock->blockSize - adjustedSize - sizeof(MemoryBlock);
      
        splitBlock->isAvailable = 1;

        splitBlock->prevBlock = currentBlock;
        splitBlock->nextBlock = currentBlock->nextBlock;
        if (currentBlock->nextBlock != NULL)
        {
            currentBlock->nextBlock->prevBlock = splitBlock;
        }
        currentBlock->nextBlock = splitBlock;
        currentBlock->blockSize = adjustedSize;

        addFreeBlock(splitBlock);
    }

    currentBlock->isAvailable = 0;

    void* memoryPointer = (void*)((char*)currentBlock + sizeof(MemoryBlock));
  
    memset(memoryPointer, 0, currentBlock->blockSize);

    return memoryPointer;
}

void myFree(void* memoryPointer)
{
    if (memoryPointer == NULL)
    {
        return;
    }

    MemoryBlock* currentBlock = (MemoryBlock*)((char*)memoryPointer - sizeof(MemoryBlock));
    if (currentBlock->isAvailable)
    {
        fprintf(stderr, "detected\n");
        return;
    }

    currentBlock->isAvailable = 1;
    addFreeBlock(currentBlock);
  
    mergeBlocks(currentBlock);
}

void* myRealloc(void* memoryPointer, size_t blockSize)
{
    if (memoryPointer == NULL)
    {
        return myMalloc(blockSize);
    }
    if (blockSize == 0)
    {
        myFree(memoryPointer);
        return NULL;
    }

    MemoryBlock* currentBlock = (MemoryBlock*)((char*)memoryPointer - sizeof(MemoryBlock));
    size_t adjustedSize = (blockSize + 7) & ~7;

    if (currentBlock->blockSize >= adjustedSize)
    {
        return memoryPointer;
    }
    else
    {
        void* resizedPointer = myMalloc(blockSize);
        if (resizedPointer == NULL)
        {
            return NULL;
        }
        memcpy(resizedPointer, memoryPointer, currentBlock->blockSize);
        myFree(memoryPointer);
      
        return resizedPointer;
    }
}

static size_t parentIndex(size_t currentIndex)
{
    return (currentIndex - 1) / 2;
}

static size_t leftChild(size_t currentIndex)
{
    return 2 * currentIndex + 1;
}

static size_t rightChild(size_t currentIndex)
{
    return 2 * currentIndex + 2;
}

static void addFreeBlock(MemoryBlock* block)
{
    if (listSize >= 1024)
    {
        fprintf(stderr, "full\n");
        return;
    }
    freeMemoryList[listSize] = block;
  
    size_t currentIndex = listSize;
    listSize++;

    while (currentIndex != 0 && freeMemoryList[parentIndex(currentIndex)]->blockSize > freeMemoryList[currentIndex]->blockSize)
    {
        MemoryBlock* tempBlock = freeMemoryList[currentIndex];
        freeMemoryList[currentIndex] = freeMemoryList[parentIndex(currentIndex)];
        
        freeMemoryList[parentIndex(currentIndex)] = tempBlock;
       
        currentIndex = parentIndex(currentIndex);
    }
}

static void removeFreeBlockAt(size_t currentIndex)
{
    if (listSize == 0 || currentIndex >= listSize)
    {
        fprintf(stderr, "error\n");
        return;
    }

    freeMemoryList[currentIndex] = freeMemoryList[listSize - 1];
    listSize--;

    siftDown(currentIndex);
}

static void siftDown(size_t currentIndex)
{
    size_t smallestIndex = currentIndex;
    size_t leftIndex = leftChild(currentIndex);
    size_t rightIndex = rightChild(currentIndex);

    if (leftIndex < listSize && freeMemoryList[leftIndex]->blockSize < freeMemoryList[smallestIndex]->blockSize)
    {
        smallestIndex = leftIndex;
    }
    if (rightIndex < listSize && freeMemoryList[rightIndex]->blockSize < freeMemoryList[smallestIndex]->blockSize)
    {
        smallestIndex = rightIndex;
    }
    if (smallestIndex != currentIndex)
    {
        MemoryBlock* tempBlock = freeMemoryList[currentIndex];

        freeMemoryList[currentIndex] = freeMemoryList[smallestIndex];
        freeMemoryList[smallestIndex] = tempBlock;
      
        siftDown(smallestIndex);


    }
}

static MemoryBlock* findSuitableBlock(size_t blockSize)
{
    if (listSize == 0)
    {
        return NULL;
    }

    size_t currentIndex;
    for (currentIndex = 0; currentIndex < listSize; currentIndex++)
    {
        if (freeMemoryList[currentIndex]->blockSize >= blockSize)
        {
            break;
        }
    }

    if (currentIndex == listSize)
    {
        return NULL;
    }

    MemoryBlock* currentBlock = freeMemoryList[currentIndex];

    removeFreeBlockAt(currentIndex);
    return currentBlock;
}

static void mergeBlocks(MemoryBlock* block)
{
    MemoryBlock* followingBlock = block->nextBlock;
    if (followingBlock != NULL && followingBlock->isAvailable)
    {
        size_t currentIndex;
        for (currentIndex = 0; currentIndex < listSize; currentIndex++)
        {
            if (freeMemoryList[currentIndex] == followingBlock)
            {
                removeFreeBlockAt(currentIndex);
                break;
            }
        }
        block->blockSize += sizeof(MemoryBlock) + followingBlock->blockSize;

        block->nextBlock = followingBlock->nextBlock;
        if (followingBlock->nextBlock != NULL)
        {
            followingBlock->nextBlock->prevBlock = block;
        }
    }

    MemoryBlock* precedingBlock = block->prevBlock;

    if (precedingBlock != NULL && precedingBlock->isAvailable)
    {
        size_t currentIndex;
        for (currentIndex = 0; currentIndex < listSize; currentIndex++)
        {
            if (freeMemoryList[currentIndex] == precedingBlock)
            {
                removeFreeBlockAt(currentIndex);
                break;
            }
        }//fixc this
        precedingBlock->blockSize += sizeof(MemoryBlock) + block->blockSize;
        precedingBlock->nextBlock = block->nextBlock;

        if (block->nextBlock != NULL)
        {
            block->nextBlock->prevBlock = precedingBlock;
        }
        block = precedingBlock;
    }

    addFreeBlock(block);
}
