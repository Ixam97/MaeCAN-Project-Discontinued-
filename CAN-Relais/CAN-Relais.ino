/*
 * MäCAN-Relais AddOn, Software-Version 0.2
 *
 *  Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *  Modified by Jochen Kielkopf.
 *  Do with this whatever you want, but keep thes Header and tell
 *  the others what you changed!
 *
 *  Last edited: 2016-11-12
 */

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <MCAN.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>

/******************************************************************************
 * EEPROM-Register-Übersicht
 * # 0 - 20 -> CONFIG:
 * ------------------------------------------------------------------------
 *       0 -> Initial Config ( 0 = unkonfiguriert, 1 = konfiguriert )
 *  1 -  4 -> UID
 *       5 -> SWITCHMODE
 *  6 -  7 -> SWITCHTIME
 *       8 -> FEEDBACK
 *       9 -> PROTOKOLL
 * 10 - 19 -> reserved / free
 *      20 -> Anzahl ACCs
 * ========================================================================
 * #21 - XX -> Adressbereich, Pin, Modul (Max 167 ACCs):
 * ------------------------------------------------------------------------
 *  2 byte -> LocalID       // wird aktuell genutzt
 *  2 byte -> Adresse       // wird nicht aktuell genutzt
 *  1 byte -> Status        // wird nicht aktuell genutzt
 *  1 byte -> Modul ( 0 = Decoder, 1 = AddOn 1, 2 = AddOn 2 ) // wird nicht aktuell genutzt
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 * Formeln zu Berechnung der Speicherregister
 * ------------------------------------------------------------------------
 * LocalID = (20+(6*i)-5) (20+(6*i)-4)
 * Adresse = (20+(6*i)-3) (20+(6*i)-2)
 * Status  = (20+(6*i)-1)
 * Modul   = (20+(6*i))
 * ========================================================================
 ******************************************************************************/

/******************************************************************************
 * Allgemeine Setup Daten:
 ******************************************************************************/
#define VERS_HIGH 0       //Versionsnummer vor dem Punkt
#define VERS_LOW  2       //Versionsnummer nach dem Punkt
#define BOARD_NUM 1       //Identifikationsnummer des Boards (Anzeige in der CS2)
#define UID 0x10052021    //CAN-UID
// UID = 0x1 device.type VERS_HIGH VERS_LOW BOARD_NUM
//#define MCAN_MAGNET  0x0050
//#define MCAN_SERVO  0x0051
//#define MCAN_RELAIS 0x0052
//#define MCAN_STELLPULT 0x0053
//#define MCAN_S88_GBS 0x0054

const uint8_t USE_ONBOARD = 1;       // On-Board Ausgänge benutzen: 0 = Nein; 1 = Ja
const uint8_t ANZ_ADDONS  = 8;       // Anzahl AddOn Platinen (max 8)
const uint8_t ANZ_ACC_PER_ADDON = 8; // Anzahl an Ausgängen pro AddOn Platine: Default = 8
                                      // => Relais = 8
                                      // => Weichen = 8
                                      // => Weichen mit Lagerückmeldung = 5 (aktuelle AddOn-HW unterstützt dies noch nicht.)
                                      // ==>> evtl. ab AddOn-HW Rev. c

/******************************************************************************
* Allgemeine Konstanten:
******************************************************************************/
const int REG_SWITCHMODE = 5;
const int REG_SWITCHTIME = 6;
const int REG_PROT = 9;         // Protokoll - Global für alle ACC gleich.
const String string1 = "Adresse Ausgang ";
const String string2 = "_1_2048";
String string3;

int start_adrs_channel = 3;

typedef struct {
  uint16_t locID;       // LocalID
  uint8_t Modul;        // 0 = On-Board; 1 = Modul1; 2 = Modul2, ...
  uint8_t pin_grn;      // PIN Grün
  uint8_t pin_red;      // PIN Rot
  int reg_locid;        // EEPROM-Register der Local-IDs
  bool state_is;        // ...
  bool state_set;       // ...
  int adrs_channel;     // Konfigkanäle für die Adressen
} acc_Magnet;

acc_Magnet acc_articles[ (4 * USE_ONBOARD) + (ANZ_ADDONS * ANZ_ACC_PER_ADDON) ]; // = 4 * USE_ONBOARD + ANZ_ACC_PER_ADDON * ANZ_ADDONS

Adafruit_MCP23017 AddOn[ANZ_ADDONS];   // Create AddOn

uint8_t NUM_ACCs;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 1000;
bool state_LED = false;

/******************************************************************************
 * Variablen der Magnetartikel:
 ******************************************************************************/
