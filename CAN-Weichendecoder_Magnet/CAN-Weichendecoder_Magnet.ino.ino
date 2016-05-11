/*
 * MäCAN-Weichendecoder Rev A, Software-Version 0.4
 * 
 *  Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *  Do with this whatever you want, but keep thes Header and tell
 *  the others what you changed!
 *  
 *  Last edited: 2016-05-11
 */

#define MANUAL_MODE false   //Programmierung über CS2 (false) oder direkt im Sketch (true)

/*
 * Konstanten für den manuellen Modus:
 */

 #define ADRS_1 1
 #define ADRS_2 2
 #define ADRS_3 3
 #define ADRS_4 4

 #define PROT_1 DCC_ACC     
 #define PROT_3 DCC_ACC
 #define PROT_3 DCC_ACC
 #define PROT_4 DCC_ACC

 #define SWITCHTIME 200     //Schaltzeit in ms (20 bis 1000)
 #define STWITCHMODE 0      //0 = Moment; 1 = Dauer
 #define FEEDBACK true

/*
 * Allgemeine Konstanten:
 */

#define VERS_HIGH 0       //Versionsnummer vor dem Punkt
#define VERS_LOW 3        //Versionsnummer nach dem Punkt

#define CONFIG_NUM 11     //Anzahl der Konfigurationspunkte
#define BOARD_NUM 1       //Identifikationsnummer des Boards (Anzeige in der CS2)

#include <MCAN.h>
#include <EEPROM.h>

#define UID 0x12345678    //CAN-UID
byte uid_mat[4];

const int adrs_channel[4] = {5,7,9,11};       //Konfigkanäle für die Adressen
const int prot_channel[4] = {4,6,8,10};       //Konfigkanäle für das Protokoll

const int reg_locid[4] = {0x10, 0x12, 0x14, 0x16};    //EEPROM-Register der Local-IDs
const int reg_adrs[4] = {0x20, 0x22, 0x24, 0x26};     //EEPROM-Register der Adressen
const int reg_prot[4] = {0x30, 0x31, 0x32, 0x33};     //EEPROM-Register der Protokolle

/*
 * Variablen der Magnetartikel:
 */
uint16_t acc_locid[4];
bool acc_got_cmd[4];
bool acc_state_is[4];
bool acc_state_set[4];
bool acc_state_rm[4];

const int acc_pin_grn[4] = {0,3,5,7};
const int acc_pin_red[4] = {1,4,6,8};
const int acc_pin_rm[4] = {A3,A2,A1,A0};

uint16_t hash;

int config_index = 0;
bool config_poll = false;

int status_chanel = 0;
bool status_set = false;

MCAN mcan;
MCANMSG can_frame_out;
MCANMSG can_frame_in;


