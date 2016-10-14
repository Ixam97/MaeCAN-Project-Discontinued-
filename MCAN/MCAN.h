/*
 *	Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *	Do with this whatever you want, but keep thes Header and tell
 *	the others what you changed1
 *
 *	Last changed: 2016-10-14
 */


#ifndef MCAN_h
#define MCAN_h

/*
 *  Gerätetypen:
 */
#define MCAN_MAGNET	0x0050
#define MCAN_SERVO 	0x0051


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
 	#define SYS_STAT	0x0b	//System - Status (sendet geänderte Konfiguration)
#define SWITCH_ACC 	0x0b	//Magnetartikel Schalten
#define S88_EVENT	0x11	//Rückmelde-Event
#define PING 		0x18	//CAN-Teilnehmer anpingen
#define CONFIG		0x1d	//Konfiguration

typedef struct {
	uint8_t cmd;
	uint16_t hash;
	bool resp_bit;
	uint8_t dlc;
	uint8_t data[8];
	bool is_set;
} MCANMSG;

typedef struct {
	uint8_t versHigh;		//Versionsnummer vor dem Punkt
	uint8_t versLow;		//Versionsnummer nach dem Punkt
	String name;			//Name des Geräts
	String artNum;			//Artikelnummer des Geräts
	int boardNum;			//Nummer des Geräts
	uint16_t hash;			//Hash des Geräts (muss vor her mit generateHash() berechnet werden)
	uint32_t uid;			//UID des Geräts
	uint32_t type;			//Typ des Geräts (z.B. MäCAN Magnetartikeldecoder: 0x0050)
} CanDevice;

class MCAN{
public:
	uint16_t generateHash(uint32_t uid);						//Hash aus der UID berechnen
	uint16_t generateLocId(uint16_t prot, uint16_t adrs);		//Local ID aus Adresse und Protokoll berechnen

	void initMCAN();											//CAN-Bus initialisieren
	void initMCAN(bool debug);									//Optional mit Konsolenausgabe

	void sendCanFrame(MCANMSG can_frame);						//Einen CAN-Frame abschicken

	void sendDeviceInfo(CanDevice device, int configNum);		//Gerätebeschreibung verschicken
	void sendConfigInfoDropdown(CanDevice device, uint8_t configChanel, uint8_t numberOfOptions, uint8_t defaultSetting, String settings);
	void sendConfigInfoSlider(CanDevice device, uint8_t configChanel, uint32_t lowerValue, uint32_t upperValue, uint32_t defaultValue, String settings);

	void sendPingResponse(CanDevice device);					//Ping-Antwort verschicken

	MCANMSG getCanFrame();										//Einen CAN-Frame auslesen
private:
};

#endif