uint8_t CONFIG_NUM;     //Anzahl der Konfigurationspunkte
uint16_t hash;
bool config_poll = false;
byte uid_mat[4];
uint8_t  config_index =   0;
uint16_t switchtime   = 350;
bool switchmode = 0;
uint16_t prot;
uint16_t prot_old;

/******************************************************************************
 * Benötigtes:
 ******************************************************************************/
MCAN mcan;
MCANMSG can_frame_out;
MCANMSG can_frame_in;

CanDevice device;

/******************************************************************************
 * Setup
 ******************************************************************************/
void setup() {
  uid_mat[0] = UID >> 24;
  uid_mat[1] = UID >> 16;
  uid_mat[2] = UID >> 8;
  uid_mat[3] = UID;

  if( (EEPROM.read(1)==uid_mat[0]) && (EEPROM.read(2)==uid_mat[1]) && (EEPROM.read(3)==uid_mat[2]) && (EEPROM.read(4)==uid_mat[3]) ){
  } else {
    EEPROM.put(0, 0);
    EEPROM.put(1, uid_mat[0]);
    EEPROM.put(2, uid_mat[1]);
    EEPROM.put(3, uid_mat[2]);
    EEPROM.put(4, uid_mat[3]);
    EEPROM.put(REG_SWITCHMODE, switchmode);
    EEPROM.put(REG_SWITCHTIME, (switchtime >> 8));
    EEPROM.put(REG_SWITCHTIME+1, switchtime);
    EEPROM.put(REG_PROT, 1);    // 0 = DCC, 1=MM
  }

  pinMode(9, OUTPUT);
  digitalWrite(9,state_LED);
  hash = mcan.generateHash(UID);

  //Geräteinformationen:
  device.versHigh = VERS_HIGH;
  device.versLow = VERS_LOW;
  device.hash = hash;
  device.uid = UID;
  device.artNum = "Relais";
  device.name = "MäCAN Decoder";
  device.boardNum = BOARD_NUM;
  device.type = MCAN_RELAIS;

  switchmode = EEPROM.read(REG_SWITCHMODE);
  switchtime = (EEPROM.read(REG_SWITCHTIME) << 8) | EEPROM.read(REG_SWITCHTIME+1);

  if(!EEPROM.read(REG_PROT)){
    prot = DCC_ACC;
  }else{
    prot = MM_ACC;
  }
  prot_old = prot;

  NUM_ACCs = ANZ_ACC_PER_ADDON * ANZ_ADDONS;
  if(USE_ONBOARD == 1){
    NUM_ACCs = NUM_ACCs + 4;
  }
  CONFIG_NUM = start_adrs_channel + NUM_ACCs - 1;

  setup_acc();

  mcan.initMCAN();
  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
  state_LED = 1;
  digitalWrite(9,state_LED);

}

/*
 * Setup Funktion zum Einrichten der ACCs
 */
void setup_acc() {
  // setup mainboard pins
  int num = 0;
  if(USE_ONBOARD == 1){
    acc_articles[num].Modul = 0;
    acc_articles[num].pin_red = 1;
    acc_articles[num].pin_grn = 0;
    acc_articles[num].reg_locid = (20+(6*(num+1))-5);
    acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
    acc_articles[num].adrs_channel = num + start_adrs_channel;
    num++;

    acc_articles[num].Modul = 0;
    acc_articles[num].pin_red = 4;
    acc_articles[num].pin_grn = 3;
    acc_articles[num].reg_locid = (20+(6*(num+1))-5);
    acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
    acc_articles[num].adrs_channel = num + start_adrs_channel;
    num++;

    acc_articles[num].Modul = 0;
    acc_articles[num].pin_red = 6;
    acc_articles[num].pin_grn = 5;
    acc_articles[num].reg_locid = (20+(6*(num+1))-5);
    acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
    acc_articles[num].adrs_channel = num + start_adrs_channel;
    num++;

    acc_articles[num].Modul = 0;
    acc_articles[num].pin_red = 8;
    acc_articles[num].pin_grn = 7;
    acc_articles[num].reg_locid = (20+(6*(num+1))-5);
    acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
    acc_articles[num].adrs_channel = num + start_adrs_channel;
    num++;
    for(int i = 0; i < 4; i++){
      pinMode(acc_articles[i].pin_red, OUTPUT);
      pinMode(acc_articles[i].pin_grn, OUTPUT);
    }

  }

  if(ANZ_ADDONS > 0){
    for(int m = 0; m < ANZ_ADDONS; m++){
      AddOn[m].begin(m);
      int pin = 0;
      for(int i = 0; i < ANZ_ACC_PER_ADDON; i++){
        acc_articles[num].Modul = m + 1;
        acc_articles[num].pin_grn = pin;
        pin++;
        acc_articles[num].pin_red = pin;
        pin++;

        AddOn[m].pinMode(acc_articles[num].pin_grn, OUTPUT);
        AddOn[m].pinMode(acc_articles[num].pin_red, OUTPUT);

        acc_articles[num].reg_locid = (20+(6*(num+1))-5);
        acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
        acc_articles[num].adrs_channel = num + start_adrs_channel;
        num++;
      }
    }
  }

  if (EEPROM.read(0) == 0) {
    EEPROM.put(0, 1);
    for(int i = 0; i < NUM_ACCs; i++) {
      uint16_t adrsss = i + 97;
      acc_articles[i].locID = mcan.generateLocId(prot, adrsss );
      byte locid_high = acc_articles[i].locID >> 8;
      byte locid_low = acc_articles[i].locID;
      EEPROM.put(acc_articles[i].reg_locid, locid_high);
      EEPROM.put(acc_articles[i].reg_locid + 1, locid_low);
    }
  }
  signal_setup_successfull();

}

