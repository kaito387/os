#ifndef PAGE_REPLACEMENT_H
#define PAGE_REPLACEMENT_H

#include "os_type.h"

// 页面置换策略
enum PageReplacementStrategy
{
    REPLACE_FIFO,  // 先进先出
    REPLACE_LRU    // 最近最少使用
};

// 帧信息
struct Frame
{
    int vaddr;           // 映射到此帧的虚拟页地址
    bool occupied;       // 此帧是否被占用
    int loadOrder;       // 加载序号（用于FIFO）
    int lastAccess;      // 最近访问时间戳（用于LRU）
};

class PageReplacementManager
{
private:
    Frame *frames;                      // 帧数组
    int frameCount;                     // 可用的物理帧数量
    int clock;                          // 全局时钟，用于FIFO和LRU的时间戳
    PageReplacementStrategy strategy;   // 当前置换策略
    int pageFaultCount;                 // 缺页次数
    int pageAccessCount;                // 页面访问次数
    int replaceCount;                   // 置换次数

public:
    PageReplacementManager();

    // 初始化页面置换管理器
    // numFrames: 可用的物理帧数量
    // strat: 置换策略
    void initialize(const int numFrames, const PageReplacementStrategy strat);

    // 访问虚拟页（返回对应的物理页地址）
    // 如果页面不在内存中，触发缺页处理
    // 返回物理页地址，如果无法处理则返回0
    int accessPage(const uint32 vaddr);

    // 切换置换策略
    void setStrategy(const PageReplacementStrategy strat);

    // 获取策略名称
    const char *getStrategyName() const;

    // 显示当前状态
    void showStatus();

    // 获取统计数据
    int getPageFaultCount() const { return pageFaultCount; }
    int getReplaceCount() const { return replaceCount; }
    int getPageAccessCount() const { return pageAccessCount; }

private:
    // FIFO置换：选择最早加载的页面
    int selectFIFO();

    // LRU置换：选择最久未使用的页面
    int selectLRU();

    // 在帧中查找虚拟页
    int findFrame(int vaddr);
};

#endif
