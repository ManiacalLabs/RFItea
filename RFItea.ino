/*********************
RFItea
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

// ************************************************
// MFRC522 RFID Module includes/defines
// ************************************************
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 9
#define SS_PIN  10

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

// ************************************************
// Adafruit OLED I2C Display includes/defines
// ************************************************
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DISPLAY_WAIT_TIMEOUT 60000
#define OLED_RESET 4

Adafruit_SSD1306 display(OLED_RESET);

// ************************************************
// States for state machine
// ************************************************
enum operatingState {PROG = 0, WAIT, TIME, DONE};
operatingState opState = PROG;

// ************************************************
// Globals
// ************************************************
int teaTime = 0;  //how long (in seconds) to run timer
char teaName[32] = "";  //holds name of tea

void setup() {
  Serial.begin(9600); // Init PC Comm Serial
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 module

  // Prepare the key using FFFFFFFFFFFFh which is the default
  // at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  //Init OLED display and show RFItea "splash screen"
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("- - - RFItea - - -");
  display.display();
  delay(1000);
}

void loop() {

  switch (opState) {
    case PROG:
      programTag();
      break;
    case WAIT:
      waitForTag();
      break;
    case TIME:
      runTimer();
      break;
    case DONE:
      timerDone();
      break;
  }
}

void programTag()
{
  byte buffer[34];
  byte len;
  byte nameBlock[16];
  byte nameBlock2[16];
  byte timeBlock[16] = {0};
  byte trailerBlock   = 7;
  MFRC522::StatusCode status;

  String toIntArray = "";

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Enter Prog. Mode?");
  display.display();

  Serial.setTimeout(5000L);
  Serial.println(F("enter 'y#' to begin programming mode..."));
  len = Serial.readBytesUntil('#', (char *) buffer, 30) ;
  if (buffer[0] != 'y')
  {
    Serial.println(F("Entering normal operation..."));
    opState = WAIT;
    return;
  }

  Serial.setTimeout(20000L);
  Serial.println(F("Type tea name, ending with # (max 21 chars)"));
  len = Serial.readBytesUntil('#', (char *) buffer, 30) ;
  for (byte i = len; i < 30; i++) buffer[i] = ' ';     // pad with spaces

  for (byte i = 0; i < 16; i++) {
    nameBlock[i] = byte(buffer[i]);
  }

  for (byte i = 0; i < 16; i++) {
    nameBlock2[i] = byte(buffer[i + 16]);
  }

  Serial.setTimeout(20000L);
  Serial.println(F("Enter time in seconds, ending with #"));
  len = Serial.readBytesUntil('#', (char *) buffer, 30) ;

  for (byte i = 0; i < 16; i++) {
    toIntArray = toIntArray + char(buffer[i]);
  }

  teaTime = toIntArray.toInt();

  timeBlock[0] = lowByte(teaTime);
  timeBlock[1] = highByte(teaTime);

  Serial.println(F("Scan Tag..."));

  while (! mfrc522.PICC_IsNewCardPresent()) {}

  while (! mfrc522.PICC_ReadCardSerial()) {}

  // Authenticate using key A
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write data to block 4 (first block of tea name)
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(4, nameBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  // Write data to block 5 (second block of tea name)
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(5, nameBlock2, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  // Write data to block 6 (steep time)
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(6, timeBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  tone(8, 900, 300); //Play tone to confirm successful tag write

  Serial.println(F("Write Complete!"));
  delay(2000);
  
  opState = WAIT;
  return;
}

void waitForTag()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Ready...");
  display.display();

  byte buffer[18];
  byte size = sizeof(buffer);
  byte len;
  byte trailerBlock   = 7;
  MFRC522::StatusCode status;

  unsigned long dispOffTime = millis();
  unsigned long dispOff = DISPLAY_WAIT_TIMEOUT;

  while (! mfrc522.PICC_IsNewCardPresent()) {
    if (millis() > (dispOffTime + dispOff))
    {
      display.clearDisplay();   // "screen saver" Tags can still be read
      display.display();
    }
  }

  while (! mfrc522.PICC_ReadCardSerial()) {}

  // Authenticate using key A
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Read data from block 4 (first block of tea name)
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(4, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  for (byte i = 0; i < 16; i++) {
    teaName[i] = char(buffer[i]);
  }

  // Read data from block 5 (second block of tea name)
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(5, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  for (byte i = 0; i < 16; i++) {
    teaName[i + 16] = char(buffer[i]);
  }

  // Read data from block 6 (steep time)
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(6, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  teaTime = buffer[0] + (buffer[1] << 8);

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  tone(8, 900, 300); //Play tone to confirm successful tag read

  opState = TIME;
  return;
}

void runTimer()
{
  for (int i = teaTime; i > 0; i--) {
    delay(1000-59); //display update takes ~59 ms. SLOWWW!!!!
    teaTime--;
    displayPrintNameTime(teaName, teaTime);
  }
  opState = DONE;
  return;
}

void timerDone()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Tea Ready!");
  display.display();

  playFinishedTone();
  delay(2000);
  playFinishedTone();
  delay(2000);
  playFinishedTone();

  delay(10000);

  opState = WAIT;
  return;
}

void displayPrintNameTime(String teaName, int teaTime) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(teaName);
  display.setCursor(0, 10);
  display.setTextSize(3);
  display.println(teaTime);
  display.display();
}

void playFinishedTone() {
  tone(8, 1100, 100);
  delay(200);
  tone(8, 1100, 100);
  delay(200);
  tone(8, 1100, 100);
  delay(200);
  tone(8, 1100, 100);
  delay(200);
  tone(8, 1100, 100);
  delay(200);
}
