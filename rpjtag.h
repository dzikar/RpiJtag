//Header file for rpjtag.c
//Credit for GPIO setup goes to http://elinux.org/RPi_Low-level_peripherals

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define XILINX_BITFILE 1

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

#define MAX_CHAIN 50
//device position in JTAG chain. Number from 0
#define DEVICE_NUM 0
#define IRLEN 6

int  mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET(g) *(gpio+7) = 1<<(g) // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR(g) *(gpio+10) = 1<<(g) // clears bits which are 1 ignores bits which are 0
#define GPIO_READ(g) (*(gpio+13) >> (g)) & 0x00000001
#define GPIO_READRAW *(gpio+13)

//Perspective is from Device connected, so TDO is output from device to input into rpi
#define JTAG_TMS 9 //MISO 	PI ---> JTAG
#define JTAG_TDI 7 //CE1 	PI ---> JTAG
#define JTAG_TDO 8 //CE0 	PI <--- JTAG
#define JTAG_TCK 11 //#4 	PI ---> JTAG

//-D DEBUG when compiling, will make all sleeps last 0.5 second, this can be used to test with LED on ports, or pushbuttons
//Else sleeps can be reduced to increase speed
#ifdef DEBUG
#define WAIT 10000000 //aprox 0.5s
#else
#define WAIT 10000 //aprox 0.5us
#endif

int n, x, i, temp, IRTotalRegSize, jtag_state, nDevices, parms, bitFileId, fileCounter,deviceDataNr, deviceChainNr;

struct IDCODE_DATA
{
	int onebit:1;
	int manuf:11;
	int size:9;
	int family:7;
	int rev:1;
} idcode[32]; //32 Devices

/*
Devices known:
 IDCODE  Manuf     Device name
01414093 Xilinx    XC3S200
0141C093 Xilinx    XC3S400
F5044093 Xilinx    XCF01S
F5045093 Xilinx    XCF02S
*/

struct Device
{
	char* DeviceName; //Device name
	int idcode:32; //Device Id Code
	int dIRLen; //Device IR Length
	int dBSRLen; //Device Boundary Scan Length, currently on v0.3 not used
	FILE* filePtr; //Pointer to our BDSL, in case we need it
} device_data[32]; //32 Devices

struct bitFileInfo
{
	int BitFile_Type;
	char* DesignName;
	char* DeviceName;
	int Bitstream_Length;
} bitfileinfo;
//struct bitFileInfo bitfileinfo;

void ProgramDevice(int deviceNr, FILE *f);
void send_byte(unsigned char byte, int lastbyte);
void send_byte_no_tms(unsigned char byte);
void send_last_byte(unsigned char byte);
void read_bdsl_file(char *filename, int fileNr);
void load_bdsl_files(char *bdslfiles);
FILE* load_bit_file(char *ifile);
void CreateDeviceTable();
int GetSegmentLength(int segment, int segmentCheck, FILE *f);
void parse_header();
void UpdateState(int j_state, int iTMS);
void nop_sleep(long x);
int read_jtag_tdo();
void send_cmd(int iTDI,int iTMS);
void send_cmdWord_msb_first(unsigned int cmd, int lastBit, int bitoffset);
void send_cmdWord_msb_last(unsigned int cmd, int lastBit, int bitoffset);
void syncJTAGs();
void SelectShiftDR();
void SelectShiftIR();
void ExitShift();
int CountDevices();
void readIDCODES();
void jtag_read_data(char* data,int iSize);
void checkUserId(int x);
void help();
void setup_io();