/*
 * Change LocalID when protocol has changed
 */
void change_prot(){
  for(int i = 0; i < NUM_ACCs; i++) {
    acc_articles[i].locID = (EEPROM.read( acc_articles[i].reg_locid ) << 8) | (EEPROM.read( acc_articles[i].reg_locid + 1 ));
    uint16_t adrsss = mcan.getadrs(prot_old, acc_articles[i].locID);
    acc_articles[i].locID = mcan.generateLocId(prot, adrsss );
    byte locid_high = acc_articles[i].locID >> 8;
    byte locid_low = acc_articles[i].locID;
    EEPROM.put(acc_articles[i].reg_locid, locid_high);
    EEPROM.put(acc_articles[i].reg_locid + 1, locid_low);
  }

}

/*
 *
 */
void signal_setup_successfull(){
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
  state_LED = !state_LED;
  digitalWrite(9,state_LED);
  delay(100);
}

/*
 * Funktion zum schalten der Ausgänge
 */
void switchAcc(int acc_num, bool set_state){
  if(!switchmode){                //0 = Moment; 1 = Dauer
    if(!set_state){                   // rot
      if(acc_articles[acc_num].Modul == 0){
        digitalWrite(acc_articles[acc_num].pin_red, HIGH);
        digitalWrite(9,0);
        delay(switchtime);
        digitalWrite(acc_articles[acc_num].pin_red, LOW);
        digitalWrite(9,1);
      } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        int pin = acc_articles[acc_num].pin_red;
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, HIGH);
        digitalWrite(9,0);
        delay(switchtime);
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, LOW);
        digitalWrite(9,1);
      }
    } else if (set_state){            // grün
      if(acc_articles[acc_num].Modul == 0){
        digitalWrite(acc_articles[acc_num].pin_grn, HIGH);
        digitalWrite(9,0);
        delay(switchtime);
        digitalWrite(acc_articles[acc_num].pin_grn, LOW);
        digitalWrite(9,1);
      } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        int pin = acc_articles[acc_num].pin_grn;
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, HIGH);
        digitalWrite(9,0);
        delay(switchtime);
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, LOW);
        digitalWrite(9,1);
      }
    }

  }else if(switchmode){          //0 = Moment; 1 = Dauer
    if(!set_state){                   // rot
      if(acc_articles[acc_num].Modul == 0){
        digitalWrite(acc_articles[acc_num].pin_grn, LOW);
        digitalWrite(9,0);
        delay(20);
        digitalWrite(acc_articles[acc_num].pin_red, HIGH);
        digitalWrite(9,1);
      } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        int pin = acc_articles[acc_num].pin_grn;
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, LOW);
        digitalWrite(9,0);
        delay(20);
        pin = acc_articles[acc_num].pin_red;
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, HIGH);
        digitalWrite(9,1);
      }
    } else if(set_state){             // grün
      if(acc_articles[acc_num].Modul == 0){
        digitalWrite(acc_articles[acc_num].pin_red, LOW);
        digitalWrite(9,0);
        delay(20);
        digitalWrite(acc_articles[acc_num].pin_grn, HIGH);
        digitalWrite(9,1);
      } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        int pin = acc_articles[acc_num].pin_red;
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, LOW);
        digitalWrite(9,0);
        delay(20);
        pin = acc_articles[acc_num].pin_grn;
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, HIGH);
        digitalWrite(9,1);
      }
    }
  }
  digitalWrite(9,0);
  mcan.sendAccessoryFrame(device, (EEPROM.read( acc_articles[acc_num].reg_locid ) << 8) | (EEPROM.read( acc_articles[acc_num].reg_locid + 1 )), acc_articles[acc_num].state_is, true);
  acc_articles[acc_num].state_is = set_state;
  digitalWrite(9,1);

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
  //previousMillis = currentMillis;

}

/*
 * Prüfen auf Schaltbefehl.
 */
