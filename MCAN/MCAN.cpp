/*
 *	Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *	Do with this whatever you want, but keep thes Header and tell
 *	the others what you changed1
 *
 *	Last changed: 2016-04-23
 */


#include "Arduino.h"
#include "MCAN.h"
#include "mcp_can.h"

MCP_CAN can(10);

uint32_t rxId;

uint16_t MCAN::generateHash(uint32_t uid){
	uint16_t highbyte = uid >> 16;
	uint16_t lowbyte = uid;
	uint16_t hash = highbyte ^ lowbyte;
	bitWrite(hash, 7, 0);
	bitWrite(hash, 8, 1);
	bitWrite(hash, 9, 1);
	return hash;
}

void MCAN::initMCAN(){
	pinMode(9,OUTPUT);
	digitalWrite(9,0);
	if (can.begin(CAN_250KBPS) == CAN_OK){
		digitalWrite(9,1);
	}
}

void MCAN::sendCanFrame(MCANMSG can_frame){
	uint32_t txId;
	if ((can_frame.cmd == SWITCH_ACC) && (can_frame.resp_bit)){
		txId = 0x00170000;
	} else if ((can_frame.cmd == SYS_CMD) && (can_frame.resp_bit)){
		txId = 0x00010000;
	} else if ((can_frame.cmd == PING) && (can_frame.resp_bit)){
		txId = 0x00310000;
	} else if ((can_frame.cmd == CONFIG) && (can_frame.resp_bit)){
		txId = 0x003b0000;
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
	
	return can_frame;
}