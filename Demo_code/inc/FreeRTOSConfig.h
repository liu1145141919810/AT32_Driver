#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
/* AT32F423 最小 FreeRTOS 配置 */
/* 调度器 */
#define configUSE_PREEMPTION                    1 
#define configCPU_CLOCK_HZ                      (144000000UL)//这里是CPU频率，与模型一致的设计
#define configTICK_RATE_HZ                      ((TickType_t)1000)//RTOS的节拍频率
#define configMAX_PRIORITIES                    (5)//任务优先级范围设置
#define configMINIMAL_STACK_SIZE                ((uint16_t)128)//单个任务的栈的最小大小
#define configMAX_TASK_NAME_LEN                 (16)//任务名称最大长度
#define configUSE_16_BIT_TICKS                  0//ticjk计数器使用32位还是16位
#define configIDLE_SHOULD_YIELD                 1//最低优先级的固定任务是否让出

/* 内存管理 */
#define configTOTAL_HEAP_SIZE                   ((size_t)(10240))//内核管理的动态内存池分配10k

/* 可选功能 —— 最小版本全部关闭 */
#define configUSE_MUTEXES                       0//是否使用互斥锁
#define configUSE_COUNTING_SEMAPHORES           0//一类资源有多个可用数量时，用一个计数值表示剩余资源数量。
#define configUSE_TIMERS                        0//软件定时器
#define configUSE_QUEUE_SETS                    0//把多个“可以阻塞等待的对象”组合成一个集合，让任务通过一个接口等待“其中任意一个对象发生事件”。
#define configUSE_TASK_NOTIFICATIONS            0//任务唤醒通知相关

/* Hook 函数 —— 最小版本全部关闭 */
#define configUSE_IDLE_HOOK                     0//低功耗后台维护
#define configUSE_TICK_HOOK                     0//每次Tick中断时调用的钩子函数
#define configUSE_MALLOC_FAILED_HOOK            0//动态内存使用失败
#define configCHECK_FOR_STACK_OVERFLOW          0//任务栈溢出时调用

/* 中断优先级（Cortex-M4，数值越小优先级越高） */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15//优先级范围
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    1//太高的优先级不能用，防止破坏
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << 4)//转到高四位
#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << 4)

/* 动态任务 API */
#define INCLUDE_vTaskDelay                      1//定时暂停
#define INCLUDE_xTaskCreate                     1//任务创建
#define INCLUDE_vTaskDelete                     0//任务删除
#define INCLUDE_vTaskSuspend                    0//永久暂停
#define INCLUDE_xTaskGetSchedulerState          0//调度器状态查看
/* 端口相关 —— 不定义 configSYSTICK_CLOCK_HZ，让 port 自动使用处理器时钟 */
#endif /* FREERTOS_CONFIG_H */
