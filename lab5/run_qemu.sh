#!/bin/bash

# 使用 qemu 模拟器运行内核
# 使用 -display curses 来显示串行输出
cd "$(dirname "$0")"

qemu-system-i386 \
    -hda ./run/hd.img \
    -monitor stdio \
    -nographic \
    -no-reboot \
    2>&1
