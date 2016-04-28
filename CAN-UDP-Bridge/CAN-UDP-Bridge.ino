/*
 * Für UDP-Teil notwendige Bibliotheken einbinden:
 */
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

typedef struct {
  uint32_t id;
  uint8_t dlc;
  byte data[8];
} CANBUF;

/*
 * Für CAN-Teil notwendige Bibliotheken einbinden:
 */
#include <mcp_can.h>
#include <SPI.h>

#define INT 2     //Interrupt-Pin
#define CS 15     //Chip-Select-Pin

#define PORT_LISTEN 15730
#define PORT_SENDTO 15731

MCP_CAN CAN(15);
WiFiUDP UDP;

const char* ssid = "#######";
const char* password = "########";

byte udpRxBuf[13];
byte udpTxBuf[13];

CANBUF canTxBuf;
CANBUF canRxBuf;

IPAddress broadcast(255,255,255,255);

void canToUdp(CANBUF canBuf){
  udpTxBuf[4] = canBuf.dlc;
  udpTxBuf[3] = canBuf.id;
  udpTxBuf[2] = canBuf.id >> 8;
  udpTxBuf[1] = canBuf.id >> 16;
  udpTxBuf[0] = canBuf.id >> 24;
  for(int i = 0; i < 8; i++){
    udpTxBuf[5+i] = canBuf.data[i];
  }
  Serial.print("CAN -> UDP :: ID: ");
  for(int i = 0; i < 4; i++){
    if(udpTxBuf[i] < 0x10){
      Serial.print("0");
    }
    Serial.print(udpTxBuf[i],HEX);
  }
  Serial.print(" DLC: ");
  Serial.print(udpTxBuf[4], HEX);
  Serial.print(" DATA: ");
  for(int i = 0; i < udpTxBuf[4]; i++){
    if(udpTxBuf[5 + i] < 0x10){
      Serial.print("0");
    }
    Serial.print(udpTxBuf[5 + i],HEX);
    Serial.print(" ");
  }
  Serial.print("\r\n");
}

CANBUF udpToCan(){
  CANBUF canBuf;
  canBuf.dlc = udpRxBuf[4];
  canBuf.id = (udpRxBuf[0] << 24) | (udpRxBuf[1] << 16) | (udpRxBuf[2] << 8) | udpRxBuf[3];
  for(int i = 0; i < 8; i++){
    canBuf.data[i] = udpRxBuf[5+i];
  }
  Serial.print("UDP -> CAN :: ID: ");
  for(int i = 0; i < 4; i++){
    if(udpRxBuf[i] < 0x10){
      Serial.print("0");
    }
    Serial.print(udpRxBuf[i],HEX);
  }
  Serial.print(" DLC: ");
  Serial.print(udpRxBuf[4], HEX);
  Serial.print(" DATA: ");
  for(int i = 0; i < udpRxBuf[4]; i++){
    if(udpRxBuf[5 + i] < 0x10){
      Serial.print("0");
    }
    Serial.print(udpRxBuf[5 + i],HEX);
    Serial.print(" ");
  }
  Serial.print("\r\n");
}

void sendCan(CANBUF canBuf){
  CAN.sendMsgBuf(canBuf.id, 1, canBuf.dlc, canBuf.data);
}

CANBUF getCan(){
  CANBUF canBuf;
  CAN.readMsgBuf(&canBuf.dlc, canBuf.data);
  canBuf.id = CAN.getCanId();
}

void sendUdp(byte udpBuf[13]){
  UDP.beginPacket(broadcast, PORT_SENDTO);
  UDP.write(udpBuf, 13);
  UDP.endPacket();
  
}

void getUdp(){
  UDP.read(udpRxBuf, 13);
  UDP.flush();
  
}

void setup() {
  //CAN-Bus initialisieren:
  Serial.begin(9600);
  Serial.print("\r\n\r\n");
  Serial.print("Starting CAN-UDP-Bridge ...\r\n");
  Serial.print("Initialising CAN ... ");
  if(CAN.begin(CAN_250KBPS) == CAN_OK){
    Serial.print("successfull!\r\n");
  }else{
    Serial.print("failed!\r\n");
    while(1);
  }
  
  //Mit dem WiFi verbinden:
  Serial.print("Connecting to '");
  Serial.print(ssid);
  Serial.print("' ");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.print(" connected!\r\n");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print("\r\n");

  //UDP-Socket öffnen:
  Serial.print("Opening UDP-socket on port ");
  Serial.print(PORT_LISTEN);
  Serial.print(" ... ");
  if(UDP.begin(PORT_LISTEN)){
    Serial.print("succsessfull!\r\n");
  }else{
    Serial.print("failure!\r\n");
    while(1);
  }
  pinMode(INT, INPUT);

}

void loop() {
  if(!digitalRead(INT)){
    canRxBuf = getCan();
    canToUdp(canRxBuf);
    sendUdp(udpTxBuf);
  }

  int udpPacketSize = UDP.parsePacket();
  if(udpPacketSize){
    getUdp();
    canTxBuf = udpToCan();
    sendCan(canTxBuf);
  }

}

