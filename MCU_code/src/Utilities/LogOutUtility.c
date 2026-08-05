#include "LogOutUtility.h"
#include "at32f423_usart.h"

//================== Following is the conversion for payload ===========
static void logout_queue(char* buffer,uint8_t* idx,char c){
    buffer[(*idx)++] = c;
}
static void printNumI(char* buffer,uint8_t* idx,int num){
    char local_buffer[16];
    int p=0;
    if(num<0)
    {
        //buffer[(*idx)++] = '-';
        logout_queue(buffer,idx,'-');
        num=-num;
    }
    if(num==0)
    {
        //buffer[(*idx)++] = '0';
        logout_queue(buffer,idx,'0');
    }
    else
    {
        while(num>0){
            local_buffer[p++]=num%10+'0';
            num/=10;
            if(p>=16) break;
        }
        for(int i=p-1;i>=0;i--)
        {logout_queue(buffer,idx,local_buffer[i]);}
    }
}
//Accuracy can not be higher than 9 decimal places, otherwise it will be rounded to 9 decimal places
static void printNumF(char *buffer, uint8_t *idx, double num, int decimalPlaces)
{
    char local_buffer[16];
    int p=0;
    if(num<0)
    {
        logout_queue(buffer,idx,'-');
        num=-num;
    }
    if(decimalPlaces>0){
        double round_base=1;
        for(int i=0;i<decimalPlaces;i++) round_base *=10;
        num=num+0.5/round_base;
    }
    int integer=(int)num;
    double fraction=num-integer;//This part must have a pre-dealing for rounding
    // 处理整数
    if(integer==0)
    {
        logout_queue(buffer,idx,'0');
    }
    else
    {
        while(integer>0){
            local_buffer[p++]=integer%10+'0';
            integer/=10;
            if(p>=16) break;
        }
        for(int i=p-1;i>=0;i--)
        {logout_queue(buffer,idx,local_buffer[i]);}
    }
    //处理小数
    if(decimalPlaces>0)
    {
        logout_queue(buffer,idx,'.');
        while(decimalPlaces--)
        {
            fraction*=10;
            int digit=(int)fraction;
            logout_queue(buffer,idx,digit+'0');
            fraction-=digit;
        }
    }
}
uint8_t demoPrint(char* buffer,uint8_t* idx,const char* str,va_list args){
    int i=0;
    while(*str){
        if(*str=='%'){
            str++;
            if(*str=='d'){//Integer Dealing
                int val=va_arg(args, int);
                printNumI(buffer,idx,val);
            }
            else if(*str == 's'){//Char String Dealing
                char* s = va_arg(args, char*);
                while(*s){
                    logout_queue(buffer,idx,*s);
                    s++;
                }
            }
            else if(*str == '.'){
                str+=1;
                double val = va_arg(args, double);
                int store=*str - '0';//Get the number of decimal places to display, no more than 9
                str+=1;
                printNumF(buffer, idx, val, store);
            }
        }
        else{
            logout_queue(buffer,idx,*str);
        }
        str++;i++;
    }
    return 0;
}