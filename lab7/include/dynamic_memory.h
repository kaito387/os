#ifndef DYNAMIC_MEMORY_H
#define DYNAMIC_MEMORY_H

#include "os_type.h"

// 动态分区分配策略
enum AllocationStrategy
{
    FIRST_FIT,  // 首次适应：分配第一个足够大的空闲块
    BEST_FIT    // 最佳适应：分配最小的足够大的空闲块
};

// 内存块头部信息
struct MemoryBlock
{
    int size;               // 块大小（包含头部），单位：字节
    bool allocated;         // 是否已分配
    MemoryBlock *previous;  // 前一个块
    MemoryBlock *next;      // 后一个块
};

class DynamicMemory
{
private:
    MemoryBlock *head;              // 内存块链表头
    char *memoryStart;              // 管理的内存起始地址
    int totalSize;                  // 管理的总内存大小
    AllocationStrategy strategy;    // 当前分配策略
    int allocCount;                 // 分配次数统计
    int freeCount;                  // 释放次数统计

public:
    DynamicMemory();

    // 初始化动态内存管理器
    // start: 内存起始地址
    // size: 内存总大小（字节）
    // strat: 分配策略
    void initialize(char *start, const int size, const AllocationStrategy strat);

    // 分配指定大小的内存（字节）
    // 返回分配的内存地址（指向可用数据区），失败返回nullptr
    void *allocate(const int size);

    // 释放之前分配的内存
    void release(void *ptr);

    // 切换分配策略
    void setStrategy(const AllocationStrategy strat);

    // 获取当前策略名称
    const char *getStrategyName() const;

    // 显示内存使用情况
    void showInfo();

    // 获取空闲块数量和总空闲大小
    int getFreeBlockCount();
    int getTotalFreeSize();

private:
    // 合并相邻的空闲块
    void mergeFreeBlocks(MemoryBlock *block);

    // 首次适应算法：找到第一个满足大小的空闲块
    MemoryBlock *firstFit(const int size);

    // 最佳适应算法：找到最小的满足大小的空闲块
    MemoryBlock *bestFit(const int size);
};

#endif
