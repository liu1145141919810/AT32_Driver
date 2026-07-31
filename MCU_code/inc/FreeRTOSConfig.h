#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
/* AT32F423 Minimal FreeRTOS Configuration */
/* Scheduler */
#define configUSE_PREEMPTION                    1 
#define configCPU_CLOCK_HZ                      (144000000UL)//CPU frequency, consistent with the system model design
#define configTICK_RATE_HZ                      ((TickType_t)1000)//RTOS tick frequency
#define configMAX_PRIORITIES                    (5)//Task priority range setting
#define configMINIMAL_STACK_SIZE                ((uint16_t)128)//Minimum stack size for a single task
#define configMAX_TASK_NAME_LEN                 (16)//Maximum length of task name
#define configUSE_16_BIT_TICKS                  0//Whether the tick counter uses 32-bit or 16-bit
#define configIDLE_SHOULD_YIELD                 1//Whether the idle task yields when there are other idle-priority tasks

/* Memory Management */
#define configTOTAL_HEAP_SIZE                   ((size_t)(10240))//Kernel-managed dynamic memory pool size: 10KB

/* Optional Features — Disabled for Minimal Version */
#define configUSE_MUTEXES                       0//Whether to use mutexes
#define configUSE_COUNTING_SEMAPHORES           0//When multiple instances of a resource are available, a count value is used to represent the remaining resources
#define configUSE_TIMERS                        0//Software timers
#define configUSE_QUEUE_SETS                   0//Combine multiple blocking objects into a set, allowing tasks to wait for any event from them through a single interface
#define configUSE_TASK_NOTIFICATIONS            1//Task notification and wake-up mechanism

/* Hook Functions — Disabled for Minimal Version */
#define configUSE_IDLE_HOOK                     0//Background maintenance for low-power applications
#define configUSE_TICK_HOOK                     0//Hook function called on every tick interrupt
#define configUSE_MALLOC_FAILED_HOOK            0//Called when dynamic memory allocation fails
#define configCHECK_FOR_STACK_OVERFLOW          0//Called when task stack overflow is detected

/* Interrupt Priority (Cortex-M4, smaller value means higher priority) */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15//Priority range
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    1//Interrupts above this priority cannot use RTOS APIs to prevent corruption
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << 4)//Shifted to the upper four bits
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << 4)

/* Dynamic Task APIs */
#define INCLUDE_vTaskDelay                      1//Timed task delay
#define INCLUDE_xTaskCreate                     1//Task creation
#define INCLUDE_vTaskDelete                     0//Task deletion
#define INCLUDE_vTaskSuspend                    0//Task suspension
#define INCLUDE_xTaskGetSchedulerState          0//Scheduler state query

/* Port-related Settings — Do not define configSYSTICK_CLOCK_HZ, allowing the port to automatically use the processor clock */
#endif /* FREERTOS_CONFIG_H */
