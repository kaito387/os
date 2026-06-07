## 实验要求

实验七的内容为操作系统的内存管理，共包含四个 Assignment：

1. **Assignment 1：实现二级分页机制** — 复现参考代码，实现二级分页机制，并能够在虚拟地址空间中进行内存管理，包括内存的申请和释放。
2. **Assignment 2：实现动态分区算法** — 参照理论课上学习的物理内存分配算法（如 First-Fit、Best-Fit 等），实现动态分区算法。
3. **Assignment 3：实现页面置换算法** — 参照理论课上虚拟内存管理的页面置换算法（如 FIFO、LRU 等），实现页面置换。
4. **Assignment 4：实现虚拟页内存管理** — 复现"虚拟页内存管理"一节的代码，分析虚拟页内存分配的三步过程和虚拟页内存释放。

---

## 实验过程

### Assignment 1 & 4：二级分页机制与虚拟页内存管理

本部分代码已实现在 `src/kernel/memory.cpp` 中。

**二级分页机制的开启**（`MemoryManager::openPageMechanism`）：

1. 将页目录表放置在物理地址 `0x100000`（`PAGE_DIRECTORY`）处。
2. 将线性地址 0~4MB 对应的页表放置在 `0x101000` 处。
3. 对线性地址 0~1MB 建立恒等映射（虚拟地址 = 物理地址），初始化 256 个页表项。
4. 设置第 0 个页目录项指向 0~1MB 的页表。
5. 设置第 768 个页目录项与第 0 个相同（将内核映射到 3GB~4GB 虚拟地址空间）。
6. 设置第 1023 个页目录项指向页目录表本身（用于构造页目录项/页表项的虚拟地址）。
7. 将页目录表地址写入 CR3，设置 CR0 的 PG 位开启分页。

**虚拟页内存分配的三步过程**（`MemoryManager::allocatePages`）：

1. **从虚拟地址池分配连续虚拟页**：调用 `allocateVirtualPages`，从内核虚拟地址池中分配 `count` 个连续虚拟页。
2. **为每个虚拟页分配物理页**：调用 `allocatePhysicalPages`，为每个虚拟页从对应的物理地址池中分配一个物理页。
3. **建立虚拟页到物理页的映射**：调用 `connectPhysicalVirtualPage`，通过 `toPDE` 和 `toPTE` 构造页目录项和页表项的虚拟地址，在其中写入物理页地址。

```cpp
int MemoryManager::allocatePages(enum AddressPoolType type, const int count) {
    // 第一步：从虚拟地址池中分配若干虚拟页
    int virtualAddress = allocateVirtualPages(type, count);
    if (!virtualAddress) {
        return 0;
    }
    
    bool flag;
    int physicalPageAddress;
    int vaddress = virtualAddress;

    // 依次为每一个虚拟页指定物理页
    for (int i = 0; i < count; ++i, vaddress += PAGE_SIZE) {
        flag = false;
        // 第二步：从物理地址池中分配一个物理页
        physicalPageAddress = allocatePhysicalPages(type, 1);
        if (physicalPageAddress) {
            //printf("allocate physical page 0x%x\n", physicalPageAddress);

            // 第三步：为虚拟页建立页目录项和页表项，使虚拟页内的地址经过分页机制变换到物理页内。
            flag = connectPhysicalVirtualPage(vaddress, physicalPageAddress);
        } else {
            flag = false;
        }

        // 分配失败，释放前面已经分配的虚拟页和物理页表
        if (!flag) {
            // 前i个页表已经指定了物理页
            releasePages(type, virtualAddress, i);
            // 剩余的页表未指定物理页
            releaseVirtualPages(type, virtualAddress + i * PAGE_SIZE, count - i);
            return 0;
        }
    }

    return virtualAddress;
}
```

**虚拟页内存释放**（`MemoryManager::releasePages`）：

1. 对每个虚拟页，通过 `vaddr2paddr` 找到对应的物理地址，释放物理页。
2. 将对应的页表项置 0（标记为不存在）。
3. 从虚拟地址池中释放虚拟页。

```cpp
void MemoryManager::releasePages(enum AddressPoolType type, const int virtualAddress, const int count) {
    int vaddr = virtualAddress;
    int *pte;
    for (int i = 0; i < count; ++i, vaddr += PAGE_SIZE) {
        // 第一步，对每一个虚拟页，释放为其分配的物理页
        releasePhysicalPages(type, vaddr2paddr(vaddr), 1);

        // 第二步，设置页表项为不存在，防止释放后被再次使用
        pte = (int *)toPTE(vaddr);
        *pte = 0;
    }

    // 第三步，释放虚拟页
    releaseVirtualPages(type, virtualAddress, count);
}

```

