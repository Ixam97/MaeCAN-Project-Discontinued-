/*
 * MäCAN-Basismodul, Software-Version 0.1
 * 
 *  Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *  Do with this whatever you want, but keep thes Header and tell
 *  the others what you changed!
 *  
 *  Last edited: 2016-10-14
 */
/*
 * Allgemeine Konstanten:
 */

#define VERS_HIGH 0       //Versionsnummer vor dem Punkt
#define VERS_LOW 1        //Versionsnummer nach dem Punkt

#define CONFIG_NUM 10     //Anzahl der Konfigurationspunkte
#define BOARD_NUM 1       //Identifikationsnummer des Boards (Anzeige in der CS2)

#define SERVO_MIN 70
#define SERVO_MAX 110
#define SERVO_SPEED 20 //(ms/°)

#include <MCAN.h>
#include <EEPROM.h>
#include <Servo.h>

#define UID 0x87654321    //CAN-UID
byte uid_mat[4];

uint16_t hash;

bool config_poll = false;

uint16_t acc_locid[] = {0x3800, 0x3801, 0x3802, 0x3803};
bool acc_state_is[] = {0,0,0,0};
bool acc_state_set[] = {0,0,0,0};

int config_index = 0;

MCAN mcan;
MCANMSG can_frame_out;
MCANMSG can_frame_in;

CanDevice device;

Servo servo_0;
Servo servo_1;
Servo servo_2;
Servo servo_3;


void setup() {

  uid_mat[0] = UID >> 24;
  uid_mat[1] = UID >> 16;
  uid_mat[2] = UID >> 8;
  uid_mat[3] = UID;

  //Geräteinformationen:
  device.versHigh = VERS_HIGH;
  device.versLow = VERS_LOW;
  device.hash = hash;
  device.uid = UID;
  device.artNum = "Servo";
  device.name = "MäCAN Decoder";
  device.boardNum = BOARD_NUM;
  device.type = 0x0050;

  servo_0.attach(8);
  servo_1.attach(7);
  servo_2.attach(6);
  servo_3.attach(5);

  pinMode(9, OUTPUT);

  hash = mcan.generateHash(UID);
  mcan.initMCAN(true);
  
  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
  
  for(int i = 0; i < 4; i++){
    switchAcc(i);
  }
}

void accFrame(){  
  if((can_frame_in.cmd == SWITCH_ACC) && (can_frame_in.resp_bit == 0)){     //Abhandlung bei gültigem Weichenbefehl
    uint16_t locid = (can_frame_in.data[2] << 8) | can_frame_in.data[3];
    for(int i = 0; i < 4; i++){
      if(locid == acc_locid[i]){                                              //Auf benutzte Adresse überprüfen
        acc_state_set[i] = can_frame_in.data[4];
        break;
      }
    }
  }
}

