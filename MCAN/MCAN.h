#ifndef MCAN_h
#define MCAN_h

/*
 * Adressbereiche:
*/
#define MM_ACC 		0x3000	//Magnetartikel Motorola
#define DCC_ACC 	0x3800	//Magbetartikel NRMA_DCC
#define MM_TRACK 	0x0000	//Gleissignal Motorola
#define DCC_TRACK 	0xC000	//Gleissignal NRMA_DCC

/*
 * CAN-Befehle (Märklin)
*/
#define SYS_CMD		0x00 	//Systembefehle
 	#define SYS_STOP 	0x00 	//System - Stopp
 	#define SYS_GO		0x01	//System - Go
 	#define SYS_HALT	0x02	//System - Halt
#define SWITCH_ACC 	0x0b	//Magnetartikel Schalten
#define S88_EVENT	0x11	//Rückmelde-Event
#define PING 		0x18	//CAN-Teilnehmer anpingen


typedef struct {
	uint8_t cmd;
	uint16_t hash;
	bool resp_bit;
	uint8_t dlc;
	uint8_t data[8];
	bool is_set;
} MCANMSG;

class MCAN{
public:
	uint16_t generateHash(uint32_t uid);
	uint16_t initMCAN(uint32_t uid, bool debug);
	void sendCanFrame(MCANMSG can_frame);
	MCANMSG getCanFrame();
	void printCanFrame(MCANMSG can_frame);
private:
};

#endif