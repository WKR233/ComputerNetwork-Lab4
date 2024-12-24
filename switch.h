#ifndef COMPNET_LAB4_SRC_SWITCH_H
#define COMPNET_LAB4_SRC_SWITCH_H

#include "types.h"
#include <vector>
#include <stdio.h>

struct Item
{
    mac_addr_t Mac;
    int Port;
    int Time;
};
class SwitchBase {
 public:
  SwitchBase() = default;
  ~SwitchBase() = default;

  virtual void InitSwitch(int numPorts) = 0;
  virtual int ProcessFrame(int inPort, char* framePtr) = 0;
};

extern SwitchBase* CreateSwitchObject();
extern int PackFrame(char* unpacked_frame, char* packed_frame, int frame_length);
extern int UnpackFrame(char* unpacked_frame, char* packed_frame, int frame_length);

class MySwitch: public SwitchBase{
    public:
        virtual void InitSwitch(int numPorts);
        virtual int ProcessFrame(int inPort, char* framePtr);
        std::vector<Item> Forwarding;
        bool Find(uint8_t* target, std::vector<Item>::iterator* it);
};

void MySwitch::InitSwitch(int numPorts)
{
    Forwarding.clear();
}

bool MySwitch::Find(uint8_t* target, std::vector<Item>::iterator* it)
{
    for(std::vector<Item>::iterator tmp=Forwarding.begin();tmp!=Forwarding.end();tmp++)
    {
        int flag=1;
        for(int i=0;i<6;i++)
        {
            if(tmp->Mac[i]!=target[i])
            {
                flag=0;
                break;
            }
        }
        if(flag==1)
        {
            *it=tmp;
            return true; //found
        }
    }
    return false; // not found
}

int MySwitch::ProcessFrame(int inPort, char* framePtr)
{
    std::vector<Item>::iterator it;
    ether_header_t* header=(ether_header_t*)framePtr;
    uint8_t* dest_mac=(uint8_t*)header;
    uint8_t* src_mac=(uint8_t*)header+6;
    char* payload=framePtr+header->length;
    if(header->ether_type==ETHER_CONTROL_TYPE) // from controller
    {
        for(it=Forwarding.begin();it!=Forwarding.end();)
        {
            it->Time--;
            if(it->Time==ETHER_COMMAND_TYPE_AGING)
                it=Forwarding.erase(it);
            else
                it++;
        }
        return -1;
    }
    else // data frame
    {
        bool flag=Find(src_mac, &it);
        if(flag==false) // failed to find
        {
            bool flag=Find(dest_mac, &it);
            if(flag==false) // failed to find
            {
                Item tmp;
                tmp.Port=inPort;
                tmp.Time=ETHER_MAC_AGING_THRESHOLD;
                memcpy((void*)tmp.Mac,src_mac,6);
                Forwarding.push_back(tmp);
                return 0;
            }
            else // found
            {
                int outPort=it->Port;
                Item tmp;
                tmp.Port=inPort;
                tmp.Time=ETHER_MAC_AGING_THRESHOLD;
                memcpy((void*)tmp.Mac,src_mac,6);
                Forwarding.push_back(tmp);
                if(inPort==outPort)
                    return -1;
                else
                    return outPort;
            }
        }
        else
        {
            bool flag=Find(dest_mac, &it);
            if(flag==false) //failed to find
            {
                Find(src_mac, &it);
                it->Time=ETHER_MAC_AGING_THRESHOLD;
                return 0;
            }
            else // found
            {
                int outPort=it->Port;
                Find(src_mac, &it);
                it->Time=ETHER_MAC_AGING_THRESHOLD;
                if(inPort==outPort)
                    return -1;
                else
                    return outPort;
            }
        }
    }
}
#endif  // ! COMPNET_LAB4_SRC_SWITCH_H
