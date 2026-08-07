#include <string.h>
#include <stdlib.h>

#include "Command_analyzer.h"
typedef struct{
    const char* name;
    Event event;
}CommandMap;
CommandMap command_list[]={
    {"Order",GETIN_ORDER},
    {"Light",ACT_LIGHT},
    {"Monitor",ACT_MONITOR},
    {"Bright",ACT_BRIGHT},
    {"Calibrate",SHIFT_CALIBRATE},
    {"Off",OFF}
};
static char* getToken(char** str){
    //Must use double pointer to change the str(address change)
    char* start;
    while(**str==' ')(*str)++;
    if(**str=='\0')return NULL;
    start=*str;
    while(**str!=' '&&**str!='\0')(*str)++;
    if(**str!='\0'){
        **str='\0';
        (*str)++;
    }return start;
}
static Event stringToEvent(const char* str){
    for(int i=0;i<sizeof(command_list)/sizeof(CommandMap);i++){
        if(strcmp(str,command_list[i].name)==0){
            return command_list[i].event;
        }
    }return ERROR_DEMO;
}
Command parseCommand(char* cmd_buf){
    Command cmd;
    char* head=getToken(&cmd_buf);
    char* token=getToken(&cmd_buf);
    Event event=stringToEvent(head);
    cmd.event=event;
    cmd.param_count=0;
    while(token){
        cmd.params[cmd.param_count++]=atoi(token);
        token=getToken(&cmd_buf);
    }
    return cmd;
}