### Assignment 2：动态分区算法

新增文件 `include/dynamic_memory.h` 和 `src/utils/dynamic_memory.cpp`。

实现了 `DynamicMemory` 类，支持两种动态分区分配策略：

- **First-Fit（首次适应）**：从空闲块链表中找到第一个足够大的空闲块进行分配。
- **Best-Fit（最佳适应）**：遍历所有空闲块，选择最小的能满足需求的空闲块。

核心数据结构为 `MemoryBlock`，形成双向链表，每个块包含：
- `size`：块大小（包含头部）
- `allocated`：分配状态
- `previous`/`next`：链表指针

分配时从空闲块中切分所需大小，剩余部分形成新的空闲块（若剩余空间足够）。释放时与相邻空闲块合并。

### Assignment 3：页面置换算法

新增文件 `include/page_replacement.h` 和 `src/utils/page_replacement.cpp`。

实现了 `PageReplacementManager` 类，模拟有限物理帧下的页面调度，支持两种置换策略：

- **FIFO（先进先出）**：选择最早加载的页面进行置换。
- **LRU（最近最少使用）**：选择最久未被访问的页面进行置换。

使用全局 `clock` 计数器记录每次访问的时间戳：
- FIFO 使用 `loadOrder` 记录页面加载时间
- LRU 使用 `lastAccess` 记录页面最近访问时间

测试中使用标准的页面引用序列，对比两种算法的缺页率。

---

## 关键代码

### 1. 二级分页机制开启

```cpp
void MemoryManager::openPageMechanism()
{
    int *directory = (int *)PAGE_DIRECTORY;        // 页目录表指针
    int *page = (int *)(PAGE_DIRECTORY + PAGE_SIZE); // 第一个页表

    memset(directory, 0, PAGE_SIZE);
    memset(page, 0, PAGE_SIZE);

    int address = 0;
    // 线性地址0~1MB恒等映射到物理地址0~1MB
    for (int i = 0; i < 256; ++i) {
        page[i] = address | 0x7;  // U/S=1, R/W=1, P=1
        address += PAGE_SIZE;
    }

    directory[0] = ((int)page) | 0x07;          // 第0个页目录项
    directory[768] = directory[0];               // 3GB内核空间映射
    directory[1023] = ((int)directory) | 0x7;    // 最后一个指向页目录表自身

    asm_init_page_reg(directory);  // 设置CR3，开启分页
}
```

### 2. 页目录项/页表项虚拟地址构造

构造页目录项和页表项的虚拟地址是实现虚拟页内存管理的关键技术。由于开启分页后所有地址都被视为虚拟地址，必须通过特殊构造来访问页目录表和页表。

- **页目录项虚拟地址** `toPDE`：利用第1023个页目录项指向页目录表自身的特性，构造 `31-22位=0x3FF, 21-12位=0x3FF, 11-0位=4*virtual[31:22]`。
- **页表项虚拟地址** `toPTE`：`31-22位=0x3FF, 21-12位=virtual[31:22], 11-0位=4*virtual[21:12]`。

```cpp
int MemoryManager::toPDE(const int virtualAddress)
{
    return (0xfffff000 + (((virtualAddress & 0xffc00000) >> 22) * 4));
}

int MemoryManager::toPTE(const int virtualAddress)
{
    return (0xffc00000 + ((virtualAddress & 0xffc00000) >> 10) 
            + (((virtualAddress & 0x003ff000) >> 12) * 4));
}
```

### 3. 动态分区算法 — First-Fit 与 Best-Fit

```cpp
MemoryBlock *DynamicMemory::firstFit(const int size)
{
    MemoryBlock *current = head;
    int totalRequired = size + sizeof(MemoryBlock);
    while (current) {
        if (!current->allocated && current->size >= totalRequired)
            return current;  // 返回第一个足够大的空闲块
        current = current->next;
    }
    return nullptr;
}

MemoryBlock *DynamicMemory::bestFit(const int size)
{
    MemoryBlock *current = head;
    MemoryBlock *best = nullptr;
    int totalRequired = size + sizeof(MemoryBlock);
    int bestSize = 0x7fffffff;
    while (current) {
        if (!current->allocated && current->size >= totalRequired) {
            int wasteSize = current->size - totalRequired;
            if (wasteSize < bestSize) {
                bestSize = wasteSize;
                best = current;  // 记录浪费最小的空闲块
            }
        }
        current = current->next;
    }
    return best;
}
```

### 4. 页面置换算法 — FIFO 与 LRU

