/*
 * Raspberry Pi JTAG Programmer using GPIO connector
 * Version 0.3 Copyright 2013 Rune 'Krelian' Joergensen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "rpjtag_bit_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "rpjtag.h" //Defines
#include "rpjtag_stateMachine.h"
#include "rpjtag_io.h"

FILE* load_bit_file(char *ifile)
{
	FILE *temp;
	temp = fopen(ifile, "r");
	if(!temp)
	{
		fprintf(stderr, "RpiJtag: could not open bitstream file %s for read, aborting.\n", ifile); 
		exit(1);
	}else{
		fprintf(stderr, "RpiJtag: bitstream file %s has been read, continuing.\n", ifile); 
	}
	parse_header(temp);
	return temp;
}

void checkStatusReg()
{
	//Xilinx CFG_IN Command words, should be loaded via bitFileId
	unsigned int DYMMYWORD = 0xFFFFFFFF;
	unsigned int NOOP = 0x20000000;
	unsigned int SYNCWORD = 0xAA995566;
	unsigned int STATUS_REG = 0x2800E002;
    int v, n;
	
	syncJTAGs();
	SelectShiftIR();

	//CFG_IN
	send_cmdWord_msb_last(0x3FC5,1,14); //11111111 000101

	send_cmd(0,1); // -> UPDATE-IR
	send_cmd(0,1); // -> SELECT-DR SCAN
	send_cmd(0,0);
	send_cmd(0,0);

	for(n=0;n<31;n++) send_cmd(0,0); //31 Leading Zero's

	send_cmdWord_msb_first(DYMMYWORD,0,32);
	send_cmdWord_msb_first(SYNCWORD,0,32);
	send_cmdWord_msb_first(NOOP,0,32);
	send_cmdWord_msb_first(STATUS_REG,0,32);
	send_cmdWord_msb_first(NOOP,0,32);
	send_cmdWord_msb_first(NOOP,1,32);

	for(n=0;n<3;n++) send_cmd(0,1); //Shift-Ir
	send_cmd(0,0);
	send_cmd(0,0);

	//CFG_OUT
	send_cmdWord_msb_last(0x3FC4,1,14); //11111111 000100

	send_cmd(0,1);
	send_cmd(0,1); //SELECT-DR
	send_cmd(0,0);
	send_cmd(0,0); //SHIFT-DR

	fprintf(stderr,"\nSTAT: ");
	for(n=0;n<32;n++)
	{
            v = read_jtag_tdo();
            send_cmd(0,0);
			fprintf(stderr,"%d",v);
	}
	fprintf(stderr,"\n");

}

//remember last device is always the one to send IR bits to first
void ProgramDevice(int deviceNr, FILE *f)
{
	syncJTAGs();
	SelectShiftIR();

	//CFG_IN
	/*
	send_cmdWord_msb_last(0x3FC5,1,14); //11111111 000101
	OR
	send_cmdWord_msb_last(0x05,0,6); //000101
	send_cmdWord_msb_last(0xFF,1,8); //11111111
	*/

    send_cmdWord_msb_last(0x05,1,IRLEN);

    // The send_cmdWord routine tags the last bit with TMS, so we are in state Exit1IR
	send_cmd(0,1); // -> UPDATE IR
	send_cmd(0,1); // -> SELECT SCAN-DR
	send_cmd(0,0); // -> CAPTURE DR
	send_cmd(0,0); // -> SHIFT DR


	int n, x;
    int n_bytes = bitfileinfo.Bitstream_Length; // this is in bytes
    unsigned char *buffer;
    buffer = (unsigned char *)malloc(n_bytes);
    fread(buffer, n_bytes, 1, f);
	fprintf(stderr, "\nProgramming device %d/%d, bitstream length (%d)\n",(nDevices-deviceNr),nDevices,n_bytes);

    // The first word is a dummy...
	for(n = 0; n < 4; n++)
    {
        if (buffer[n] != 0xFF)
		{
			fprintf(stderr, "\nError in bitfile, no Dummy Word at start");
			exit(1);
		}
    }

    // Now send all the data except the last byte. Separate this, because
    // the intermediate bytes don't need a tms strobe at their end
	for(n = 4; n < n_bytes-1; n++) //send all but the last byte
	{
	    send_byte_no_tms(buffer[n]);
		if((n % 1048576) == 0)
        {
            fprintf(stdout,"%.2f complete\n",(100.*n)/n_bytes);
        }
	}

    // send last byte with a tms strobe on the final bit
    send_byte(buffer[n++],1);

	//Last bit has TMS 1, so we are in EXIT-DR
	fprintf(stderr, "\nProgrammed %d bytes", n);
	send_cmd(0,1); // -> UPDATE-DR
	send_cmd(0,1); // -> SELECT DR SCAN
	send_cmd(0,1); // -> SELECT IR SCAN
	send_cmd(0,0);
	send_cmd(0,0);

	//JSTART
	//send_cmdWord_msb_last(0x3FCC,1,14); //11111111 001100
	for(x=nDevices;x>0;x--)
	{
		if((nDevices-deviceNr)==x)
		{
			if((x-1)==0) //To check is MSB in chain is sent with this cmd
				send_cmdWord_msb_last(0x0C,1,device_data[deviceNr].dIRLen);
			else
				send_cmdWord_msb_last(0x0C,0,device_data[deviceNr].dIRLen);
		}else{	
			if((x-1)==0) //To check is MSB in chain is sent with this cmd
				send_cmdWord_msb_last(0xFFFFF,1,device_data[x].dIRLen);//Bypass max, 20bit register
			else
				send_cmdWord_msb_last(0xFFFFF,0,device_data[x].dIRLen);//Bypass max, 20bit register
		}
	}

	//FPGA in JSTART MODE and PROM in BYPASS

	send_cmd(0,1); //->UPDATE IR
	for(n=0;n<16;n++) send_cmd(0,0); //Go to Run-Test, and idle for 12+ clocks to start

}

