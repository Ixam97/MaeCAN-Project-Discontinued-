/*
 *	Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *	Do with this whatever you want, but keep thes Header and tell
 *	the others what you changed!
 */


#include "Arduino.h"
#include "MCAN.h"
#include "mcp_can.h"

MCP_CAN can(10);

uint32_t rxId;

uint16_t MCAN::generateHash(uint32_t uid){
	uint16_t highbyte = uid >> 8;
	uint16_t lowbyte = uid;
	uint16_t hash = highbyte ^ lowbyte;
	bitWrite(hash, 7, 0);
	bitWrite(hash, 8, 1);
	bitWrite(hash, 9, 1);
	return hash;
}

uint16_t MCAN::initMCAN(uint32_t uid, bool debug){
	uint16_t hash = generateHash(uid);
	pinMode(9,OUTPUT);
	digitalWrite(9,0);
	if (debug)
	{
		Serial.begin(250000);
		Serial.println("MCAN started ...");

		if (can.begin(CAN_250KBPS) == CAN_OK)
		{
			Serial.println("CAN-Initialisation successfull!");
		} else {
			Serial.println("CAN-Initialisation failed!");
		}
	}else if (can.begin(CAN_250KBPS) == CAN_OK){
		digitalWrite(9,1);
	}

	return hash;
}

void MCAN::sendCanFrame(MCANMSG can_frame){
	uint32_t txId;
	if ((can_frame.cmd == SWITCH_ACC) && (can_frame.resp_bit))
	{
		txId = 0x00170000;
	} else if ((can_frame.cmd == PING) && (can_frame.resp_bit))
	{
		txId = 0x00310000;
	} else {
		txId = 0x00000000;
	}
	txId = txId | can_frame.hash;

	can.sendMsgBuf(txId, 1, can_frame.dlc, can_frame.data);
}

MCANMSG MCAN::getCanFrame(){

	MCANMSG can_frame;
	can.readMsgBuf(&can_frame.dlc, can_frame.data);
	rxId = can.getCanId();
	can_frame.cmd = rxId >> 17;
	can_frame.hash = rxId;
	can_frame.resp_bit = bitRead(rxId, 16);

	Serial.print("received Data: ");
	
	return can_frame;
}

void MCAN::printCanFrame(MCANMSG can_frame){
	Serial.print(can_frame.cmd, HEX);
	Serial.print(" ");
	Serial.print(can_frame.resp_bit);
	Serial.print(" ");
	Serial.print(can_frame.hash, HEX);
	Serial.print(" ");
	Serial.print(can_frame.dlc);
	for (int i = 0; i < 8; ++i)
	{
		Serial.print(" ");
		Serial.print(can_frame.data[i], HEX);
	}
	Serial.println();

}
