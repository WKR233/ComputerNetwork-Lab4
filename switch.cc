#include "switch.h"

SwitchBase* CreateSwitchObject()
{
    return new MySwitch();
}

int PackFrame(char* unpacked_frame, char* packed_frame, int frame_length)
{
    *packed_frame=FRAME_DELI;
    memcpy(packed_frame+1, unpacked_frame, frame_length);
    char* end=packed_frame+1+frame_length;
    for(char* tmp=packed_frame+1;tmp!=end;tmp++) // process the FRAME_DELI
    {
        if(*(unsigned char*)tmp==FRAME_DELI)
        {
            memmove(tmp+1,tmp,end-tmp);
            tmp++;
            end++;
        }
    }
    int count=0;
    for(char* tmp=packed_frame;tmp!=end;tmp++)
    {
        uint8_t n=*(unsigned char*)tmp;
        while(n)
        {
            count++;
            n&=(n-1);
        }
    }
    if(count%2==0)
        *end=(unsigned char)0x00;
    else
        *end=(unsigned char)0x01;
    end++;
    return end-packed_frame;
}

int UnpackFrame(char* unpacked_frame, char* packed_frame, int frame_length)
{
    memcpy(unpacked_frame, packed_frame, frame_length);
    char* end=unpacked_frame+frame_length;
    int count=0;
    for(char* tmp=unpacked_frame;tmp!=end;tmp++)
    {
        uint8_t n=*(unsigned char*)tmp;
        while(n)
        {
            count++;
            n&=(n-1);
        }
    }
    if(count%2!=0)
        return -1;
    memmove(unpacked_frame, unpacked_frame+1, end-(unpacked_frame+1));
    end--;
    end--;
    for(char* tmp=unpacked_frame+1;tmp!=end;tmp++)
    {
        if(*(unsigned char*)tmp==FRAME_DELI&&*(unsigned char*)(tmp-1)==FRAME_DELI)
        {
            memmove(tmp-1,tmp,end-tmp);
            tmp--;
            end--;
        }
    }
    return end-unpacked_frame;
}