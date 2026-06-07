#include "dynamic_memory.h"
#include "stdio.h"
#include "stdlib.h"

DynamicMemory::DynamicMemory()
{
    head = nullptr;
    memoryStart = nullptr;
    totalSize = 0;
    strategy = FIRST_FIT;
    allocCount = 0;
    freeCount = 0;
}

void DynamicMemory::initialize(char *start, const int size, const AllocationStrategy strat)
{
    memoryStart = start;
    totalSize = size;
    strategy = strat;
    allocCount = 0;
    freeCount = 0;

    // 创建一个初始的空闲块，占据全部内存
    head = (MemoryBlock *)memoryStart;
    head->size = totalSize;
    head->allocated = false;
    head->previous = nullptr;
    head->next = nullptr;
}

void *DynamicMemory::allocate(const int size)
{
    if (size <= 0)
        return nullptr;

    MemoryBlock *target = nullptr;

    // 根据策略选择空闲块
    switch (strategy)
    {
    case FIRST_FIT:
        target = firstFit(size);
        break;
    case BEST_FIT:
        target = bestFit(size);
        break;
    default:
        target = firstFit(size);
        break;
    }

    if (!target)
        return nullptr;

    // target指向一个空闲块，需要从中切分出需要的部分
    int requiredSize = size + sizeof(MemoryBlock);
    int remainingSize = target->size - requiredSize;

    if (remainingSize >= (int)(sizeof(MemoryBlock) + 8))
    {
        // 剩余空间足够大，进行切分
        // 在target后面创建新的空闲块
        MemoryBlock *newFreeBlock = (MemoryBlock *)((char *)target + requiredSize);
        newFreeBlock->size = remainingSize;
        newFreeBlock->allocated = false;
        newFreeBlock->previous = target;
        newFreeBlock->next = target->next;

        if (target->next)
        {
            target->next->previous = newFreeBlock;
        }

        target->next = newFreeBlock;
        target->size = requiredSize;
    }
    // 如果剩余空间不够大，就将整个块分配出去

    target->allocated = true;
    ++allocCount;

    // 返回分配的内存地址（跳过块头部）
    return (void *)((char *)target + sizeof(MemoryBlock));
}

void DynamicMemory::release(void *ptr)
{
    if (!ptr)
        return;

    // 找到块头部
    MemoryBlock *block = (MemoryBlock *)((char *)ptr - sizeof(MemoryBlock));

    if (!block->allocated)
    {
        printf("Error: double free detected at 0x%x\n", (int)ptr);
        return;
    }

    block->allocated = false;
    ++freeCount;

    // 尝试合并相邻的空闲块
    mergeFreeBlocks(block);
}

void DynamicMemory::mergeFreeBlocks(MemoryBlock *block)
{
    // 合并后面的空闲块
    MemoryBlock *nextBlock = block->next;
    while (nextBlock && !nextBlock->allocated)
    {
        block->size += nextBlock->size;
        block->next = nextBlock->next;
        if (nextBlock->next)
        {
            nextBlock->next->previous = block;
        }
        nextBlock = block->next;
    }

    // 合并前面的空闲块
    MemoryBlock *prevBlock = block->previous;
    while (prevBlock && !prevBlock->allocated)
    {
        prevBlock->size += block->size;
        prevBlock->next = block->next;
        if (block->next)
        {
            block->next->previous = prevBlock;
        }
        block = prevBlock;
        prevBlock = block->previous;
    }
}

MemoryBlock *DynamicMemory::firstFit(const int size)
{
    MemoryBlock *current = head;
    int totalRequired = size + sizeof(MemoryBlock);

    while (current)
    {
        if (!current->allocated && current->size >= totalRequired)
        {
            return current;
        }
        current = current->next;
    }

    return nullptr;
}

MemoryBlock *DynamicMemory::bestFit(const int size)
{
    MemoryBlock *current = head;
    MemoryBlock *best = nullptr;
    int totalRequired = size + sizeof(MemoryBlock);
    int bestSize = 0x7fffffff;  // 一个很大的数

    while (current)
    {
        if (!current->allocated && current->size >= totalRequired)
        {
            int wasteSize = current->size - totalRequired;
            if (wasteSize < bestSize)
            {
                bestSize = wasteSize;
                best = current;
            }
        }
        current = current->next;
    }

    return best;
}

void DynamicMemory::setStrategy(const AllocationStrategy strat)
{
    strategy = strat;
}

const char *DynamicMemory::getStrategyName() const
{
    return strategy == FIRST_FIT ? "First-Fit" : "Best-Fit";
}

int DynamicMemory::getFreeBlockCount()
{
    int count = 0;
    MemoryBlock *current = head;
    while (current)
    {
        if (!current->allocated)
            ++count;
        current = current->next;
    }
    return count;
}

int DynamicMemory::getTotalFreeSize()
{
    int total = 0;
    MemoryBlock *current = head;
    while (current)
    {
        if (!current->allocated)
            total += current->size - sizeof(MemoryBlock);
        current = current->next;
    }
    return total;
}

void DynamicMemory::showInfo()
{
    printf("=== Dynamic Memory Info ===\n");
    printf("Strategy: %s\n", getStrategyName());
    printf("Total size: %d bytes\n", totalSize);
    printf("Alloc count: %d, Free count: %d\n", allocCount, freeCount);
    printf("Free blocks: %d, Total free: %d bytes\n", getFreeBlockCount(), getTotalFreeSize());
    printf("Block list:\n");

    MemoryBlock *current = head;
    int idx = 0;
    while (current)
    {
        printf("  [%d] addr=0x%x, size=%d, %s\n",
               idx, (int)current, current->size,
               current->allocated ? "ALLOCATED" : "FREE");
        current = current->next;
        ++idx;
    }
    printf("==========================\n");
}