void setup() {

  #if MANUAL_MODE

  EEPROM.put(0,SWITCHMODE);
  EEPROM.put(1,FEEDBACK);

  uint16_t switchtime;
  if(SWITCHTIME < 20) switchtime = 20;
  else if(SWITCHTIME > 1000) switchtime = 1000;
  else switchtime = SWITCHTIME;
  byte switchtime_high = switchtime >> 8;
  byte switchtime_low = switchtime;
  EEPROM.put(2,switchtime_high);
  EEPROM.put(3,switchtime_low);

  uint16_t locid_1 = PROT_1 + ADRS_1 - 1;
  byte locid_1_high = locid_1 >> 8;
  byte locid_1_low = locid_1;
  EEPROM.put(reg_locid[0],locid_1_high);
  EEPROM.put(reg_locid[0] + 1, locid_1_low);

  uint16_t locid_2 = PROT_2 + ADRS_2 - 1;
  byte locid_2_high = locid_2 >> 8;
  byte locid_2_low = locid_2;
  EEPROM.put(reg_locid[1],locid_2_high);
  EEPROM.put(reg_locid[1] + 1, locid_2_low);
  
  uint16_t locid_3 = PROT_3 + ADRS_3 - 1;
  byte locid_3_high = locid_3 >> 8;
  byte locid_3_low = locid_3;
  EEPROM.put(reg_locid[0],locid_3_high);
  EEPROM.put(reg_locid[0] + 1, locid_3_low);
  
  uint16_t locid_4 = PROT_4 + ADRS_4 - 1;
  byte locid_4_high = locid_4 >> 8;
  byte locid_4_low = locid_4;
  EEPROM.put(reg_locid[0],locid_4_high);
  EEPROM.put(reg_locid[0] + 1, locid_4_low);
  
  #endif

  uid_mat[0] = UID >> 24;
  uid_mat[1] = UID >> 16;
  uid_mat[2] = UID >> 8;
  uid_mat[3] = UID;

  for(int i = 0; i < 4; i++){
    pinMode(acc_pin_red[i], OUTPUT);
    pinMode(acc_pin_grn[i], OUTPUT);
    pinMode(acc_pin_rm[i], INPUT);
  }

  hash = mcan.generateHash(UID);
  mcan.initMCAN();
  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);

  for(int i = 0; i < 4; i++){
    acc_state_is[i] = EEPROM.read(0xa0 + i);
    acc_state_set[i] = EEPROM.read(0xa0 + i); 
  }

  for(int i = 0; i < 4; i++){
    acc_locid[i] = (EEPROM.read(reg_locid[i]) << 8) | (EEPROM.read(reg_locid[i] + 1));
  }

  for(int i = 0; i < 4; i++){
    switchAcc(i, acc_state_is[i]);
  }
}

/*
 * Funktion zum schalten der Ausgänge
 */
void switchAcc(int acc_num, bool set_state){

  bool switchmode = EEPROM.read(0);
  bool feedback = EEPROM.read(1);
  uint16_t switchtime = (EEPROM.read(2) << 8) | EEPROM.read(3);
  
if(!switchmode){
  if(!set_state){
    digitalWrite(acc_pin_red[acc_num], HIGH);
    digitalWrite(9,0);
    delay(switchtime);
    digitalWrite(acc_pin_red[acc_num], LOW);
    digitalWrite(9,1);
    
  } else if (set_state){
    digitalWrite(acc_pin_grn[acc_num], HIGH);
    digitalWrite(9,0);
    delay(switchtime);
    digitalWrite(acc_pin_grn[acc_num], LOW);
    digitalWrite(9,1);
    
  }
  if(feedback == 0){
    digitalWrite(9,0);
    switchAccResponse(acc_num, set_state);
    acc_state_is[acc_num] = set_state;
    EEPROM.put(0xa0 + acc_num, acc_state_is[acc_num]);
    digitalWrite(9,1);
  }
}else if(switchmode == 1){
  if(!set_state){
    digitalWrite(acc_pin_grn[acc_num], LOW);
    digitalWrite(9,0);
    delay(20);
    digitalWrite(acc_pin_red[acc_num], HIGH);
    digitalWrite(9,1);
  } else if(set_state){
    digitalWrite(acc_pin_red[acc_num], LOW);
    digitalWrite(9,0);
    delay(20);
    digitalWrite(acc_pin_grn[acc_num], HIGH);
    digitalWrite(9,1);
  }
  switchAccResponse(acc_num, set_state);
  acc_state_is[acc_num] = set_state;
  EEPROM.put(0xa0 + acc_num, acc_state_is[acc_num]);
}
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

void accFrame(){  
  if((can_frame_in.cmd == SWITCH_ACC) && (can_frame_in.resp_bit == 0)){     //Abhandlung bei gültigem Weichenbefehl
    uint16_t locid = (can_frame_in.data[2] << 8) | can_frame_in.data[3];
    for(int i = 0; i < 4; i++){
      acc_locid[i] = (EEPROM.read(reg_locid[i]) << 8) | (EEPROM.read(reg_locid[i] + 1));
       if(locid == acc_locid[i]){                                              //Auf benutzte Adresse überprüfen
        acc_got_cmd[i] = true;
        acc_state_set[i] = can_frame_in.data[4];
        break;
      }
    }
  }
}

void pingFrame(){  
  if((can_frame_in.cmd == PING) && (can_frame_in.resp_bit == 0)){          //Auf Ping Request antworten
    pingResponse();
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
      for(int i = 0; i < 4; i++){
        if(can_frame_in.data[5] == adrs_channel[i]){
          EEPROM.put(reg_adrs[i], can_frame_in.data[6]);
          EEPROM.put(reg_adrs[i] + 1, can_frame_in.data[7]);
          statusResponse(can_frame_in.data[5]);
        }
        if(can_frame_in.data[5] == prot_channel[i]){
          EEPROM.put(reg_prot[i], can_frame_in.data[7]);
          statusResponse(can_frame_in.data[5]);
        }
        uint16_t prot;
        if(!EEPROM.read(reg_prot[i])){
          prot = DCC_ACC;
        }else{
          prot = MM_ACC;
        }
        uint16_t adrs = (EEPROM.read(reg_adrs[i]) << 8) | (EEPROM.read(reg_adrs[i] + 1));
        uint16_t locid = prot + adrs - 1;
        byte locid_high = locid >> 8;
        byte locid_low = locid;
        EEPROM.put(reg_locid[i], locid_high);
        EEPROM.put(reg_locid[i] + 1, locid_low);
      }
      if(can_frame_in.data[5] == 1){
        EEPROM.put(0, can_frame_in.data[7]);
        statusResponse(can_frame_in.data[5]);
      }
      if(can_frame_in.data[5] == 2){
        EEPROM.put(1, can_frame_in.data[7]);
        statusResponse(can_frame_in.data[5]);
      }
      if(can_frame_in.data[5] == 3){
        EEPROM.put(2, can_frame_in.data[6]);
        EEPROM.put(3, can_frame_in.data[7]);
        statusResponse(can_frame_in.data[5]);
      }
    }
  }
}

