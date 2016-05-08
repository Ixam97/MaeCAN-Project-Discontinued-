# MäCAN UDP-Bridge

Bei diesem Modul handelt es sich um eine kleine Platine, dessen primärer Zweck die kabellose Anbindung einer Mobile Station 2 ist. 
Es werden CAN-Frames auf UDP übersetzt und umgekehrt. Zwei dieser Module können direkt per WiFi miteinander verbunden werden und stellen
so eine kabellose CAN-Brücke her. Mehrere Clients können sich mit einem Basismodul verbinden, welches für diesen Zweck ein eigenes
WiFi-Netzwerk bereitstellt. 

Es ist geplant, dass dieses oder ein ähnliches Modul später ein Web-Interface bereitstellt, über das weitere MäCAN-Module und manche
Märklin-Produkte (z.B. Link S88) konfiguriert werden können.

## Dokumentation

#### Inhalt

### Einrichten der Arduino-IDE

Um veränderungen an der Software vorzunehmen ist eine spezielle einrichtung der Arduino-IDE erforderlich. Als Controller wird der ESP8266
verwendet, welcher bereits ein eingebautes WiFi-Modul verfügt. 

Um die Arduino-IDE kompatibel zu machen muss wie folgt vorgegangen werden:
* Datei -> Voreinstellungen öffnen,
* in das Feld "Zusätzliche Boardverwallter-URLs:" den Link "http://arduino.esp8266.com/stable/package_esp8266com_index.json" eintragen,
* Werkzeuge -> Board: "..." -> Boardverwallter ... öffnen und hier nach "esp8266" suchen und anschließend installieren.

Nun kann das Board wie jede andere unter Werkzeuge ausgewählt werden.
