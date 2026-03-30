# 实验一：编译内核 / 利用已有内核构建 OS

> **更新说明**：原 slide 基于 Linux 5.10 + i386（32位）架构。  
> **Linux 6.7 起已彻底移除 i386（x86 32位）支持**，本教程更新为：
> - 内核：**Linux 6.19.6**
> - 架构：**x86_64（64位）**
> - 系统：**Ubuntu 24.04 LTS**

---

## 实验要求

独立完成以下 5 个部分：

1. 环境配置
2. 编译 Linux 内核
3. QEMU 启动内核并开启远程调试（GDB）
4. 制作 Initramfs（Hello World）
5. 编译并启动 Busybox

---

## 第一部分：环境配置

### 1.1 安装 Ubuntu 24.04 LTS

推荐使用 Ubuntu 24.04 LTS，可直接安装或使用虚拟机（VMware / VirtualBox）。

### 1.2 更换清华镜像源（可选，加速下载）

Ubuntu 24.04 使用新的 DEB822 格式配置源，文件位于 `/etc/apt/sources.list.d/ubuntu.sources`：

```bash
sudo cp /etc/apt/sources.list.d/ubuntu.sources /etc/apt/sources.list.d/ubuntu.sources.backup
sudo nano /etc/apt/sources.list.d/ubuntu.sources
```

将 `URIs: http://archive.ubuntu.com/ubuntu/` 替换为 `URIs: https://mirrors.tuna.tsinghua.edu.cn/ubuntu/`，保存退出，然后更新：

```bash
sudo apt update
```

### 1.3 安装编译工具链及依赖

```bash
sudo apt install -y \
    build-essential binutils gcc g++ \
    nasm cmake \
    qemu-system-x86 \
    libncurses-dev bison flex \
    libssl-dev libelf-dev \
    gdb
```

> **与原 slide 的差异：**
> - `qemu` → `qemu-system-x86`（包名更正）
> - 去掉 `libc6-dev-i386`（不再需要 32 位库）
> - 新增 `libelf-dev`（6.x 内核编译依赖）

验证安装：

```bash
gcc -v
qemu-system-x86_64 --version
gdb --version
```

---

## 第二部分：编译 Linux 内核

### 2.1 创建实验目录并获取内核源码

```bash
mkdir -p ~/lab1
cd ~/lab1
```

