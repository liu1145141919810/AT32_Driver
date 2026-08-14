import json
import os
class Translate:
    def __init__(self):
        _base = os.path.dirname(os.path.abspath(__file__))
        _can_dir = os.path.join(_base, "..", "..", "..", "CAN_midware")
        with open(os.path.join(_can_dir, "can_protocol.json")) as f:
            readfile=json.load(f)
            self.readfile_command=readfile["command"]
            self.readfile_report=readfile["report"]
        self.writefileH1=os.path.join(_can_dir, "can_protocol.h")
        self.writefileC1=os.path.join(_can_dir, "can_protocol.c")
        self.writefileH2=os.path.join(_can_dir,"typeBasement.h")
    def translateH1(self):
        with open(self.writefileH1, "w") as f:
            f.write("#ifndef CAN_PROTOCOL_H\n")
            f.write("//This file is auto-generated, and could only be used by canUtility.c\n")
            f.write("\n")
            f.write('#include "Msg_Protocol.h"\n')
            f.write('#include "Command_analyzer.h"\n')
            f.write("\n")
            f.write("#define CAN_PROTOCOL_H\n")
            f.write("\n")
            f.write(f"typedef struct {{\n")
            f.write(f"    CommandType command;\n")
            f.write(f"    uint16_t id;\n")
            f.write(f"}} Transition_T;\n")
            f.write(f"typedef struct {{\n")
            f.write(f"    uint16_t id;\n")
            f.write(f"    Event event;\n")
            f.write(f"}} Transition_R;\n")
            f.write("Event can_id_to_event(uint16_t id);\n")
            f.write("uint16_t can_command_to_id(CommandType command,uint8_t substate);\n")
            # Other definitions
            f.write("#endif // CAN_PROTOCOL_H\n")
    def translateC1(self):
        with open(self.writefileC1, "w") as f:
            f.write('#include "can_protocol.h"\n')
            f.write('#include "typeBasement.h"\n')
            f.write("\n")

            f.write("static Transition_R can_receive_table[]={\n") 
            idx=0;
            for name,value in self.readfile_command.items(): 
                f.write("   {CMD_"+name.upper()+", "+str(idx)+"},\n")
                idx+=1        
            f.write("};\n")

            f.write("static Transition_T can_send_table[]={\n")
            idx=0;
            for name,value in self.readfile_report.items():
                if not isinstance(value,dict):
                    f.write("   { "+str(idx)+" ,RPT_"+value.upper()+"},\n")
                    idx+=1
            f.write("};\n")

            f.write("Event can_id_to_event(uint16_t id){\n")
            f.write("   for(int i=0;i<sizeof(can_receive_table)/sizeof(Transition_R);i++){\n")
            f.write("       if(can_receive_table[i].id==id){\n")
            f.write("           return can_receive_table[i].event;\n")
            f.write("       }\n")
            f.write("   }\n")
            f.write("   return ERROR_DEMO;\n")
            f.write("}\n")
            f.write("uint16_t can_command_to_id(CommandType command,uint8_t substate){\n")
            f.write("   for(int i=0;i<sizeof(can_send_table)/sizeof(Transition_T);i++){\n")
            f.write("       if(can_send_table[i].command==command){\n")
            f.write("           if (command==MONITOR&&substate!=0){\n")
            f.write("               if (substate==1)\n")
            f.write("                   return RPT_MONITOR_INTERNAL_TEMPERATURE;\n")
            f.write("               else if (substate==2)\n")
            f.write("                   return RPT_MONITOR_INTERNAL_VREF;\n")
            f.write("           }\n")
            f.write("           return can_send_table[i].id;\n")
            f.write("       }\n")
            f.write("   }\n")
            f.write("   return 0xFFFF;\n")
            f.write("}\n")
    def translateH2(self):
        with open(self.writefileH2, "w") as f:
            f.write("#ifndef TYPE_BASEMENT_H\n")
            f.write("#define TYPE_BASEMENT_H\n\n")
            f.write("//========== Attention: You need still revise the logic in FSM.c when change the transition mode ==========\n")
             # transmit protocol
            for name,value in self.readfile_command.items():
                macro_name="CMD_"+name.upper()
                f.write(f"#define {macro_name} {value}\n")
            # receive protocol
            f.write("\n")
            for name,value in self.readfile_report.items():
                if isinstance(value,dict):
                    macro_name="RPT_"+value["state"].upper()+'_'+value["substate"].replace(" ", "_").upper()
                else:
                    macro_name="RPT_"+value.upper()
                f.write(f"#define {macro_name} {int(name)}\n")

            f.write("\n")
            f.write("typedef enum {\n")
            for name in self.readfile_command:
                f.write(f"    {name.upper()},\n")
            f.write("}CommandType;\n")
            f.write("typedef enum {\n")
            for name in self.readfile_report:
                if not isinstance(self.readfile_report[name], dict):
                    f.write(f"    {self.readfile_report[name].upper()},\n")
            f.write("}Event;\n")
            f.write("#endif // TYPE_BASEMENT_H\n")
if __name__=="__main__":
    translator=Translate()
    translator.translateH1()
    translator.translateC1()
    translator.translateH2()