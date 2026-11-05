# FreeRTOS Kernel cho STM32F103C8T6 - Keil ARMCC5

## Trang thai
Thu muc nay chua chua toan bo ma nguon FreeRTOS Kernel. Kernel chinh thuc can duoc lay tu repository FreeRTOS/FreeRTOS-Kernel va pin theo commit/release da duoc nhom kiem thu. Khong sao chep cac file tu nhanh DEVELOPMENT vao project san pham ma chua xac nhan phien ban.

## Port can dung
- MCU: STM32F103C8T6 (Cortex-M3).
- Compiler: Keil ARM Compiler 5 (ARMCC).
- FreeRTOS port: portable/RVDS/ARM_CM3 (khong dung GCC/ARM_CM3).
- Memory manager: portable/MemMang/heap_4.c.

## Cac file kernel can them vao Keil
Tu repository FreeRTOS-Kernel, bo sung cac module:
- tasks.c, queue.c, list.c
- timers.c (Software Timer)
- event_groups.c va stream_buffer.c (neu kernel version/ung dung su dung)
- portable/MemMang/heap_4.c
- portable/RVDS/ARM_CM3/port.c
- cac header trong include/
- portable/RVDS/ARM_CM3/portmacro.h

Header include toi thieu can duoc doi chieu theo kernel release: FreeRTOS.h, task.h, queue.h, list.h, semphr.h, timers.h, projdefs.h, portable.h, StackMacros.h va cac header phu thuoc cua release do. Khong tu tao header thay the neu thieu.

## Keil uVision
Them include path:
- ..\FreeRTOS\Source\include
- ..\FreeRTOS\Source\portable\RVDS\ARM_CM3
- ..\config
- duong dan CMSIS/SPL cua project

Them source vao group FreeRTOS:
- tasks.c, queue.c, list.c, timers.c
- event_groups.c, stream_buffer.c (neu duoc bat/dung)
- heap_4.c
- portable/RVDS/ARM_CM3/port.c

Khong them port.c cua GCC. Kiem tra startup co cac vector SVC_Handler, PendSV_Handler va SysTick_Handler; cac macro trong FreeRTOSConfig.h map cac vector nay dung mot lan duy nhat.

## Cau hinh can xac nhan truoc khi build
1. Dung cung mot release kernel cho toan bo source va header.
2. Xac nhan SystemCoreClock sau khi cau hinh clock (khong mac dinh doan 72 MHz neu firmware chua cau hinh).
3. Xac nhan __NVIC_PRIO_BITS va configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY voi CMSIS/IRQ thuc te. ISR goi API FreeRTOS phai dung muc uu tien duoc phep.
4. Xac nhan heap size voi SRAM 20 KB cua STM32F103C8T6, stack tung task va stack Timer Service.
5. Chi co mot noi quan ly SysTick va cac vector exception.
6. Build bang ARMCC5 va kiem tra map/memory truoc khi merge.

## Luu y
File config/FreeRTOSConfig.h trong repository la cau hinh khoi dau, chua duoc build tren board. Khong coi day la cau hinh da kiem chung.