void pingFrame(){  
  if((can_frame_in.cmd == PING) && (can_frame_in.resp_bit == 0)){          //Auf Ping Request antworten
    mcan.sendPingResponse(device);
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

void statusResponse(int chanel){
  can_frame_out.cmd = SYS_CMD;
  can_frame_out.hash = hash;
  can_frame_out.resp_bit = true;
  can_frame_out.dlc = 7;
  for(int i = 0; i < 4; i++){
    can_frame_out.data[i] = uid_mat[i];
  }
  can_frame_out.data[4] = SYS_STAT;
  can_frame_out.data[5] = chanel;
  can_frame_out.data[6] = true;
  can_frame_out.data[7] = 0;
  
  mcan.sendCanFrame(can_frame_out);
}

/*
 * Meldung nach erfolgreichem Schalten
 */
void switchAccResponse(int acc_num, bool set_state){
  can_frame_out.cmd = SWITCH_ACC;
  can_frame_out.hash = hash;
  can_frame_out.resp_bit = true;
  can_frame_out.dlc = 6;
  can_frame_out.data[0] = 0;
  can_frame_out.data[1] = 0;
  can_frame_out.data[2] = acc_locid[acc_num] >> 8;
  can_frame_out.data[3] = acc_locid[acc_num];
  can_frame_out.data[4] = set_state;            /* Meldung der Lage für Märklin-Geräte.*/
  can_frame_out.data[5] = 0;

  mcan.sendCanFrame(can_frame_out);
  
  can_frame_out.data[4] = 0xfe - set_state;     /* Meldung für CdB-Module und Rocrail Feldereignisse. */
  
  mcan.sendCanFrame(can_frame_out);
}

void switchAcc(int num){
  digitalWrite(9, 0);
  switch(num){
    case 0 : 
      acc_state_is[0] = acc_state_set[0];
      slowServo(&servo_0, acc_state_set[0]);
      break;
    case 1 :
      acc_state_is[1] = acc_state_set[1];
      slowServo(&servo_1, acc_state_set[1]);
      break;
    case 2 :
      acc_state_is[2] = acc_state_set[2];
      slowServo(&servo_2, acc_state_set[2]);
      break;
    case 3 :
      acc_state_is[3] = acc_state_set[3];
      slowServo(&servo_3, acc_state_set[3]);
      break;
      
  }
  switchAccResponse(num, acc_state_is[num]);
  digitalWrite(9,1);
}

void slowServo(Servo *servo, bool state){
  if(state){
    for(int i = SERVO_MIN + 1; i <= SERVO_MAX; i++){
      servo->write(i);
      delay(SERVO_SPEED);
    }
  }else{
    for(int i = SERVO_MAX - 1; i >= SERVO_MIN; i--){
      servo->write(i);
      delay(SERVO_SPEED);
    }
  }
}

/*
 * Ausführen, wenn eine Nachricht verfügbar ist.
 * Nachricht wird geladen und anhängig vom CAN-Befehl verarbeitet.
 */
void interruptFn(){
  can_frame_in = mcan.getCanFrame();
  accFrame();
  pingFrame();
  configFrame();
  //statusFrame();
}

void loop() {
  for(int i = 0; i < 4; i++){
    if(acc_state_is[i] != acc_state_set[i]){
      switchAcc(i);
    }
  }
  if(config_poll){
    if(config_index == 0) mcan.sendDeviceInfo(device, CONFIG_NUM);
    if(config_index == 1) mcan.sendConfigInfoSlider(device, 1, 10, 90, 40, "Winkel_10_90_°");
    if(config_index == 2) mcan.sendConfigInfoSlider(device, 2, 1, 20, 5, "Geschwindigkeit_1_20_ms/°");
    if(config_index == 3) mcan.sendConfigInfoDropdown(device, 3, 2, 0, "Protokoll Ausgang 1_DCC_MM");
    if(config_index == 4) mcan.sendConfigInfoSlider(device, 4, 1, 2048, 0, "Adresse Ausgang 1_1_2048");
    if(config_index == 5) mcan.sendConfigInfoDropdown(device, 5, 2, 0, "Protokoll Ausgang 2_DCC_MM");
    if(config_index == 6) mcan.sendConfigInfoSlider(device, 6, 1, 2048, 0, "Adresse Ausgang 2_1_2048");
    if(config_index == 7) mcan.sendConfigInfoDropdown(device, 7, 2, 0, "Protokoll Ausgang 3_DCC_MM");
    if(config_index == 8) mcan.sendConfigInfoSlider(device, 8, 1, 2048, 0, "Adresse Ausgang 3_1_2048");
    if(config_index == 9) mcan.sendConfigInfoDropdown(device, 9, 2, 0, "Protokoll Ausgang 4_DCC_MM");
    if(config_index == 10) mcan.sendConfigInfoSlider(device, 10, 1, 2048, 0, "Adresse Ausgang 4_1_2048");
    config_poll = false;
  }
}


