# Keil uVision - huong dan tich hop

## Trang thai hien tai
Repository da co bo khung app va FreeRTOSConfig.h ban dau, nhung chua co day du kernel/CMSIS/SPL va project Keil (.uvprojx). Vi vay chua the build hoac xac nhan firmware.

## Buoc tich hop
1. Chon mot release FreeRTOS Kernel co port RVDS/ARM_CM3 va su dung dong bo source/header cung release.
2. Bo sung kernel vao FreeRTOS/Source theo huong dan trong FreeRTOS/README.md.
3. Bo sung CMSIS, STM32 Standard Peripheral Library, startup va system_stm32f10x.c dung voi STM32F103C8T6.
4. Tao project MDK-ARM/BTL_N13.uvprojx voi device STM32F103C8, ARM Compiler 5, include paths va source groups.
5. Kiem tra startup exception handlers khong bi trung dinh nghia.
6. Build va xu ly warning/error truoc khi merge.

## Cau hinh IRQ
Voi configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5, moi ISR goi API FreeRTOS (vi du xSemaphoreGiveFromISR) phai co muc uu tien NVIC duoc phep theo quy tac Cortex-M: gia tri priority so hoc khong duoc nho hon muc max-syscall. Khong goi API FreeRTOS tu ISR uu tien cao hon gioi han nay.

## Can nhom xac nhan
- Tan so clock he thong va nguon clock.
- Ban FreeRTOS Kernel duoc chon.
- Dinh nghia/phan cong SysTick.
- Dung luong stack tung task va heap.
- Chan PB12/PB13 active level, debounce va cach xu ly EXTI.
