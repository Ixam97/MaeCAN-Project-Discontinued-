#include <MCAN.h>
#include <EEPROM.h>
/*
 * Protokoll festlegen:
 * Mögliche Protokolle:
 * Magnetartikel Motorola: MM_ACC
 * Magnetartikel NRMA-DCC: DCC_ACC
 * Gleissignal Motorola: MM_TRACK
 * Gleissignal NRMA-DCC: DCC_TRACK
 */
#define PROTOCOL_1 DCC_ACC
#define PROTOCOL_2 DCC_ACC
#define PROTOCOL_3 DCC_ACC
#define PROTOCOL_4 DCC_ACC

/*
 * Adressen der Ausgänge festlegen:
 */
#define ADRS_1 1
#define ADRS_2 2
#define ADRS_3 3
#define ADRS_4 4

/*
 * UID festlegen:
 */
#define UID 0x12345678

/*
 * Variablen der Magnetartikel:
 */
uint16_t acc_adrs[4] = {PROTOCOL_1 + ADRS_1 - 1,PROTOCOL_2 + ADRS_2 - 1,PROTOCOL_3 + ADRS_3 - 1,PROTOCOL_4 + ADRS_4 - 1};
bool acc_got_cmd[4];
bool acc_state_is[4];
bool acc_state_set[4];
bool acc_state_rm[4];
static int acc_pin_grn[4] = {0,3,5,7};
static int acc_pin_red[4] = {1,4,6,8};
static int acc_pin_rm[4] = {A3,A2,A1,A0};
uint16_t hash;

MCAN mcan;
MCANMSG can_frame_out;
MCANMSG can_frame_in;


void setup() {

  for(int i = 0; i < 4; i++){
    pinMode(acc_pin_red[i], OUTPUT);
    pinMode(acc_pin_grn[i], OUTPUT);
    pinMode(acc_pin_rm[i], INPUT);
  }

  mcan.generateHash(UID);
  mcan.initMCAN(false);
  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);

  for(int i = 0; i < 4; i++){
    acc_state_is[i] = EEPROM.read(i);
    //acc_state_set[i] = EEPROM.read(i); 
  }
}

/*
 * Funktion zum schalten der Ausgänge
 * Schalteit: 20ms
 * Jeder Ausgang wird zwei mal HIGH für doppelte Redundanz.
 */
void switchAcc(int acc_num, bool set_state){
  if(!set_state){
    digitalWrite(acc_pin_red[acc_num], HIGH);
    digitalWrite(9,0);
    delay(20);
    digitalWrite(acc_pin_red[acc_num], LOW);
    digitalWrite(9,1);
    delay(20);
    digitalWrite(acc_pin_red[acc_num], HIGH);
    digitalWrite(9,0);
    delay(20);
    digitalWrite(acc_pin_red[acc_num], LOW);
    digitalWrite(9,1);
    
  } else if (set_state){
    digitalWrite(acc_pin_grn[acc_num], HIGH);
    digitalWrite(9,0);
    delay(20);
    digitalWrite(acc_pin_grn[acc_num], LOW);
    digitalWrite(9,1);
    delay(20);
    digitalWrite(acc_pin_grn[acc_num], HIGH);
    digitalWrite(9,0);
    delay(20);
    digitalWrite(acc_pin_grn[acc_num], LOW);
    digitalWrite(9,1);
    
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
  can_frame_out.data[2] = acc_adrs[acc_num] >> 8;
  can_frame_out.data[3] = acc_adrs[acc_num];
  can_frame_out.data[4] = set_state;            /* Meldung der Lage für Märklin-Geräte.*/
  can_frame_out.data[5] = 0;

  mcan.sendCanFrame(can_frame_out);
  
  can_frame_out.data[4] = 0xfe - set_state;     /* Meldung für CdB-Module und Rocrail Feldereignisse. */
  
  mcan.sendCanFrame(can_frame_out);
}

/*
 * Antwort auf einen Ping Request.
 */
void pingResponse(){
  can_frame_out.cmd = PING;
  can_frame_out.hash = hash;
  can_frame_out.resp_bit = true;
  can_frame_out.dlc = 8;
  can_frame_out.data[0] = UID >> 24;
  can_frame_out.data[1] = UID >> 16;
  can_frame_out.data[2] = UID >> 8;
  can_frame_out.data[3] = UID;
  can_frame_out.data[4] = 0x00;
  can_frame_out.data[5] = 0x01;
  can_frame_out.data[6] = 0x00;
  can_frame_out.data[7] = 0x40;

  mcan.sendCanFrame(can_frame_out);
}

void isAccFrame(){  
  if((can_frame_in.cmd == SWITCH_ACC) && (can_frame_in.resp_bit == 0)){     //Abhandlung bei gültigem Weichenbefehl
    uint16_t adrs = (can_frame_in.data[2] << 8) | can_frame_in.data[3];
    for(int i = 0; i < 4; i++){
       if(adrs == acc_adrs[i]){                                              //Auf benutzte Adresse überprüfen
        //switchAcc(i, can_frame_in.data[4]);
        acc_got_cmd[i] = true;
        acc_state_set[i] = can_frame_in.data[4];
        switchAccResponse(i, can_frame_in.data[4]);
        break;
      }
    }
  }
}

void isPingFrame(){  
  if((can_frame_in.cmd == PING) && (can_frame_in.resp_bit == 0)){          //Auf Ping Request antworten
    pingResponse();
  }
}
/*
 * Ausführen, wenn eine Nachricht verfügbar ist.
 * Nachricht wird geladen und anhängig vom CAN-Befehl verarbeitet.
 */
void interruptFn(){
  can_frame_in = mcan.getCanFrame();
  isAccFrame();
  isPingFrame();
}

void loop() {
  for(int i = 0; i < 4; i++){
    if(acc_state_is[i] != digitalRead(acc_pin_rm[i])){                        //Änderungen der Weichenlage überprüfen
      acc_state_is[i] = digitalRead(acc_pin_rm[i]);
      if(acc_state_is[i] != acc_state_set[i]){
         switchAccResponse(i, acc_state_is[i]);
        acc_state_set[i] = acc_state_is[i];
      }
    }
    if(acc_got_cmd[i]){                                                       //Nach eingegangenem Weichenbefehl schalten
      switchAcc(i, acc_state_set[i]);
      acc_got_cmd[i] = false;
    }
  }
}

/*
 * Ab hier experimenteller Code, um Parameter über den CAN-Bus zu programmieren:
 */

void isProgFrame(){
  if((can_frame_in.cmd == PROG) && (can_frame_in.resp_bit == 0)){
    uint32_t target_uid = ((can_frame_in.data[0] << 24) | (can_frame_in.data[2] << 16) | (can_frame_in.data[3] << 8) | (can_frame_in.data[4]));
    if(target_uid == UID){
      for(int i = 0; i < 4; i++){
        if(can_frame_in.data[4] == PROG_ADRS){
          break;
        }
        if(can_frame_in.data[4] == PROG_TIME){
          break;
        }
        if(can_frame_in.data[4] == PROG_ANGLE){
          break;
        }
      }
    }
  }
}

