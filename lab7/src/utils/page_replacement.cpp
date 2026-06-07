#include "page_replacement.h"
#include "stdio.h"
#include "memory.h"
#include "os_modules.h"

PageReplacementManager::PageReplacementManager()
{
    frames = nullptr;
    frameCount = 0;
    clock = 0;
    strategy = REPLACE_FIFO;
    pageFaultCount = 0;
    pageAccessCount = 0;
    replaceCount = 0;
}

void PageReplacementManager::initialize(const int numFrames, const PageReplacementStrategy strat)
{
    frameCount = numFrames;
    strategy = strat;
    clock = 0;
    pageFaultCount = 0;
    pageAccessCount = 0;
    replaceCount = 0;

    // 从内核物理地址池分配帧数组
    int frameArrayPages = (numFrames * sizeof(Frame) + 4095) / 4096 + 1;
    frames = (Frame *)memoryManager.allocatePages(AddressPoolType::KERNEL, frameArrayPages);
    if (!frames)
    {
        printf("Failed to allocate frames array\n");
        return;
    }

    // 初始化所有帧为空闲
    for (int i = 0; i < numFrames; ++i)
    {
        frames[i].vaddr = 0;
        frames[i].occupied = false;
        frames[i].loadOrder = 0;
        frames[i].lastAccess = 0;
    }

    printf("Page Replacement Manager initialized: %d frames, strategy=%s\n",
           numFrames, getStrategyName());
}

int PageReplacementManager::findFrame(int vaddr)
{
    for (int i = 0; i < frameCount; ++i)
    {
        if (frames[i].occupied && frames[i].vaddr == vaddr)
            return i;
    }
    return -1;
}

int PageReplacementManager::selectFIFO()
{
    // 选择loadOrder最小的帧（最早加载的）
    int victim = -1;
    int minOrder = 0x7fffffff;

    for (int i = 0; i < frameCount; ++i)
    {
        if (frames[i].occupied && frames[i].loadOrder < minOrder)
        {
            minOrder = frames[i].loadOrder;
            victim = i;
        }
    }

    return victim;
}

int PageReplacementManager::selectLRU()
{
    // 选择lastAccess最小的帧（最久未使用的）
    int victim = -1;
    int minAccess = 0x7fffffff;

    for (int i = 0; i < frameCount; ++i)
    {
        if (frames[i].occupied && frames[i].lastAccess < minAccess)
        {
            minAccess = frames[i].lastAccess;
            victim = i;
        }
    }

    return victim;
}

int PageReplacementManager::accessPage(const uint32 vaddr)
{
    ++pageAccessCount;
    ++clock;

    int pageAddr = vaddr & 0xfffff000;  // 页对齐

    // 1. 检查页面是否已在帧中
    int frameIdx = findFrame(pageAddr);
    if (frameIdx >= 0)
    {
        // 页面命中
        frames[frameIdx].lastAccess = clock;
        return frames[frameIdx].vaddr;  // 返回已有的映射
    }

    // 2. 缺页
    ++pageFaultCount;

    // 3. 检查是否有空闲帧
    int freeIdx = -1;
    for (int i = 0; i < frameCount; ++i)
    {
        if (!frames[i].occupied)
        {
            freeIdx = i;
            break;
        }
    }

    if (freeIdx >= 0)
    {
        // 有空闲帧，直接使用
        frames[freeIdx].vaddr = pageAddr;
        frames[freeIdx].occupied = true;
        frames[freeIdx].loadOrder = clock;
        frames[freeIdx].lastAccess = clock;
        return frames[freeIdx].vaddr;
    }

    // 4. 没有空闲帧，需要置换
    ++replaceCount;
    int victim;
    switch (strategy)
    {
    case REPLACE_FIFO:
        victim = selectFIFO();
        break;
    case REPLACE_LRU:
        victim = selectLRU();
        break;
    default:
        victim = selectFIFO();
        break;
    }

    if (victim < 0)
    {
        printf("Page replacement failed: no victim frame\n");
        return 0;
    }

    // 输出被置换的页面信息
    printf("  [%s] Evict page 0x%x (frame %d), load page 0x%x\n",
           getStrategyName(), frames[victim].vaddr, victim, pageAddr);

    // 置换：用新页面替换旧页面
    frames[victim].vaddr = pageAddr;
    frames[victim].loadOrder = clock;
    frames[victim].lastAccess = clock;
    // occupied保持为true

    return frames[victim].vaddr;
}

void PageReplacementManager::setStrategy(const PageReplacementStrategy strat)
{
    strategy = strat;
}

const char *PageReplacementManager::getStrategyName() const
{
    return strategy == REPLACE_FIFO ? "FIFO" : "LRU";
}

void PageReplacementManager::showStatus()
{
    printf("=== Page Replacement Status [%s] ===\n", getStrategyName());
    printf("Frames: %d, Clock: %d\n", frameCount, clock);
    printf("Page Faults: %d, Accesses: %d, Replacements: %d\n",
           pageFaultCount, pageAccessCount, replaceCount);
    if (pageAccessCount > 0)
    {
        printf("Fault Rate: %d/%d = %d%%\n",
               pageFaultCount, pageAccessCount,
               pageFaultCount * 100 / pageAccessCount);
    }
    printf("Frame Table:\n");
    for (int i = 0; i < frameCount; ++i)
    {
        if (frames[i].occupied)
        {
            printf("  Frame[%d]: vaddr=0x%x, loadOrder=%d, lastAccess=%d\n",
                   i, frames[i].vaddr, frames[i].loadOrder, frames[i].lastAccess);
        }
        else
        {
            printf("  Frame[%d]: [FREE]\n", i);
        }
    }
    printf("=========================================\n");
}
