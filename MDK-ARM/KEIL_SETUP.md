# Keil uVision 5 / ARM Compiler 5 project setup

This package contains a `BTL_N13.uvprojx` source-list project; it needs the locally
installed licensed ARM CMSIS core headers and the FreeRTOS kernel sources.
The STM32F10x SPL v3.5.0 source subset is included under `vendor/STM32F10x_SPL`; CMSIS core and FreeRTOS sources are prepared separately.
The current sources have been compiled and linked directly with ARM Compiler 5;
see `../tests/VERIFICATION.md` for build and emulated regression results.

1. Install packs with Keil Pack Installer: STMicroelectronics STM32F1xx DFP,
   `ARM::CMSIS 5.x`, and a compatible FreeRTOS Kernel (for instance sources
   from `ARM::CMSIS-FreeRTOS 10.5.1`). Use **ARM Compiler V5.06**.
2. Run from the **folder that directly contains scripts/, MDK-ARM/, app/** in PowerShell or CMD:
   `powershell -ExecutionPolicy Bypass -File .\scripts\setup_vendor.ps1`
   or supply explicit paths:
   `-CmsisDir "D:\keilc\ARM\PACK\ARM\CMSIS\5.7.0\CMSIS\Core\Include"`
   `-FreeRtosDir "D:\keilc\ARM\PACK\ARM\CMSIS-FreeRTOS\10.5.1\Source"`
   (Actual layout may vary; these are examples.)
   The FIXED ZIP is flat: extract the ZIP into one folder; no extra BTL_N13/ subfolder.
3. Open `MDK-ARM/BTL_N13.uvprojx`, confirm selected device **STM32F103C8**.
   Do not add RTE-generated `cmsis_os2.c`, Keil RTOS2, duplicate system clocks,
   or duplicate startup files; this project uses native FreeRTOS directly.
4. In `Options for Target -> Target`: Xtal irrelevant (HSI used), IROM1:
   `0x08000000`, SIZE `0x0000FC00`; IRAM1: `0x20000000`, SIZE `0x00005000`.
   **Never use 0x10000 IROM1** for this build: the final 1KB is reserved at
   `0x0800FC00` for Flash threshold settings.
   This corrected project also supplies `MDK-ARM/BTL_N13.sct` and uses it in
   **Options -> Linker -> Use Scatter File**, which explicitly sets flash and RAM.
   Do not enable automatic memory layout simultaneously.
5. `Options -> C/C++`: Include paths (relative to MDK-ARM directory):
   `..\bsp\inc;..\hal\inc;..\app\inc;..\middlewares\FreeRTOS\config;..\vendor\CMSIS\Include;..\vendor\STM32F10x_SPL\inc;..\vendor\STM32F10x_SPL\CMSIS\CM3\DeviceSupport\ST\STM32F10x;..\middlewares\FreeRTOS\Source\include;..\middlewares\FreeRTOS\Source\portable\RVDS\ARM_CM3`
   Defines: `STM32F103xB,STM32F10X_MD,USE_STDPERIPH_DRIVER`. SPL include paths are already listed in the project.
6. `Options -> Output`: enable **Create HEX File**. MicroLIB is enabled in
   project; SWD debug and STM32F10x Flash programming algorithm must be
   selected on your machine's physical ST-Link.
7. Build using F7 (or Project -> Rebuild all target files). In case of
   `port.c` missing, verify `middlewares/FreeRTOS/Source/portable/RVDS/ARM_CM3/port.c`.
   In case of missing `core_cm3.h`, verify `vendor/CMSIS/Include/core_cm3.h`.
   For `undefined xTaskCreate`, `vTaskStartScheduler`, `xTimerCreate`, etc.,
   confirm the native kernel Source C files are included in the project.
8. Flash using ST-Link with board powered safely at 3.3V, BOOT0=0.

`hal/src/startup_stm32f10x_md.s` provides the vector table.
`hal/src/hal_stm32f103.c` provides board services, SPL ADC/Flash setup and SystemInit/SystemCoreClock (8 MHz HSI). `hal_gpio.c` and `hal_i2c.c` use SPL APIs.
FreeRTOS handlers: SVC_Handler, PendSV_Handler, SysTick_Handler are mapped
through FreeRTOSConfig.h. Ensure there are **no duplicate handlers**.

## User code groups
- Startup: `hal/src/startup_stm32f10x_md.s`
- Main: `main.c`
- BSP interface and pin map: `bsp/inc/bsp_board.h`, `bsp/inc/bsp_pins.h`
- HAL APIs: `hal/inc/hal_gpio.h`, `hal/inc/hal_i2c.h`
- Vendor SPL: `vendor/STM32F10x_SPL/inc`, `src`
- Hardware: `hal/src/hal_stm32f103.c`, `hal/src/ssd1306.c`
- App: `app/src/app_ipc.c`, `task_sensor.c`, `task_alarm.c`,
  `task_button.c`, `task_oled.c`
- FreeRTOS: `tasks.c`, `queue.c`, `list.c`, `timers.c`, `event_groups.c`,
  `portable/MemMang/heap_4.c`, `portable/RVDS/ARM_CM3/port.c`.

## Static priorities
- FreeRTOS tick 1000 Hz; HSI core clock 8 MHz
- EXTI15_10 priority 6 (lower logical priority than MAX_SYSCALL level 5)
- Alarm 4, Sensor 3, Button 3, OLED 2, Timer service 3
- Task stack sizes in 32-bit words; Kernel heap 10 KiB

This is a functional design reference, not a certified gas alarm. Verify actual
MCU, regulator, flash map, board wiring and sensors before use.

## Corrections in BTL_N13_Keil_Fixed.zip (2026-11-05)
- Set valid IROM1/IRAM1 default flags and a linker scatter file.
- Shortened the `FileName` metadata for `heap_4.c` and `port.c` so Keil
  doesn't try to generate `.\Objects\portable\...\*. __i` in missing nested directories.
- Packaged the project files directly at the ZIP root (script at scripts/).
- Included `MDK-ARM/Objects` and `MDK-ARM/Listings` folders.
- **This project requires external CMSIS and FreeRTOS source packages.** The local
  ARMCC5 build has now been verified; on-board validation remains outstanding.
