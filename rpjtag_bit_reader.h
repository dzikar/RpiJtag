#ifndef RPJTAG_BIT_READER_H
#define RPJTAG_BIT_READER_H

#include <stdio.h>
struct Device
{
	char* DeviceName; //Device name
	int idcode:32; //Device Id Code
	int dIRLen; //Device IR Length
	int dBSRLen; //Device Boundary Scan Length, currently on v0.3 not used
	FILE* filePtr; //Pointer to our BDSL, in case we need it
} device_data[32]; //32 Devices

FILE* load_bit_file(char *ifile);
void checkStatusReg();
void ProgramDevice(int deviceNr, FILE *f);
void checkUserId(int x);
int GetSegmentLength(int segment, int segmentCheck, FILE *f);
void parse_header(FILE *f);

#endif
