# MäCAN-Weichendecoder (Magnet)

Der nachfolgende Teil bezieht sich ausschließlich auf die Hardware-Revision A und Software-Version 0.2.1. Es ist zu beachten, welche Software-Version mit welcher Hardware-Revision kompatibel ist! Alle Hardware-Versionen der gleichen Revision sind zuheinander kompatibel und unterscheiden sich nicht in ihrer Funktionsweise. 

Das Projekt ist noch im Anfangsstadium. Es ist also mit Veränderungen in relativ kurzen abständen zu rechnen und weder Hard- noch Software sind final! Hier kann sich jederzeit einiges verändern. Es wird versucht diese Dokumentation so aktuell wie möglich zu halten. Hierfür besteht aber keine Gewähr.

## Changelog

##### V0.2.1

+ Der Decoder behält nun seinen letzten Schaltzustand bei und führt diesen bei jedem Einschalten erneut aus.

##### V0.2.0

+ Auswahl des Betriebsmodus (Dauer- oder Momentkontakt) und der Schaltzeit (bei Momentkontakten).
+ Lagerückmeldung per Endabschaltung der Weichenantriebe kann ein oder aus geschaltet werden.
+ Das doppelte Schalten eines Ausgangs kann ein oder aus geschaltet werden (bei Momentkontakten).

## Dokumentation

#### Inhalt

* [Das wird benötigt](#daswirdbenötigt)
* [Flashen des ATMega328P](#flashen-des-atmega328p)

#### Das wird benötigt

* Arduino (getestet mit UNO R3)
* Passendes Programmierkabel
* Arduino IDE
* MäCAN Weichendecoder (Magnet) 

##### Programmierkabel

(kommt noch)

##### Anschluss des Decoders an den Arduino

(kommt noch)

#### Flashen des ATMega328P

Um den Decoder in betrieb nehemn zu können, muss, wenn nicht schon geschehen, die zur Hardware-Revision passende Software auf den Mikrocontroller (µC) gebrannt werden. Hierzu ist ein Arduino sowie ein passendes [Programmierkabel](#programmierkabel) notwendig.

##### Arduino als ISP

Um den Decoder programmieren zu können muss ein Arduino (getestet mit UNO R3) als ISP programmiert werden. Dies ist mithilfe der Arduino-IDE sehr einfach.
Dafür wird der Arduino nach erfolgreicher Installation der Arduino-IDE wie gewöhnlich per USB an den Computer angeschlossen. Nun kann in der IDE das Beispiel "ArduinoISP" ausgewählt werden:

![img1](/images/arduino-ISP_1.png)

Sofern das richtige Board ausgewählt wurde kann dieser Sketch nun hochgeladen werden:

![img2](/images/arduino-ISP_2.png)
![img3](/images/arduino-ISP_3.png)

Nun sollte für alle weiteren Schritte als Programmer "Arduino as ISP" ausgewählt werden. Andernfalls ist ein Hochladen der Software auf den Decoder nicht möglich!

##### Erstmaliges Flashen

Ist der µC fabrikneu muss zu aller erst der Arduino Bootloader auf den Chip gebrannt werden. Der Bootloader ist zwar im Betrieb nicht mehr notwendig, allerdings werden so die internen Einstellungen des µC auf die Hardware angepasst.
Sofern der Decoder richtig mit dem Arduino verbunden wurde kann mit den folgenden Einstellungen der Bootloader auf den Decoder geladen werden:

![img4](/images/arduino-bootloader_1.png)

Hinweis: Kommt der µC schon mit einer beliebeigen Version der Decodersoftware, so ist dieser schritt optional und für die Funktion NICHT notwendig!

##### Laden einer neuen Software-Version

Das Projekt befindet sich noch in der Entwicklungsphase, es wird also in unregelmäßigen Abständen neue Softwareversionen geben. Es ist immer sinnvoll, diese möglichst bald aufzuspielen, da hier Fehler behoben werden und neue oder verbesserte Funktionen implementiert werden. Die Software, die sic im "master"-Branch befindet ist getestet und sollte sich immer problemlos einsetzen lassen.

Um eine neue Software aufzuspielen muss der Sketch in der Arduino-IDE geöffnet werden sowie alle Einstellungen gemäß [Arduino als ISP](#arduino-als-isp) getätigt werden. Nachdem die [Konstanten](#konstanten) nach den eigenen Wünschen bearbeitet wurden kann die Software wie folgt hochgeladen werden:

![img5](/images/arduino-upload_1.png)
