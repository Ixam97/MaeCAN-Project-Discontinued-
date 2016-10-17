/*
 *	Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *  If you need Help, feel free to contact me any time!
 *	Do with this whatever you want, but keep thes Header and tell
 *	the others what you changed!
 *
 *	Last changed: 2016-10-17
 */


#include <Arduino.h>
#include <MCAN.h>
#include "mcp_can.h"

MCP_CAN can(10);

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
	Serial.println(canFrameToString(can_frame, 1));
}

void MCAN::sendDeviceInfo(CanDevice device, int configNum){
	/*
	 *  Sendet die Basisinformationen über das Gerät an die GUI der CS2.
	 */
	MCANMSG can_frame;
	int frameCounter = 0;

	can_frame.cmd = CONFIG;
	can_frame.resp_bit = 1;
	can_frame.dlc = 8;
	for(int i = 0; i < 8; i++){can_frame.data[i] = 0;}
	can_frame.data[1] = configNum;
	can_frame.data[7] = device.boardNum;
	can_frame.hash = 0x0301;

	sendCanFrame(can_frame);
	frameCounter++;

	for(int i = 0; i < 8; i++) { can_frame.data[i] = 0; }
	device.artNum.getBytes(can_frame.data,8);

	can_frame.hash++;
	sendCanFrame(can_frame);
	frameCounter++;

	int nameLen = device.name.length();
	int neededFrames;

	if(nameLen % 8) neededFrames = (nameLen / 8) + 1;
	else neededFrames = nameLen / 8;

	for(int i = 0; i < neededFrames; i++){
		for(int j = 0; j < 8; j++){
			if((8*i)+j < nameLen) can_frame.data[j] = device.name[(8*i)+j];
			else can_frame.data[j] = 0;
		}
		can_frame.hash++;
		sendCanFrame(can_frame);
		frameCounter++;
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

void MCAN::sendConfigInfoSlider(CanDevice device, uint8_t configChanel, uint16_t lowerValue, uint16_t upperValue, uint16_t defaultValue, String settings){
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

void MCAN::sendPingFrame(CanDevice device, bool response){
	MCANMSG can_frame;
	can_frame.cmd = PING;
	can_frame.hash = device.hash;
	can_frame.resp_bit = response;
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
}

void MCAN::sendAccessoryFrame(CanDevice device, uint32_t locId, bool state, bool response){

	MCANMSG can_frame;

	can_frame.cmd = SWITCH_ACC;
	can_frame.resp_bit = response;
	can_frame.hash = device.hash;
	can_frame.dlc = 6;
	can_frame.data[0] = 0;
	can_frame.data[1] = 0;
	can_frame.data[2] = locId >> 8;
	can_frame.data[3] = locId;
	can_frame.data[4] = state;
	can_frame.data[5] = 0;

	sendCanFrame(can_frame);

	can_frame.data[4] = 0xfe - state;

	sendCanFrame(can_frame);
}

MCANMSG MCAN::getCanFrame(){
	uint32_t rxId;

	MCANMSG can_frame;
	can.readMsgBuf(&can_frame.dlc, can_frame.data);
	rxId = can.getCanId();
	can_frame.cmd = rxId >> 17;
	can_frame.hash = rxId;
	can_frame.resp_bit = bitRead(rxId, 16);

	//Serial.println(canFrameToString(can_frame, 0));
	
	return can_frame;
}

int MCAN::checkAccessoryFrame(MCANMSG can_frame, uint16_t locIds[], int accNum, bool response){

	uint16_t currentLocId = (can_frame.data[2] << 8) | can_frame.data[3];

	if((can_frame.cmd == SWITCH_ACC) || (can_frame.resp_bit == response)){
		for( int i = 0; i < accNum; i++){
			if(currentLocId == locIds[i]) return i;
		}
	}

	return -1;
}

void MCAN::saveConfigData(MCANMSG can_frame, uint8_t memory[]){

	int chanel = can_frame.data[5] - 1;

	memory[chanel*2] = can_frame.data[6];
	memory[(chanel*2) + 1] = can_frame.data[7];
}

String MCAN::canFrameToString(MCANMSG can_frame, bool direction){

	String direction_s;
	String cmd = String(can_frame.cmd, HEX);
	String hash = String(can_frame.hash, HEX);

	if(can_frame.cmd < 0x10) cmd = String("0x0" + cmd);
	else cmd = String("0x" + cmd);

	if(can_frame.hash < 0x10) hash = String("0x000" + hash);
	else if (can_frame.hash < 0x100) hash = String("0x00" + hash);
	else if (can_frame.hash < 0x1000) hash = String("0x0" + hash);
	else hash = String("0x" + hash);

	String data;
	for(int i = 0; i < can_frame.dlc; i++){
		String byte_s = String(can_frame.data[i], HEX);
		if(can_frame.data[i] < 0x10) data = String(data + " 0x0" + byte_s);
		else data = String(data + " 0x" + byte_s);
	}

	if(!direction) direction_s = "IN:  ";
	else direction_s = "OUT: ";

	return (String(direction_s + cmd + " Response: " + can_frame.resp_bit + " Hash: " + hash + " DLC: " + can_frame.dlc + " Data:" + data));
}