void checkUserId(int x)
{
	syncJTAGs();
	SelectShiftIR();

	send_cmdWord_msb_last(0x3FC8,1,14); //11111111 001000

	send_cmd(0,1);
	send_cmd(0,1);
	send_cmd(0,0);
	send_cmd(0,0);

	int data;
	jtag_read_data((char *)&data,32);
	if(!x)
		fprintf(stderr,"\nUserID Before programming: %08X",data);
	else
		fprintf(stderr,"\nUserID After programming: %08X",data);
	send_cmd(0,1);
	send_cmd(0,0);
	send_cmd(0,0); //Run-Test/Idle
}

int GetSegmentLength(int segment, int segmentCheck, FILE *f)
{
	//Get Next Segment
	segment = fgetc(f);
	if(segment != segmentCheck)
	{
		fprintf(stderr, "Error in header segment: %d, should be %d\n",segment,segmentCheck);
		exit(1);
	}

	return ((fgetc(f) << 8) + fgetc(f)); //Lenght of segment
}

//Made from XC3S200_FT256 bit file
//Code is based on code from http://panteltje.com/panteltje/raspberri_pi/
//And David Sullins BitInfo/BitFile code
void parse_header(FILE *f)
{
	//00 09  0F F0  0F F0  0F F0  0F F0  00 00 01 
	unsigned int XilinxID13[] = {0x00,0x09, 0x0F,0xF0, 0x0F,0xF0, 0x0F,0xF0, 0x0F,0xF0, 0x00,0x00,0x01};
	unsigned int *HeaderBufferData;
	int segmentCheck = 0x61; //Start at a char(97)
	int segment = 0;
	int	segmentLength = 0;
	int offset = 0;
	unsigned char tempChar;

	//First 13 bits, so far all Xilinx Bitstream files, start with XilinxID13 bytes
	//Seems to be identifier for bitfile creator
	int t, x;
	HeaderBufferData = (unsigned int*)malloc( sizeof(unsigned int) *13);
	for(t=0;t<13;t++) HeaderBufferData[t] = fgetc(f);

	if(0==memcmp(HeaderBufferData,XilinxID13,sizeof(HeaderBufferData)*13)) 
		bitFileId = XILINX_BITFILE;
	else
		bitFileId = 0;
	//if(0==memcmp(HeaderBufferData,XilinxID13,sizeof(HeaderBufferData)*13)) bitFileId = Altera_BITFILE; else bitFileId = 0; //Future stuff
	free(HeaderBufferData);

	fprintf(stderr, "Bitfile type: %d\n",bitFileId);
	bitfileinfo.BitFile_Type = bitFileId;
//----------------------------------------------------------------------
	segmentLength = GetSegmentLength(segment,segmentCheck,f);
	segmentCheck++;
	fprintf(stderr, "Design Name: ");
	bitfileinfo.DesignName = (char*)malloc(sizeof(char)*segmentLength);
	for(x = 0; x < segmentLength; x++)
	{
		bitfileinfo.DesignName[x] = fgetc(f);
	}
	fprintf(stderr, "%s\n",bitfileinfo.DesignName);
//----------------------------------------------------------------------
	segmentLength = GetSegmentLength(segment,segmentCheck,f);
	segmentCheck++;
	fprintf(stderr, "Device: ");
	if(bitFileId==XILINX_BITFILE)
	{
			bitfileinfo.DeviceName = (char*)malloc(sizeof(char)*(segmentLength+2));
			bitfileinfo.DeviceName[0] = 'x'; //Xilinx XC not stored in bit file
			bitfileinfo.DeviceName[1] = 'c';
			offset=2;

	}else{
		bitfileinfo.DeviceName = (char*)malloc(sizeof(char)*segmentLength);
		offset=0;
	}
	for(x = 0; x < segmentLength; x++)
	{
		bitfileinfo.DeviceName[x+offset] = fgetc(f);
	}
	fprintf(stderr, "%s\n",bitfileinfo.DeviceName);
//----------------------------------------------------------------------
	segmentLength = GetSegmentLength(segment,segmentCheck,f);
	segmentCheck++;
	fprintf(stderr, "Date: ");
	for(x = 0; x < segmentLength; x++)
		{
		tempChar = fgetc(f);
		fprintf(stderr, "%c", tempChar);
		}
	fprintf(stderr, "\n");
//----------------------------------------------------------------------
	segmentLength = GetSegmentLength(segment,segmentCheck,f);
	segmentCheck++;
	fprintf(stderr, "Time: ");
	for(x = 0; x < segmentLength; x++)
		{
		tempChar = fgetc(f);
		fprintf(stderr, "%c", tempChar);
		}
	fprintf(stderr, "\n");
//----------------------------------------------------------------------
	segment = fgetc(f);
	if(segment != segmentCheck)	fprintf(stderr, "Error in header segment: %d, should be %d\n",segment,segmentCheck);
	segmentLength = (fgetc(f) << 24) + (fgetc(f) << 16) + (fgetc(f) << 8) + fgetc(f);
	fprintf(stderr, "Bitstream Length: %0d bits\n", segmentLength * 8);
	bitfileinfo.Bitstream_Length = segmentLength;

	//That's it for now, bitfile header info is stored in bitfileinfo, FILE *f, needs to be kept until we program
	//This leaves us at 0xFF FF FF FF (Dummy Word) and the Sync Cmd 0xAA 99 55 66 (XILINX)
}
