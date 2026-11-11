This directory holds the CMSIS core headers used by the project.
Run `scripts/setup_vendor.ps1` on the Windows/Keil machine to populate CMSIS
headers from the locally installed packs. The FreeRTOS kernel is stored under
`middlewares/FreeRTOS/Source`; the project uses the native kernel directly.
Do not mix CMSIS-RTOS2 wrappers and native FreeRTOS source in one project.
