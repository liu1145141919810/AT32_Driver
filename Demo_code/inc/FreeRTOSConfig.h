#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* AT32F423 最小 FreeRTOS 配置 */
/* 调度器 */
#define configUSE_PREEMPTION                    1
#define configCPU_CLOCK_HZ                      (144000000UL)//与模型一致的设计
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    (5)
#define configMINIMAL_STACK_SIZE                ((uint16_t)128)
#define configMAX_TASK_NAME_LEN                 (16)
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

/* 内存管理 */
#define configTOTAL_HEAP_SIZE                   ((size_t)(10240))

/* 可选功能 —— 最小版本全部关闭 */
#define configUSE_MUTEXES                       0
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_TIMERS                        0
#define configUSE_QUEUE_SETS                    0
#define configUSE_TASK_NOTIFICATIONS            0

/* Hook 函数 —— 最小版本全部关闭 */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configCHECK_FOR_STACK_OVERFLOW          0

/* 中断优先级（Cortex-M4，数值越小优先级越高） */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << 4)
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << 4)

/* 动态任务 API */
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskCreate                     1
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_vTaskSuspend                    0
#define INCLUDE_xTaskGetSchedulerState          0

/* 端口相关 —— 不定义 configSYSTICK_CLOCK_HZ，让 port 自动使用处理器时钟 */

#endif /* FREERTOS_CONFIG_H */