void accFrame(){
  if((can_frame_in.cmd == SWITCH_ACC) && (can_frame_in.resp_bit == 0)){     //Abhandlung bei gültigem Weichenbefehl
    uint16_t locid = (can_frame_in.data[2] << 8) | can_frame_in.data[3];
    for(int i = 0; i < NUM_ACCs; i++){
      if(locid == acc_articles[i].locID){                                              //Auf benutzte Adresse überprüfen
        acc_articles[i].state_set = can_frame_in.data[4];
        break;
      }
    }
  }

}

/*
 * Auf Ping Request antworten.
 */
void pingFrame(){
  if((can_frame_in.cmd == PING) && (can_frame_in.resp_bit == 0)){          //Auf Ping Request antworten
    mcan.sendPingFrame(device, true);
  }

}


/*
 *
 */
void configFrame(){
  if((can_frame_in.cmd == CONFIG) && (can_frame_in.resp_bit == 0)){
    if((uid_mat[0] == can_frame_in.data[0])&&(uid_mat[1] == can_frame_in.data[1])&&(uid_mat[2] == can_frame_in.data[2])&&(uid_mat[3] == can_frame_in.data[3])){
      config_poll = true;
      config_index = can_frame_in.data[4];
    }
  }

}

/*
 *
 */
void statusFrame(){
  if((can_frame_in.cmd == SYS_CMD) && (can_frame_in.resp_bit == 0) && (can_frame_in.data[4] == SYS_STAT)){
    if((uid_mat[0] == can_frame_in.data[0])&&(uid_mat[1] == can_frame_in.data[1])&&(uid_mat[2] == can_frame_in.data[2])&&(uid_mat[3] == can_frame_in.data[3])){
      if(can_frame_in.data[5] == 1){                          // Protokoll schreiben ( MM oder DCC)
        prot_old = prot;
        if(!can_frame_in.data[7]){
          prot = DCC_ACC;
        }else{
          prot = MM_ACC;
        }
        if(prot != prot_old){
          EEPROM.put(REG_PROT, can_frame_in.data[7]);
          change_prot();
        }
        mcan.statusResponse(device, can_frame_in.data[5]);

      } else if(can_frame_in.data[5] == 2){                   // SWITCHTIME
        EEPROM.put(REG_SWITCHTIME, can_frame_in.data[6]);
        EEPROM.put(REG_SWITCHTIME+1, can_frame_in.data[7]);
        switchtime = (can_frame_in.data[6] << 8) | can_frame_in.data[6];
        mcan.statusResponse(device, can_frame_in.data[5]);

      } else if(can_frame_in.data[5] >= start_adrs_channel){
        if(can_frame_in.data[5] == acc_articles[can_frame_in.data[5]-start_adrs_channel].adrs_channel){
          acc_articles[can_frame_in.data[5]-start_adrs_channel].locID = mcan.generateLocId(prot, (can_frame_in.data[6] << 8) | can_frame_in.data[7] );
          byte locid_high = acc_articles[can_frame_in.data[5]-start_adrs_channel].locID >> 8;
          byte locid_low = acc_articles[can_frame_in.data[5]-start_adrs_channel].locID;
          EEPROM.put(acc_articles[can_frame_in.data[5]-start_adrs_channel].reg_locid, locid_high);
          EEPROM.put(acc_articles[can_frame_in.data[5]-start_adrs_channel].reg_locid + 1, locid_low);
          mcan.statusResponse(device, can_frame_in.data[5]);

        }
      }
    }
  }

}

/*
 * Main loop
 */
void loop() {
  /*
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    detachInterrupt(digitalPinToInterrupt(2));
    delay(50);
    attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
  }
  */

  currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    state_LED = !state_LED;
    digitalWrite(9,state_LED);
  }


  for(int i = 0; i < NUM_ACCs; i++){
    if(acc_articles[i].state_is != acc_articles[i].state_set){
      switchAcc(i, acc_articles[i].state_set);
    }
  }

  if(config_poll){
    if(config_index == 0) mcan.sendDeviceInfo(device, CONFIG_NUM);
    if(config_index == 1) mcan.sendConfigInfoDropdown(device, 1, 2, EEPROM.read(REG_PROT), "Protokoll_DCC_MM");
    if(config_index == 2) mcan.sendConfigInfoSlider(device, 2, 350, 2000, switchtime, "Schaltzeit_350_2000");
    if(config_index >= 3){
      string3 = string1 + ( config_index - start_adrs_channel + 1 ) + string2;
      uint16_t adrs = mcan.getadrs(prot, acc_articles[ config_index - start_adrs_channel ].locID);
      mcan.sendConfigInfoSlider(device, config_index, 1, 2048, adrs, string3);
    }
    config_poll = false;
  }

}