从 [https://www.kernel.org/](https://www.kernel.org/) 下载 Linux 6.19.6，或直接解压已有的压缩包：

```bash
# 如果已有 linux-6.19.6.tar.xz
xz -d linux-6.19.6.tar.xz
tar -xvf linux-6.19.6.tar
cd linux-6.19.6
```

### 2.2 生成默认配置

> **与原 slide 的关键差异：** 原 slide 使用 `make i386_defconfig`（32位），  
> Linux 6.7+ 已移除 i386，**必须改用 x86_64 默认配置**：

```bash
make x86_64_defconfig
```

### 2.3 开启调试信息（menuconfig）

```bash
make menuconfig
```

在图形界面中按如下路径操作：

```
Kernel hacking
  └── Compile-time checks and compiler options
        └── Debug information
              └── 选择 "Generate DWARF Version 4 debuginfo"  ← 按 Y 或 空格选中
```

> **与原 slide 的差异：** 6.x 内核将 debug info 改为多选项（choice），不再是单一复选框。  
> 推荐选 **DWARF Version 4**（与 gdb 兼容性最好）。  
> 也可选 "Rely on the toolchain's implicit default DWARF version"。

保存退出（选 `<Save>` → `<Exit>`）。

### 2.4 编译内核

```bash
make -j$(nproc)
```

编译完成后验证以下两个文件已生成：

```bash
ls -lh arch/x86/boot/bzImage   # 压缩内核镜像
ls -lh vmlinux                  # 带符号表的未压缩内核（供 GDB 使用）
```

---

## 第三部分：QEMU 启动内核并开启远程调试

### 3.1 使用 QEMU 启动内核（挂起等待 GDB）

进入 `~/lab1` 目录：

```bash
cd ~/lab1
```

> **与原 slide 的差异：** `qemu-system-i386` → `qemu-system-x86_64`

```bash
qemu-system-x86_64 \
    -kernel linux-6.19.6/arch/x86/boot/bzImage \
    -s -S \
    -append "console=ttyS0" \
    -nographic
```

QEMU 不会有任何输出——`-S` 表示启动后立即暂停，等待 GDB 连接。

### 3.2 GDB 调试（新开一个终端）

```bash
cd ~/lab1
gdb
```

在 GDB 提示符中依次执行：

```gdb
# 加载符号表
file linux-6.19.6/vmlinux

# 连接 QEMU 的 GDB stub（默认端口 1234）
target remote :1234

# 在 start_kernel 处设置断点
break start_kernel

# 继续运行
c
```

GDB 会在 `start_kernel` 断点处停下，此时 QEMU 窗口也会开始有串口输出。

继续运行 `c` 后，QEMU 输出最终会以 `Kernel panic - not syncing: VFS: Unable to mount root fs` 结束，这是正常的——因为我们还没有提供 initramfs。

---

## 第四部分：制作 Initramfs（Hello World）

### 4.1 编写 Hello World 程序

```bash
cd ~/lab1
cat > helloworld.c << 'EOF'
#include <stdio.h>

void main()
{
    printf("lab1: Hello World\n");
    fflush(stdout);
    /* 打印完后维持在用户态 */
    while(1);
}
EOF
```

> **与原 slide 的关键差异：** 去掉 `-m32` 标志，编译为 x86_64 静态二进制：

```bash
gcc -o helloworld -static helloworld.c
```

验证架构：

```bash
file helloworld
# 应输出：ELF 64-bit LSB executable, x86-64, statically linked ...
```

### 4.2 打包 initramfs

```bash
echo helloworld | cpio -o --format=newc > hwinitramfs
```

### 4.3 启动内核并加载 initramfs

```bash
qemu-system-x86_64 \
    -kernel linux-6.19.6/arch/x86/boot/bzImage \
    -initrd hwinitramfs \
    -s -S \
    -append "console=ttyS0 rdinit=helloworld" \
    -nographic
```

重复第三部分的 GDB 步骤，运行后可在 QEMU 串口输出中看到：

```
lab1: Hello World
```

---

## 第五部分：编译并启动 Busybox

### 5.1 解压 Busybox

```bash
cd ~/lab1
# 解压 busybox-1.33.0（或更新版本）
tar -xf busybox-1.33.0.tar.bz2   # 根据实际文件名调整扩展名
cd busybox-1.33.0
```

### 5.2 配置 Busybox

```bash
make defconfig
make menuconfig
```

在 menuconfig 中：

```
Settings
  └── [*] Build BusyBox as a static binary (no shared libs)  ← 选中 Y
```

> **与原 slide 的关键差异：** 不再需要设置 `-m32 -march=i386` 的 CFLAGS/LDFLAGS，  
> 直接编译 x86_64 静态二进制即可。

### 5.3 编译并安装

```bash
make -j$(nproc)
make install
# 安装结果在 ./_install 目录下
```

### 5.4 制作 Busybox Initramfs

```bash
cd ~/lab1
mkdir -p mybusybox
mkdir -pv mybusybox/{bin,sbin,etc,proc,sys,usr/{bin,sbin}}
cp -av busybox-1.33.0/_install/* mybusybox/

cd mybusybox
```

创建 init 脚本：

```bash
cat > init << 'EOF'
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
echo -e "\nBoot took $(cut -d' ' -f1 /proc/uptime) seconds\n"
exec /bin/sh
EOF

chmod u+x init
```

打包为 cpio.gz：

```bash
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ~/lab1/initramfs-busybox-x86_64.cpio.gz
```

> **与原 slide 的差异：** 文件名改为 `initramfs-busybox-x86_64.cpio.gz` 以反映架构。

### 5.5 用 QEMU 启动 Busybox OS

```bash
cd ~/lab1
qemu-system-x86_64 \
    -kernel linux-6.19.6/arch/x86/boot/bzImage \
    -initrd initramfs-busybox-x86_64.cpio.gz \
    -nographic \
    -append "console=ttyS0"
```

启动后会进入 BusyBox 的 `/bin/sh`，可以运行 `ls`、`echo` 等命令。

退出 QEMU：按 `Ctrl+A`，然后按 `X`。

---

## 新旧版本差异总览

| 项目 | 原 slide（2023）| 更新版（2025+） |
|------|----------------|----------------|
| Linux 版本 | 5.10.169 | **6.19.6** |
| 目标架构 | i386（32位） | **x86_64（64位）** |
| Ubuntu 版本 | 18.04 | **24.04 LTS** |
| 内核配置命令 | `make i386_defconfig` | **`make x86_64_defconfig`** |
| QEMU 命令 | `qemu-system-i386` | **`qemu-system-x86_64`** |
| 编译 helloworld | `gcc -m32 -static ...` | **`gcc -static ...`**（去掉 -m32） |
| BusyBox CFLAGS | `-m32 -march=i386` | **不需要**（默认 x86_64）|
| 调试信息配置 | 单一复选框 | **choice 菜单，选 DWARF4** |
| 包名 | `qemu`, `libc6-dev-i386` | **`qemu-system-x86`**（去掉 i386 库）|
| apt 源配置文件 | `/etc/apt/sources.list` | **`/etc/apt/sources.list.d/ubuntu.sources`** |

---

## 常见问题

**Q: `make x86_64_defconfig` 提示找不到目标？**  
A: 确认在内核源码根目录下执行，且内核版本 ≥ 6.7。

**Q: GDB 连接后 `target remote :1234` 超时？**  
A: 确认 QEMU 已加 `-s -S` 参数（`-s` 开启 gdbserver 在 1234 端口，`-S` 启动时暂停）。

**Q: Busybox 编译报错 `fatal error: asm/unistd.h`？**  
A: 安装缺失的头文件：`sudo apt install linux-libc-dev`。

**Q: QEMU 启动后没有串口输出？**  
A: 确认 `-append` 中包含 `console=ttyS0`，且使用了 `-nographic`。
