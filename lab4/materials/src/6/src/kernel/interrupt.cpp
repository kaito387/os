#include "interrupt.h"
#include "os_type.h"
#include "os_constant.h"
#include "asm_utils.h"

InterruptManager::InterruptManager()
{
    initialize();
}

void InterruptManager::initialize()
{
    // 初始化IDT
    IDT = (uint32 *)IDT_START_ADDRESS;
    asm_lidt(IDT_START_ADDRESS, 256 * 8 - 1);

    for (uint i = 0; i < 256; ++i)
    {
        setInterruptDescriptor(i, (uint32)asm_my_interrupt_handler, 0);
    }

}

void InterruptManager::setInterruptDescriptor(uint32 index, uint32 address, byte DPL)
{
    // 中断描述符的低32位
    IDT[index * 2] = (CODE_SELECTOR << 16) | (address & 0xffff);
    // 中断描述符的高32位
    IDT[index * 2 + 1] = (address & 0xffff0000) | (0x1 << 15) | (DPL << 13) | (0xe << 8);
}

extern "C" void my_interrupt_handler() {
    const char info[] = "MY interrupt handler is called!";
    volatile uint16 *screen = (uint16 *)0xb8000;

    for (int i = 0; info[i] != '\0'; ++i)
    {
        screen[i] = (0x0a << 8) | (uint8)info[i];
    }
}
