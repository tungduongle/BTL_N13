#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* STM32F103C8T6 Blue Pill, HSI 8 MHz, Cortex-M3 (4 NVIC priority bits).
 * Native FreeRTOS API, NOT CMSIS-RTOS2 adapter. */
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                 1
#define configCPU_CLOCK_HZ                     (8000000UL)
#define configTICK_RATE_HZ                     (1000U)
#define configMAX_PRIORITIES                   6
#define configMINIMAL_STACK_SIZE               128U
#define configTOTAL_HEAP_SIZE                  (10U * 1024U)
#define configMAX_TASK_NAME_LEN                12
#define configUSE_16_BIT_TICKS                 0
#define configIDLE_SHOULD_YIELD                1
#define configUSE_MUTEXES                      1
#define configUSE_RECURSIVE_MUTEXES            0
#define configUSE_COUNTING_SEMAPHORES          0
#define configQUEUE_REGISTRY_SIZE              0
#define configUSE_TASK_NOTIFICATIONS           1
#define configCHECK_FOR_STACK_OVERFLOW         2
#define configUSE_MALLOC_FAILED_HOOK           1
#define configUSE_IDLE_HOOK                    0
#define configUSE_TICK_HOOK                    0
#define configUSE_TIMERS                       1
#define configTIMER_TASK_PRIORITY              (tskIDLE_PRIORITY + 3)
#define configTIMER_QUEUE_LENGTH               8
#define configTIMER_TASK_STACK_DEPTH           256
#define configSUPPORT_DYNAMIC_ALLOCATION       1
#define configSUPPORT_STATIC_ALLOCATION        0
#define configUSE_TRACE_FACILITY               0
#define configUSE_STATS_FORMATTING_FUNCTIONS   0
#define configGENERATE_RUN_TIME_STATS          0

#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY         (15 << (8-configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (5 << (8-configPRIO_BITS))

#define INCLUDE_vTaskDelay                     1
#define INCLUDE_vTaskDelayUntil                1
#define INCLUDE_uxTaskGetStackHighWaterMark    1
#define INCLUDE_xTaskGetSchedulerState         1
#define INCLUDE_vTaskDelete                    0
#define INCLUDE_vTaskSuspend                   1
#define INCLUDE_xTimerPendFunctionCall         0

/* Native FreeRTOS handlers in port.c; startup vector labels. */
#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

void n13_assert_failed(const char *file, int line);
#define configASSERT(x) do { if (!(x)) n13_assert_failed(__FILE__, __LINE__); } while (0)

#endif
