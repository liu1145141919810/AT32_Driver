#ifndef TYPE_BASEMENT_H
#define TYPE_BASEMENT_H

//========== Attention: You need still revise the logic in FSM.c when change the transition mode ==========
#define CMD_GETIN_ORDER 256
#define CMD_RETURN_DEFAULT 257
#define CMD_ACT_LIGHT 258
#define CMD_ACT_MONITOR 259
#define CMD_ACT_BRIGHT 260
#define CMD_SHIFT_CALIBRATE 261
#define CMD_OFF 262
#define CMD_ERROR_DEMO 263

#define RPT_DEFAULT 768
#define RPT_ORDER 769
#define RPT_LIGHT 770
#define RPT_MONITOR 771
#define RPT_MONITOR_INTERNAL_TEMPERATURE 772
#define RPT_MONITOR_INTERNAL_VREF 773
#define RPT_BRIGHT 774
#define RPT_CALIBRATE 775
#define RPT_NOADDING 776
#define RPT_ERROR_EVENT 777
#define RPT_ERROR_STATE 778

typedef enum {
    GETIN_ORDER,
    RETURN_DEFAULT,
    ACT_LIGHT,
    ACT_MONITOR,
    ACT_BRIGHT,
    SHIFT_CALIBRATE,
    OFF,
    ERROR_DEMO,
}CommandType;
typedef enum {
    DEFAULT,
    ORDER,
    LIGHT,
    MONITOR,
    BRIGHT,
    CALIBRATE,
    NOADDING,
    ERROR_EVENT,
    ERROR_STATE,
}Event;
#endif // TYPE_BASEMENT_H
