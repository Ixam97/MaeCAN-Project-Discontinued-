# MäCAN-Project

![MäCAN](images/mäcan-logo.png)

by [Maximilian Goldschmidt](mailto:maxigoldschmidt@gmail.com)

Hier entsteht ein System zur digitalen Steuerung einer Modellbahn, das auf dem CAN-Bus-Protokoll von Märklin basiert. Ziel ist es, eine preisgünstige und frei zugängliche Lösung zu schaffen.

# MäCAN-Bibliothek

Die [MCAN-Bibliothek](MCAN/) dient dazu, eine einfache Möglichkeit zu haben, auf den Märklin CAN-Bus zuzugreifen. Dabei ist es nicht notwendig irgendwelche Berechnungen durchzuführen. Die CAN-Frames werden in nahezu selbsterklärender Weise dageboten

##Dokumentation MCAN-Bibliothek

#### Konstanten

###### CAN-Befehle
````
 SYS_CMD	0x00 	//Systembefehle (abhängig von Unterbefehlen!)
 	 SYS_STOP 	0x00 	//System - Stopp
 	 SYS_GO		0x01	//System - Go
 	 SYS_HALT	0x02	//System - Halt
 SWITCH_ACC 	0x0b	//Magnetartikel Schalten
 S88_EVENT	0x11	//Rückmelde-Event
 PING 		0x18	//CAN-Teilnehmer anpingen
````

###### Protokolle
````
 MM_ACC 		0x3000	//Magnetartikel Motorola
 DCC_ACC 	0x3800	//Magbetartikel NRMA_DCC
 MM_TRACK 	0x0000	//Gleissignal Motorola
 DCC_TRACK 	0xC000	//Gleissignal NRMA_DCC
````

#### MCANMSG

Hierbei handelt es sich um einen Struct, der alle relevanten Informationen über einen CAN-Frame, wie er von Märklin dokumentiert ist, beinhaltet.
````
````

####generateHash(uid)

Diese funktion kann dazu genutzt, um aus der 32-bit UID der CAN-Komponente gemäß der Dokumentation von Märklin den Hash zu generieren.

Beispiel:
```
#include <MCAN.h>

uint32_t uid = 0x12345678;
uint16_t hash;

MCAN mcan;

void setup() {
  hash = mcan.generateHash(uid);
}
````

Für den Hash ergäbe sich dann 
````
hash = 0x474C
````
Weitere Informationen zum Hash und dessen berechnung sind [Hier](http://medienpdb.maerklin.de/digital22008/files/cs2CAN-Protokoll-2_0.pdf) unter dem Punkt "1.2.4 Hash" zu finden.
