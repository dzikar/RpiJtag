int read_jtag_tdo()
{
    return ( GPIO_READ(JTAG_TDO) ) ? 1 : 0;
}

int tdi=-1;

void send_cmd_no_tms(int iTDI)
{
    if(iTDI == 0)
    {
      if (tdi != 0)
      {
          GPIO_CLR(JTAG_TDI);
          tdi = 0;
      }
    }
    else
    {
        if (tdi != 1)
        {
            GPIO_SET(JTAG_TDI);
            tdi = 1;
        }
    }

    //nop_sleep(WAIT);
    GPIO_SET(JTAG_TCK);
    //nop_sleep(WAIT);
    GPIO_CLR(JTAG_TCK);
    //nop_sleep(WAIT);
}

void send_cmd(int iTDI,int iTMS)
{
    
    if(iTDI == 1)
    {
        GPIO_SET(JTAG_TDI);
        tdi = 1;
    }
    else
    {
        GPIO_CLR(JTAG_TDI);
        tdi = 0;
    }

    if(iTMS == 1)
    {
        GPIO_SET(JTAG_TMS);
    }
    else
        GPIO_CLR(JTAG_TMS);

    //nop_sleep(WAIT);
    GPIO_SET(JTAG_TCK);
    //nop_sleep(WAIT);
    GPIO_CLR(JTAG_TCK);
    //nop_sleep(WAIT);
}

//Mainly used for command words (CFG_IN)
void send_cmdWord_msb_first(unsigned int cmd, int lastBit, int bitoffset) //Send data, example 0xFFFF,0,20 would send 20 1's, with not TMS
{
    while(bitoffset--)
    {
        int x = ( cmd >> bitoffset) & 0x01;
        send_cmd(x,(lastBit==1 && bitoffset==0));
    }
}

//Mainly used for IR Register codes
void send_cmdWord_msb_last(unsigned int cmd, int lastBit, int bitoffset) //Send data, example 0xFFFF,0,20 would send 20 1's, with not TMS
{
    int i;
    for(i=0;i<bitoffset;i++)
    {
        int x = ( cmd >> i ) & 0x01;
        send_cmd(x,(lastBit==1 && bitoffset==i+1));
    }
}

void send_byte(unsigned char byte, int lastbyte) //Send single byte, example from fgetc
{
    int x;
        for(x=7;x>=0;x--)
        {
            send_cmd(byte>>x&0x01,( x==0) && (lastbyte==1));
        }
}

void send_byte_no_tms(unsigned char byte)
{
    //int x;
    //    for(x=7;x>=0;x--)
    //    {
    //        send_cmd_no_tms(byte>>x&0x01);
    //    }
    send_cmd_no_tms(byte&0x80);
    send_cmd_no_tms(byte&0x40);
    send_cmd_no_tms(byte&0x20);
    send_cmd_no_tms(byte&0x10);
    send_cmd_no_tms(byte&0x08);
    send_cmd_no_tms(byte&0x04);
    send_cmd_no_tms(byte&0x02);
    send_cmd_no_tms(byte&0x01);
}

//Does a NOP call in BCM2708, and is meant to be run @ 750 MHz
void nop_sleep(long x)
{
    while (x--) {
        asm("nop");
    }
}

void jtag_read_data(char* data,int iSize)
{
    if(iSize==0) return;
    int bitOffset = 0;
    memset(data,0,(iSize+7)/8);

    iSize--;
    while(iSize--)
    {
        temp = read_jtag_tdo();
        send_cmd(0,0);
        data[bitOffset/8] |= (temp << (bitOffset & 7));
        bitOffset++;
    }

    temp = read_jtag_tdo(); //Read last bit, while also going to EXIT
    send_cmd(0,1);
    data[bitOffset/8] |= (temp << (bitOffset & 7));
    send_cmd(0,1); //Go to UPDATE STATE
    send_cmd(0,1); //Go to SELECT DR-SCAN
}
