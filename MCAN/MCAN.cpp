/*
 *	Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *	Do with this whatever you want, but keep thes Header and tell
 *	the others what you changed1
 *
 *	Last changed: 2016-10-14
 */


#include <Arduino.h>
#include <MCAN.h>
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

uint16_t MCAN::generateLocId(uint16_t prot, uint16_t adrs){
	return (prot + adrs - 1);
}

void MCAN::initMCAN(bool debug){
	if(debug) Serial.begin(250000);
	pinMode(9,OUTPUT);
	digitalWrite(9,0);
	if (can.begin(CAN_250KBPS) == CAN_OK){
		digitalWrite(9,1);
		Serial.println("CAN-Init Successfull!");
	}
}
void MCAN::initMCAN(){
	pinMode(9,OUTPUT);
	digitalWrite(9,0);
	if (can.begin(CAN_250KBPS) == CAN_OK){
		digitalWrite(9,1);
	}
}

void MCAN::sendCanFrame(MCANMSG can_frame){
	uint32_t txId = can_frame.cmd;
	txId = (txId << 17) | can_frame.hash;
	bitWrite(txId, 16, can_frame.resp_bit);

	can.sendMsgBuf(txId, 1, can_frame.dlc, can_frame.data);
}

void MCAN::sendDeviceInfo(CanDevice device, int configNum){
	/*
	 *  Sendet die Basisinformationen über das Gerät an die GUI der CS2.
	 */
	MCANMSG can_frame;
	int frameCounter = 0;

	uint8_t buffer[8] = {0, configNum, 0, 0, 0, 0, 0, device.boardNum};
	can_frame.cmd = CONFIG;
	can_frame.resp_bit = 1;
	can_frame.dlc = 8;
	for(int i = 0; i < 8; i++){
		can_frame.data[i] = buffer[i];
	}
	can_frame.hash = 0x0301;

	sendCanFrame(can_frame);
	frameCounter++;
	Serial.println(can_frame.hash, HEX);


	for(int i = 0; i < 8; i++){
		buffer[i] = 0;
	}
	device.artNum.getBytes(buffer,8);
	for(int i = 0; i < 8; i++){
		can_frame.data[i] = buffer[i];
	}
	can_frame.hash = 0x0302;

	sendCanFrame(can_frame);
	frameCounter++;
	Serial.println(can_frame.hash, HEX);

	int nameLen = device.name.length();
	int neededFrames;

	if(nameLen % 8) neededFrames = (nameLen / 8) + 1;
	else neededFrames = nameLen / 8;

	for(int i = 0; i < neededFrames; i++){
		for(int j = 0; j < 8; j++){
			if((8*i)+j < nameLen) can_frame.data[j] = device.name[(8*i)+j];
			else can_frame.data[j] = 0;
		}
		can_frame.hash = 0x0303 + i;

		sendCanFrame(can_frame);
		frameCounter++;
	Serial.println(can_frame.hash, HEX);
	}


	for(int i = 0; i < 8; i++){
		buffer[i] = 0;
	}
	can_frame.hash = device.hash;
	can_frame.dlc = 6;
	can_frame.data[0] = device.uid >> 24;
	can_frame.data[1] = device.uid >> 16;
	can_frame.data[2] = device.uid >> 8;
	can_frame.data[3] = device.uid;
	can_frame.data[4] = 0;
	can_frame.data[5] = frameCounter;

	sendCanFrame(can_frame);
	Serial.println(can_frame.hash, HEX);
}

