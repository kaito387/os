我不知道我的截图是不是对的，我可以描述一下每个截图的内容。

---

a3-qemu.png

大概内容是：

```plain
start process
I am parent, pid: 1, my child pid: 3
parent exit without waiting, pid: 1
thread exit
first cihld (orphan), pid: 3
recycle zombie process, pid: 3
second child exit, pid: 4
recycle zombie process, pid: 4
```



---

```plain
(gdb) process_watch
=== Running PID: $1 = 1
=== Running Status: $2 = RUNNING
=== Running pageDir: $3 = 0xc0100000
=== allPrograms size: $4 = 5
=== readyPrograms size: $5 = 4
(gdb)
```

a3-exit-entry

---

截图 3 -- exit 释放页表，这个没有必要截图，只是代码，不是运行，所以我们不放这个环节，可以略过。

---

截图 4 — exit 调用 schedule 之前，查看 

```plain
(gdb) process_watch
=== Running PID: $11 = 1
=== Running Status: $12 = DEAD
=== Running pageDir: $13 = 0xc0100000
=== allPrograms size: $14 = 5
=== readyPrograms size: $15 = 4
(gdb)

```

a3-schedule-entry.png

---

进入 wait

```plain
(gdb) process_watch
=== Running PID: $1 = 1
=== Running Status: $2 = RUNNING
=== Running pageDir: $3 = 0xc0100000
=== allPrograms size: $4 = 5
=== readyPrograms size: $5 = 4
(gdb)

```

a3-wait-entry.png

---

```plain
(gdb) show_pcb
$27 = 4
$28 = DEAD
$29 = 1
(gdb) p child->retValue
$30 = 123934
(gdb) p programManager.allPrograms.size()
$31 = 3
(gdb)

```

a3-dead-process.png

其中，show_pcb 定义如下：

```plain
define show_pcb
    p child->pid
    p child->status
    p child->parentPid
end
```

---

```plain
(gdb) p programManager.allPrograms.size()
$32 = 2
(gdb)

```

a3-release.png

---

截图 7 没时间搞了，我们就不做这个了。