```cpp
int PageReplacementManager::selectFIFO()
{
    int victim = -1;
    int minOrder = 0x7fffffff;
    for (int i = 0; i < frameCount; ++i) {
        if (frames[i].occupied && frames[i].loadOrder < minOrder) {
            minOrder = frames[i].loadOrder;
            victim = i;  // 选择loadOrder最小的（最早加载的）
        }
    }
    return victim;
}

int PageReplacementManager::selectLRU()
{
    int victim = -1;
    int minAccess = 0x7fffffff;
    for (int i = 0; i < frameCount; ++i) {
        if (frames[i].occupied && frames[i].lastAccess < minAccess) {
            minAccess = frames[i].lastAccess;
            victim = i;  // 选择lastAccess最小的（最久未用的）
        }
    }
    return victim;
}
```

---

## 实验结果

### 系统启动与内存初始化

```
open page mechanism
total memory: 133038080 bytes ( 126 MB )
kernel pool
    start address: 0x200000
    total pages: 15984 ( 62 MB )
    bitmap start address: 0x10000
user pool
    start address: 0x4070000
    total pages: 15984 ( 62 MB )
    bit map start address: 0x107CE
kernel virtual pool
    start address: 0xC0100000
    total pages: 15984  ( 62 MB )
    bit map start address: 0x10F9C
```

系统成功探测到 126MB 物理内存，预留内核空间后，将剩余内存等分为内核物理地址池和用户物理地址池（各约 62MB），内核虚拟地址池起始于 `0xC0100000`（3GB+1MB 处）。

### Assignment 1 & 4：虚拟页内存分配与释放

测试用例如下：

```cpp
printf("=== Assignment 1 & 4: Virtual Page Memory Management ===\n");
char *p1 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 100);
char *p2 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 10);
char *p3 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 100);

printf("%x %x %x\n", p1, p2, p3);

memoryManager.releasePages(AddressPoolType::KERNEL, (int)p2, 10);
p2 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 100);

printf("%x\n", p2);

p2 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 10);

printf("%x\n", p2);
printf("=== Virtual Page Test Complete ===\n\n");

```

代码实际输出如下：

```
=== Assignment 1 & 4: Virtual Page Memory Management ===
C0100000 C0164000 C016E000
C01D2000
C0164000
```

分析：
- 分配 100 页 → `0xC0100000`，跨度 `100 × 4096 = 0x64000`
- 分配 10 页 → `0xC0164000`（紧接上一块），跨度 `0xA000`
- 分配 100 页 → `0xC016E000`（紧接上一块），跨度 `0x64000`
- 释放中间的 10 页（`0xC0164000`）
- 再次分配 100 页 → `0xC01D2000`（跨过中间的空隙，在末尾分配）
- 分配 10 页 → `0xC0164000`（重用之前释放的空隙）

这验证了：连续的虚拟地址空间管理、空闲空间的回收与重用均正确工作。

### Assignment 3：页面置换算法对比

使用 15 个页面的引用序列，4 个物理帧进行测试：

| 算法 | 缺页次数 | 访问次数 | 缺页率 |
|------|---------|---------|-------|
| FIFO | 12 | 15 | 80% |
| LRU  | 10 | 15 | 66% |

LRU 缺页率明显低于 FIFO，说明 LRU 利用了时间局部性，保留了更可能被再次访问的页面。从置换日志可见，FIFO 盲目地按加载顺序淘汰，而 LRU 选择最久未使用的页面，更符合实际访问模式。

---

## 总结

### 实验收获

1. **深入理解二级分页机制**：通过实现 `openPageMechanism`、`toPDE`、`toPTE` 等函数，深刻理解了 IA-32 架构下虚拟地址到物理地址的转换过程，特别是如何通过页目录表和页表构造自引用的方式访问页表项。

2. **掌握动态分区算法**：First-Fit 实现简单但可能产生大量外部碎片；Best-Fit 选择最合适的空闲块，减少浪费但需要遍历整个链表。通过内存块链表管理空闲空间、块分裂与合并，理解了 malloc/free 的底层原理。

3. **理解页面置换算法**：FIFO 简单但可能存在 Belady 异常；LRU 性能更好但实现复杂。实验验证了局部性原理在页面调度中的重要性。

### 遇到的问题与解决

1. **编译问题**：在 C++11 的 `-Wnarrowing` 选项下，超过 `INT_MAX` 的十六进制字面量会被视为 `unsigned int`，导致数组初始化的 narrowing conversion 错误。解决方案是将数组类型改为 `unsigned int`。

---

## 注

1. 请在报告首页填写好相关信息。
2. 实验报告需要将必要的实验过程和结果通过截图等方式放入报告内。并且可以在总结处附上自己解决问题的过程。
3. 锻炼实践能力，尽量自主解决遇到的问题，切忌抄袭。
4. 请将实验报告导出为PDF文件，并命名为 **学号+姓名.pdf** (如 `21210001李华.pdf`)
