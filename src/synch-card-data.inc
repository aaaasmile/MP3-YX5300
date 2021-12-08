// Songs are in folders: F01 till  F06. F00 is not used
int g_currFolder = 0x1; // F01
int g_lastFolder = 0x6; // F06
// each element is the number of files on each folder. Start with F00 that is not used.
int8_t g_maxSongs[] = {0, /*F01*/90, /*F02*/23, /*F03*/69,/*F04*/15,/*F05*/36,/*F06*/15}; // CHANGE these values if you change the content of the SD-Card
int8_t g_rndSongs[] = {0, /*F01*/1, /*F02*/1, /*F03*/1,/*F04*/1,/*F05*/0,/*F06*/0};