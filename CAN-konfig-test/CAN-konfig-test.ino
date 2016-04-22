/*
 * MäCAN-Konfig-Test
 * 
 *  Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *  Do with this whatever you want, but keep thes Header and tell
 *  the others what you changed!
 */

#include <MCAN.h>
#include <EEPROM.h>
/*
 * UID festlegen:
 */
//#define UID 0x53383898
#define UID 0x12345678

#define VERSION 

byte uid_mat[4] = {0x12,0x34,0x56,0x78};

/*
 * 3D-Matrix mit sämtlichen informationen für die Konfigurationsabfrage:
 */

#define CONFIG_NUM 0x08
#define BOARD_NUM 32

uint16_t hash;

int config_index = 0;
bool config_poll = false;

MCAN mcan;
MCANMSG can_frame_out;
MCANMSG can_frame_in;


void setup() {

  hash = mcan.generateHash(UID);
  mcan.initMCAN(false);
  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);

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
  can_frame_out.data[5] = 0x02;
  can_frame_out.data[6] = 0x00;
  can_frame_out.data[7] = 0x50;

  mcan.sendCanFrame(can_frame_out);
}
void isPingFrame(){  
  if((can_frame_in.cmd == PING) && (can_frame_in.resp_bit == 0)){          //Auf Ping Request antworten
    pingResponse();
  }
}
void isConfigFrame(){
  if((can_frame_in.cmd == CONFIG) && (can_frame_in.resp_bit == 0)){
    if((uid_mat[0] == can_frame_in.data[0])&&(uid_mat[1] == can_frame_in.data[1])&&(uid_mat[2] == can_frame_in.data[2])&&(uid_mat[3] == can_frame_in.data[3])){
      config_poll = true;
      config_index = can_frame_in.data[4];
    }
  }
}
void isStatusFrame(){
  if((can_frame_in.cmd == SYS_CMD) && (can_frame_in.resp_bit == 0) && (can_frame_in.data[4] == SYS_STAT)){
    if((uid_mat[0] == can_frame_in.data[0])&&(uid_mat[1] == can_frame_in.data[1])&&(uid_mat[2] == can_frame_in.data[2])&&(uid_mat[3] == can_frame_in.data[3])){
      //Neue Einstellungen Speichen!
      pingResponse();
    }
  }
}

/*
 * Ausführen, wenn eine Nachricht verfügbar ist.
 * Nachricht wird geladen und anhängig vom CAN-Befehl verarbeitet.
 */
void interruptFn(){
  can_frame_in = mcan.getCanFrame();
  isPingFrame();
  isConfigFrame();
  isStatusFrame();
}

void loop() {
  if(config_poll){
    config_poll = false;
    sendConfig(config_index);
  }
}

/*
 * Logik zum versenden von Konfiguartionsdaten:
 */

void sendConfig(int index){
  
  byte config_len[] = {4,5,5,5,5,5,5,5,5};
  byte config_frames[][10][8] = {{
      {0,CONFIG_NUM,0,0,0,0,0,BOARD_NUM},
      {'M','a','g','n','e','t',0,0},
      {'M',0xc3,0xa4,'C','A','N',' ','D'},
      {'e','c','o','d','e','r',0,0}
    },{
      {1,1,2,0,0,0,0,0},
      {'P','r','o','t','o','k','o','l'},
      {'l',' ','A','u','s','g','a','n'},
      {'g',' ','1',0,'D','C','C',0},
      {'M','o','t','o','r','o','l','a'}
    },{
      {2,2,0,0x01,0x08,0x00,0,1},
      {'A','d','r','e','s','s','e',' '},
      {'A','u','s','g','a','n','g',' '},
      {'1',0,'1',0,'2','0','4','8'},
      {0,0,0,0,0,0,0,0}
    },{
      {3,1,2,0,0,0,0,0},
      {'P','r','o','t','o','k','o','l'},
      {'l',' ','A','u','s','g','a','n'},
      {'g',' ','2',0,'D','C','C',0},
      {'M','o','t','o','r','o','l','a'}
    },{
      {4,2,0,0x01,0x08,0x00,0,2},
      {'A','d','r','e','s','s','e',' '},
      {'A','u','s','g','a','n','g',' '},
      {'2',0,'1',0,'2','0','4','8'},
      {0,0,0,0,0,0,0,0}
    },{
      {5,1,2,0,0,0,0,0},
      {'P','r','o','t','o','k','o','l'},
      {'l',' ','A','u','s','g','a','n'},
      {'g',' ','3',0,'D','C','C',0},
      {'M','o','t','o','r','o','l','a'}
    },{
      {6,2,0,0x01,0x08,0x00,0,3},
      {'A','d','r','e','s','s','e',' '},
      {'A','u','s','g','a','n','g',' '},
      {'3',0,'1',0,'2','0','4','8'},
      {0,0,0,0,0,0,0,0}
    },{
      {7,1,2,0,0,0,0,0},
      {'P','r','o','t','o','k','o','l'},
      {'l',' ','A','u','s','g','a','n'},
      {'g',' ','4',0,'D','C','C',0},
      {'M','o','t','o','r','o','l','a'}
    },{
      {8,2,0,0x01,0x08,0x00,0,4},
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

void configTerminator(int chanel, int framecount){
  can_frame_out.cmd = CONFIG;
  can_frame_out.hash = hash;
  can_frame_out.resp_bit = true;
  can_frame_out.dlc = 6;
  can_frame_out.data[0] = UID >> 24;
  can_frame_out.data[1] = UID >> 16;
  can_frame_out.data[2] = UID >> 8;
  can_frame_out.data[3] = UID;
  can_frame_out.data[4] = chanel;
  can_frame_out.data[5] = framecount;
  can_frame_out.data[6] = 0;
  can_frame_out.data[7] = 0;

  mcan.sendCanFrame(can_frame_out);
}