void MCAN::sendConfigInfoDropdown(CanDevice device, uint8_t configChanel, uint8_t numberOfOptions, uint8_t defaultSetting, String settings){
	MCANMSG can_frame;
	int frameCounter;

	can_frame.cmd = CONFIG;
	can_frame.resp_bit = 1;
	can_frame.hash = 0x0301;
	can_frame.dlc = 8;

	//Erster Frame mit Grundinformationen:

	can_frame.data[0] = configChanel;
	can_frame.data[1] = 1;
	can_frame.data[2] = numberOfOptions;
	can_frame.data[3] = defaultSetting;
	for(int i = 4; i < 8; i++){
		can_frame.data[i] = 0;
	}

	sendCanFrame(can_frame);
	can_frame.hash++;
	frameCounter++;
	Serial.println(frameCounter);

	//Frames, die Strings enthalten:

	int length = settings.length();
	int neededFrames;

	if(length % 8) neededFrames = (length / 8) + 1;
	else neededFrames = length / 8;

	for(int i = 0; i < neededFrames; i++){
		for(int j = 0; j < 8; j++){
			if(((8*i)+j < length) && (settings[(8*i)+j] != '_')) can_frame.data[j] = settings[(8*i)+j];
			else can_frame.data[j] = 0;
		}

		sendCanFrame(can_frame);
		can_frame.hash++;
		frameCounter++;
		Serial.println(frameCounter);
	}

	//Abschließender bestätigungsframe:

	can_frame.hash = device.hash;
	can_frame.dlc = 6;
	can_frame.data[0] = device.uid >> 24;
	can_frame.data[1] = device.uid >> 16;
	can_frame.data[2] = device.uid >> 8;
	can_frame.data[3] = device.uid;
	can_frame.data[4] = configChanel;
	can_frame.data[5] = frameCounter;

	sendCanFrame(can_frame);
}

void MCAN::sendConfigInfoSlider(CanDevice device, uint8_t configChanel, uint32_t lowerValue, uint32_t upperValue, uint32_t defaultValue, String settings){
	MCANMSG can_frame;
	int frameCounter;

	can_frame.cmd = CONFIG;
	can_frame.resp_bit = 1;
	can_frame.hash = 0x0301;
	can_frame.dlc = 8;

	//Erster Frame mit Grundinformationen:

	can_frame.data[0] = configChanel;
	can_frame.data[1] = 2;
	can_frame.data[2] = lowerValue >> 8;
	can_frame.data[3] = lowerValue;
	can_frame.data[4] = upperValue >> 8;
	can_frame.data[5] = upperValue;
	can_frame.data[6] = defaultValue >> 8;
	can_frame.data[7] = defaultValue;

	sendCanFrame(can_frame);
	can_frame.hash++;
	frameCounter++;
	Serial.println(frameCounter);

	//Frames, die Strings enthalten:

	int length = settings.length();
	int neededFrames;

	if(length % 8) neededFrames = (length / 8) + 1;
	else neededFrames = length / 8;

	for(int i = 0; i < neededFrames; i++){
		for(int j = 0; j < 8; j++){
			if(((8*i)+j < length) && (settings[(8*i)+j] != '_')) can_frame.data[j] = settings[(8*i)+j];
			else can_frame.data[j] = 0;
		}

		sendCanFrame(can_frame);
		can_frame.hash++;
		frameCounter++;
		Serial.println(frameCounter);
	}

	//Abschließender bestätigungsframe:

	can_frame.hash = device.hash;
	can_frame.dlc = 6;
	can_frame.data[0] = device.uid >> 24;
	can_frame.data[1] = device.uid >> 16;
	can_frame.data[2] = device.uid >> 8;
	can_frame.data[3] = device.uid;
	can_frame.data[4] = configChanel;
	can_frame.data[5] = frameCounter;

	sendCanFrame(can_frame);
}

void MCAN::sendPingResponse(CanDevice device){
	MCANMSG can_frame;
	can_frame.cmd = PING;
	can_frame.resp_bit = 1;
	can_frame.dlc = 8;
	can_frame.data[0] = device.uid >> 24;
	can_frame.data[1] = device.uid >> 16;
	can_frame.data[2] = device.uid >> 8;
	can_frame.data[3] = device.uid;
	can_frame.data[4] = device.versHigh;
	can_frame.data[5] = device.versLow;
	can_frame.data[6] = device.type >> 8;
	can_frame.data[7] = device.type;

	sendCanFrame(can_frame);
	Serial.println("Sent ping response...");
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