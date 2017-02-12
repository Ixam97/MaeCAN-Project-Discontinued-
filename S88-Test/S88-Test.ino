#define DEBUG 1

#define VERS_HIGH 0       //Versionsnummer vor dem Punkt
#define VERS_LOW 1        //Versionsnummer nach dem Punkt

#define CONFIG_NUM 4     //Anzahl der Konfigurationspunkte
#define BOARD_NUM 1       //Identifikationsnummer des Boards (Anzeige in der CS2)
#define UID 0x12345678    //CAN UID

#include <MCAN.h>
#include <EEPROM.h>

byte      uid_mat[4];
uint16_t  hash;
bool      config_poll   = false;
int       config_index  = 0;

typedef struct {
  uint8_t contact;
  bool state;
  int bus;
  int module;
  int mod_contact;
} S88C;

S88C s88c;

MCAN mcan;
MCANMSG can_frame_out;
MCANMSG can_frame_in;

CanDevice device;

void setup(){

  hash = mcan.generateHash(UID);

  s88c.contact = 0;

  int starting_contact = (mcan.getConfigData(3)-1)*16 + mcan.getConfigData(4);
  
  uid_mat[0] = UID >> 24;
  uid_mat[1] = UID >> 16;
  uid_mat[2] = UID >> 8;
  uid_mat[3] = UID;

  //Geräteinformationen:
  device.versHigh = VERS_HIGH;
  device.versLow = VERS_LOW;
  device.hash = hash;
  device.uid = UID;
  device.artNum = "GBS S88";
  device.name = "MäCAN GBS S88";
  device.boardNum = BOARD_NUM;
  device.type = 0x2234;

  pinMode(9, OUTPUT);

  mcan.initMCAN(DEBUG, device);

  #ifdef DEBUG
  for(int i = 1; i <= CONFIG_NUM; i++){
    Serial.print("Konfig-Kanal "); Serial.print(i); Serial.print(": ");
    Serial.println(mcan.getConfigData(i));
  }
  Serial.print("Startkontakt: "); Serial.println(starting_contact);
  #endif

  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
}

void pingFrame(){  
  if((can_frame_in.cmd == PING) && (can_frame_in.resp_bit == 0)){          //Auf Ping Request antworten
    mcan.sendPingFrame(device, 1);
  }
}

void configFrame(){
  if((can_frame_in.cmd == CONFIG) && (can_frame_in.resp_bit == 0)){
    if((uid_mat[0] == can_frame_in.data[0])&&(uid_mat[1] == can_frame_in.data[1])&&(uid_mat[2] == can_frame_in.data[2])&&(uid_mat[3] == can_frame_in.data[3])){
      config_poll = true;
      config_index = can_frame_in.data[4];
    }
  }
}

void statusFrame(){
  if((can_frame_in.cmd == SYS_CMD) && (can_frame_in.resp_bit == 0) && (can_frame_in.data[4] == SYS_STAT)){
    if((uid_mat[0] == can_frame_in.data[0])&&(uid_mat[1] == can_frame_in.data[1])&&(uid_mat[2] == can_frame_in.data[2])&&(uid_mat[3] == can_frame_in.data[3])){
      mcan.saveConfigData(device, can_frame_in);
    }
  }
}

void contactFrame(){
  if((can_frame_in.cmd == S88_EVENT) && (can_frame_in.resp_bit == 1)){
    if(((can_frame_in.data[0] << 8) + can_frame_in.data[1]) == mcan.getConfigData(1)){
      s88c.contact = (can_frame_in.data[2] << 8) + can_frame_in.data[3];
      s88c.state = can_frame_in.data[5];
    }
  }
}

void interruptFn(){
  can_frame_in = mcan.getCanFrame();
  pingFrame();
  configFrame();
  statusFrame();
  contactFrame();
}

void loop(){
  if (config_poll){
    if(config_index == 0) mcan.sendDeviceInfo(device, CONFIG_NUM);
    if(config_index == 1) mcan.sendConfigInfoSlider(device, 1, 0, 65534, mcan.getConfigData(1), "Gerätekenner:_0_65534");
    if(config_index == 2) mcan.sendConfigInfoDropdown(device, 2, 4, mcan.getConfigData(2), "Bus:_L88 0/CS_L88 1_L88 2_L88 3");
    if(config_index == 3) mcan.sendConfigInfoSlider(device, 3, 1, 31, mcan.getConfigData(3), "Beginnt bei Modul_1_31");
    if(config_index == 4) mcan.sendConfigInfoSlider(device, 4, 1, 16, mcan.getConfigData(4), "Kontakt_1_16");
  config_poll = false;
  }
  if(s88c.contact != 0){
    if(s88c.contact > 3000){
      s88c.bus = 3;
      s88c.module = ((s88c.contact - 3000) / 16) +1;
      s88c.mod_contact = (s88c.contact - 3000) % 16;
    }
    else if(s88c.contact > 2000){
      s88c.bus = 3;
      s88c.module = ((s88c.contact - 2000) / 16) +1;
      s88c.mod_contact = (s88c.contact - 2000) % 16;
    }
    else if(s88c.contact > 1000){
      s88c.bus = 3;
      s88c.module = ((s88c.contact - 1000) / 16) +1;
      s88c.mod_contact = (s88c.contact - 1000) % 16;
    }
    else
    {
      s88c.bus = 0;
      s88c.module = ((s88c.contact) / 16) +1;
      s88c.mod_contact = (s88c.contact) % 16;
    }
    Serial.print("Bus "); Serial.print(s88c.bus);
    Serial.print(", Modul "); Serial.print(s88c.module);
    Serial.print(", Kontakt "); Serial.print(s88c.mod_contact);
    if(s88c.state) Serial.println(": belegt."); else Serial.println(": frei");
    s88c.contact = 0;
  }
}