void pingResponse(){
  can_frame_out.cmd = PING;
  can_frame_out.hash = hash;
  can_frame_out.resp_bit = true;
  can_frame_out.dlc = 8;
  for(int i = 0; i < 4; i++){
    can_frame_out.data[i] = uid_mat[i];
  }
  can_frame_out.data[4] = VERS_HIGH;
  can_frame_out.data[5] = VERS_LOW;
  can_frame_out.data[6] = 0x00;
  can_frame_out.data[7] = 0x50;

  mcan.sendCanFrame(can_frame_out);
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

void sendConfig(int index){
  
  byte config_len[] = {4,4,4,4,5,5,5,5,5,5,5,5};
  byte config_frames[][10][8] = {{
      {0,CONFIG_NUM,0,0,0,0,0,BOARD_NUM},
      {'M','a','g','n','e','t',0,0},
      {'M',0xc3,0xa4,'C','A','N',' ','D'},
      {'e','c','o','d','e','r',0,0}
    },{
      {1,1,2,EEPROM.read(0),0,0,0,0},
      {'B','e','t','r','i','e','b','s'},
      {'a','r','t',0,'M','o','m','e'},
      {'n','t',0,'D','a','u','e','r'}
    },{
      {2,1,2,EEPROM.read(1),0,0,0,0},
      {'L','a','g','e','m','e','l','d'},
      {'u','n','g',0,'A','u','s',0},
      {'E','i','n',0,0,0,0,0}
    },{
      {3,2,0,0x14,0x03,0xe8,EEPROM.read(2),EEPROM.read(3)},
      {'S','c','h','a','l','t','z','e'},
      {'i','t',0,'2','0',0,'1','0'},
      {'0','0',0,'m','s',0,0,0}
    },{
      {4,1,2,EEPROM.read(0x30),0,0,0,0},
      {'P','r','o','t','o','k','o','l'},
      {'l',' ','A','u','s','g','a','n'},
      {'g',' ','1',0,'D','C','C',0},
      {'M','o','t','o','r','o','l','a'}
    },{
      {5,2,0,0x01,0x08,0x00,EEPROM.read(0x20),EEPROM.read(0x21)},
      {'A','d','r','e','s','s','e',' '},
      {'A','u','s','g','a','n','g',' '},
      {'1',0,'1',0,'2','0','4','8'},
      {0,0,0,0,0,0,0,0}
    },{
      {6,1,2,EEPROM.read(0x31),0,0,0,0},
      {'P','r','o','t','o','k','o','l'},
      {'l',' ','A','u','s','g','a','n'},
      {'g',' ','2',0,'D','C','C',0},
      {'M','o','t','o','r','o','l','a'}
    },{
      {7,2,0,0x01,0x08,0x00,EEPROM.read(0x22),EEPROM.read(0x23)},
      {'A','d','r','e','s','s','e',' '},
      {'A','u','s','g','a','n','g',' '},
      {'2',0,'1',0,'2','0','4','8'},
      {0,0,0,0,0,0,0,0}
    },{
      {8,1,2,EEPROM.read(0x32),0,0,0,0},
      {'P','r','o','t','o','k','o','l'},
      {'l',' ','A','u','s','g','a','n'},
      {'g',' ','3',0,'D','C','C',0},
      {'M','o','t','o','r','o','l','a'}
    },{
      {9,2,0,0x01,0x08,0x00,EEPROM.read(0x24),EEPROM.read(0x25)},
      {'A','d','r','e','s','s','e',' '},
      {'A','u','s','g','a','n','g',' '},
      {'3',0,'1',0,'2','0','4','8'},
      {0,0,0,0,0,0,0,0}
    },{
      {10,1,2,EEPROM.read(0x33),0,0,0,0},
      {'P','r','o','t','o','k','o','l'},
      {'l',' ','A','u','s','g','a','n'},
      {'g',' ','4',0,'D','C','C',0},
      {'M','o','t','o','r','o','l','a'}
    },{
      {11,2,0,0x01,0x08,0x00,EEPROM.read(0x26),EEPROM.read(0x27)},
      {'A','d','r','e','s','s','e',' '},
      {'A','u','s','g','a','n','g',' '},
      {'4',0,'1',0,'2','0','4','8'},
      {0,0,0,0,0,0,0,0}
    }};
  
  for(int i = 0; i < config_len[index]; i++){
    configDataFrame(config_frames[index][i], i);
  }
  configTerminator(index, config_len[index]);
}

void configDataFrame(uint8_t config_data[8], int framecount){  
  can_frame_out.cmd = CONFIG;
  can_frame_out.hash = 0x300 | framecount + 1;
  can_frame_out.resp_bit = true;
  can_frame_out.dlc = 8;
  for(int i = 0; i < 8; i++){
    can_frame_out.data[i] = config_data[i];
  }

  mcan.sendCanFrame(can_frame_out);

  delay(5);
  
}

void configTerminator(int channel, int framecount){
  can_frame_out.cmd = CONFIG;
  can_frame_out.hash = hash;
  can_frame_out.resp_bit = true;
  can_frame_out.dlc = 6;
  can_frame_out.data[0] = UID >> 24;
  can_frame_out.data[1] = UID >> 16;
  can_frame_out.data[2] = UID >> 8;
  can_frame_out.data[3] = UID;
  can_frame_out.data[4] = channel;
  can_frame_out.data[5] = framecount;
  can_frame_out.data[6] = 0;
  can_frame_out.data[7] = 0;

  mcan.sendCanFrame(can_frame_out);
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
  statusFrame();
}

void loop() {
  
  bool switchmode = EEPROM.read(0);
  bool feedback = EEPROM.read(1);
  
  for(int i = 0; i < 4; i++){
    if((feedback == 1) && (switchmode == 0)){
      if(acc_state_is[i] != digitalRead(acc_pin_rm[i])){                        //Änderungen der Weichenlage überprüfen
        acc_state_is[i] = digitalRead(acc_pin_rm[i]);
        if(acc_state_is[i] != acc_state_set[i]){
          switchAccResponse(i, acc_state_is[i]);
          acc_state_set[i] = acc_state_is[i];
          EEPROM.put(0xa0 + i, acc_state_is[i]);
        }
      }
    }
    if(acc_got_cmd[i]){                                                       //Nach eingegangenem Weichenbefehl schalten
      switchAcc(i, acc_state_set[i]);
      acc_got_cmd[i] = false;
    }
  }
  
  if(config_poll){
    config_poll = false;
    sendConfig(config_index);
  }
}


