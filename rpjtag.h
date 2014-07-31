# ifndef RPJTAG_H
# define RPJTAG_H

//Header file for rpjtag.c

#define XILINX_BITFILE 1


#define MAX_CHAIN 50
//device position in JTAG chain. Number from 0
#define DEVICE_NUM 0
#define IRLEN 6




int n, x, i, temp, IRTotalRegSize, jtag_state, nDevices, parms, bitFileId, fileCounter,deviceDataNr, deviceChainNr;

#endif
