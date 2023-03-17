// Songs are in folders: F01 till  F06. F00 is not used
int g_currFolder = 0x1;  // F01
int g_lastFolder = 0x7;  // F07
// each element is the number of files on each folder. Start with F00 that is
// not used. CHANGE these values if you change the content of the SD-Card
uint16_t g_maxSongs[] = {0,          /*F01*/ 145, /*F02*/ 24, /*F03*/ 69,
                         /*F04*/ 15, /*F05*/ 36,  /*F06*/ 15, /*F07*/ 8};
// Random sequence folder: 0 no random, 1 random shuffle before playing the
// folder
uint8_t g_rndSongs[] = {0,         /*F01*/ 1, /*F02*/ 1, /*F03*/ 1,
                        /*F04*/ 1, /*F05*/ 1, /*F06*/ 1, /*F07*/ 1};