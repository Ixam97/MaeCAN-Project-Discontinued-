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
#include <ESP8266WebServer.h>

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

#define MODE 0       //Betriebsart: 0: An dem Tragbaren Gerät; 1: An der Gleisbox


#if MODE == 0
#define PORT_LISTEN 15733
#define PORT_SENDTO 15734

#else if MODE == 1
#define PORT_LISTEN 15733
#define PORT_SENDTO 15732

#endif

ESP8266WebServer server(80);

MCP_CAN CAN(15);
WiFiUDP UDP;

const char* ssid = "SlowRotkohl";
const char* password = "Hackbraten";

byte udpRxBuf[13];
byte udpTxBuf[13];

CANBUF canTxBuf;
CANBUF canRxBuf;

IPAddress broadcast(192,168,1,128);

void handleRoot() {
  server.send(200, "text/html", "<h1>MS2-WiFi Access Point</h1>");
}

void canToUdp(){
  udpTxBuf[4] = canRxBuf.dlc;
  udpTxBuf[3] = canRxBuf.id;
  udpTxBuf[2] = canRxBuf.id >> 8;
  udpTxBuf[1] = canRxBuf.id >> 16;
  udpTxBuf[0] = canRxBuf.id >> 24;
  for(int i = 0; i < 8; i++){
    udpTxBuf[5+i] = canRxBuf.data[i];
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

void udpToCan(){
  CANBUF canBuf;
  canTxBuf.dlc = udpRxBuf[4];
  canTxBuf.id = (udpRxBuf[0] << 24) | (udpRxBuf[1] << 16) | (udpRxBuf[2] << 8) | udpRxBuf[3];
  for(int i = 0; i < 8; i++){
    canTxBuf.data[i] = udpRxBuf[5+i];
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

void sendCan(){
  CAN.sendMsgBuf(canTxBuf.id, 1, canTxBuf.dlc, canTxBuf.data);
}

void getCan(){
  CAN.readMsgBuf(&canRxBuf.dlc, canRxBuf.data);
  canRxBuf.id = CAN.getCanId();
}

void sendUdp(){
  UDP.beginPacket(broadcast, PORT_SENDTO);
  UDP.write(udpTxBuf, 13);
  UDP.endPacket();  
}

void interruptFn(){
    getCan();
    canToUdp();
    sendUdp();
}

void setup() {
  //CAN-Bus initialisieren:
  Serial.begin(250000);
  Serial.print("\r\n\r\n");
  Serial.print("Starting CAN-UDP-Bridge ...\r\n");
  Serial.print("Initialising CAN ... ");
  if(CAN.begin(CAN_250KBPS) == CAN_OK){
    Serial.print("successfull!\r\n");
  }else{
    Serial.print("failed!\r\n");
    while(1);
  }
#if MODE == 0
  //Mit dem WiFi verbinden:
  Serial.print("Connecting to '");
  Serial.print(ssid);
  //Serial.print("' ");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.print(" connected!\r\n");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print("\r\n");

#else if MODE == 1
  WiFi.softAP(ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

#endif
    
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

  attachInterrupt(INT, interruptFn, LOW);

}

void loop() {
#if MODE == 1
  server.handleClient();
#endif
  
  int packetsize = UDP.parsePacket();
  if(packetsize){
    for(int i = 0; i < 13; i++){
      udpRxBuf[i] = UDP.read();
    }
    udpToCan();
    sendCan();
  }

}

