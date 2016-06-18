##RFItea
Dan Ternes
www.maniacallabs.com

A timer for tea using RFID tags. In programming mode, the tags can be programmed
with the tea name and steep time. Scanning the tag in normal mode will display 
the name of the tea and start a timer. A piezo buzzer will sounds once the timer
has run down and the tea is ready.

RFID Module:
SunFounder Mifare RC522
http://www.amazon.com/SunFounder-Mifare-Antenna-Proximity-Arduino/dp/B00E0ODLWQ

Mifare 1K Classic Tags:
http://www.amazon.com/Mifare-NFC-Galaxy-Nexus-Tags/dp/B00BRKUPHA

Adafruit Monochrome 128x32 OLED I2c Graphic Display:
https://www.adafruit.com/products/931

Arduino RFID Library for MFRC522 by miguelbalboa:
https://github.com/miguelbalboa/rfid

To program tags:
NOTE: The programming prompt will time out after 5 seconds and the device
will then enter normal operation.
Connect a serial programmer to the Arduino platform and connect the programmer to 
a computer. When the port is opened, you will be prompted to enter programming 
mode. Send 'y#' to the device to begin programming. Follow the instructions to 
set the name of the tea and the time (in seconds) to steep). Once the data is 
entered, you will be prompted to scan a tag. Once the write is completed, the tag 
is ready to use.

Normal operation:
NOTE: The display will go blank after 60 seconds, but tags can still be read.
When prompted by the display, scan the RFID tag for the desired tea. The tea name 
will be displayed and the timer will start.

PINOUT (for Arduino Pro Mini):
Tag Reader:
RST/Reset   9
SPI SS      10
SPI MOSI    11
SPI MISO    12
SPI SCK     13

OLED Display:
SDA     A4
SCL     A5
RST     4

Piezo Buzzer: Pin 8

**********************/
