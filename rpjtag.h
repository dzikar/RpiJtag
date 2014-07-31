//Header file for rpjtag.c

#define XILINX_BITFILE 1


#define MAX_CHAIN 50
//device position in JTAG chain. Number from 0
#define DEVICE_NUM 0
#define IRLEN 6




int n, x, i, temp, IRTotalRegSize, jtag_state, nDevices, parms, bitFileId, fileCounter,deviceDataNr, deviceChainNr;

//struct IDCODE_DATA
//{
//	int onebit:1;
//	int manuf:11;
//	int size:9;
//	int family:7;
//	int rev:1;
//} idcode[32]; //32 Devices

/*
Devices known:
 IDCODE  Manuf     Device name
01414093 Xilinx    XC3S200
0141C093 Xilinx    XC3S400
F5044093 Xilinx    XCF01S
F5045093 Xilinx    XCF02S
*/


struct bitFileInfo
{
	int BitFile_Type;
	char* DesignName;
	char* DeviceName;
	int Bitstream_Length;
} bitfileinfo;
//struct bitFileInfo bitfileinfo;

//void ProgramDevice(int deviceNr, FILE *f);
//void send_byte(unsigned char byte, int lastbyte);
//void send_byte_no_tms(unsigned char byte);
//void send_last_byte(unsigned char byte);
//void read_bdsl_file(char *filename, int fileNr);
//void load_bdsl_files(char *bdslfiles);
//FILE* load_bit_file(char *ifile);
void CreateDeviceTable();
//void nop_sleep(long x);
//int read_jtag_tdo();
//void send_cmd(int iTDI,int iTMS);
//void send_cmdWord_msb_first(unsigned int cmd, int lastBit, int bitoffset);
//void send_cmdWord_msb_last(unsigned int cmd, int lastBit, int bitoffset);
//void syncJTAGs();
//void SelectShiftDR();
//void SelectShiftIR();
//void ExitShift();
//int CountDevices();
void readIDCODES();
//void jtag_read_data(char* data,int iSize);
void checkUserId(int x);
void help();
//void setup_io();
