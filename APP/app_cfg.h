#ifndef __APP_CFG_H__
#define __APP_CFG_H__


/**************************设置任务优先级***********************************/
#define TASK_LIFT_PRIO              2
#define STARTUP_TASK_PRIO           3
#define TASK_AM2305_PRIO            4
#define TASK_STEP_PRIO              5
#define TASK_SR04_PRIO              6
#define TASK_COMM_PRIO				7
#define TASK_RTC_PRIO7              9
#define TASK_MSD_PRIO               10


#define TASK_PIP_PRIO               1 //优先级继承优先级  用于优先级反转处理

#define TASK_STKCHK_PRIO           OS_LOWEST_PRIO - 10

/***********************设置栈大小（单位为 OS_STK）***********************/
#define STARTUP_TASK_STK_SIZE       50
#define TASK_AM2305_STK_SIZE        100
#define TASK_SR04_STK_SIZE          60
#define TASK_COMM_STK_SIZE          100
#define TASK_RTC_STK_SIZE           100
#define TASK_MSD_STK_SIZE           160
#define TASK_STKCHK_STK_SIZE        120
#define TASK_LIFT_STK_SIZE          50
#define TASK_STEP_STK_SIZE          80



#endif //__APP_CFG_H__
