# BTL_N13
De tai: He thong giam sat va canh bao ro ri khi GAS su dung STM32F103C8T6 va FreeRTOS.

## Cau truc
- Core/: main va khai bao he thong
- CMSIS/: thu vien CMSIS cho Cortex-M3
- StdPeriph_Driver/: STM32 Standard Peripheral Library
- FreeRTOS/: kernel va port cho Keil/ARMCC
- bsp/: cau hinh phan cung board
- drivers/: driver ngoai vi va thiet bi
- app/: cac task va giao tiep IPC
- config/: cau hinh ung dung va FreeRTOS
- startup/: startup va system file
- MDK-ARM/: project Keil uVision
- docs/: tai lieu thiet ke
- tests/: tai lieu kiem thu

Luu y: cac thu vien CMSIS, SPL, FreeRTOS va startup can duoc bo sung dung phien ban tuong thich voi project Keil cua nhom. Khong commit cac file build.
