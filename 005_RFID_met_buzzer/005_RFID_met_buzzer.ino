#include<Arduino.h>

const long interval = 100;      //Tijd tussen elk pakketje dat wordt verstuurd via xBee in millisecondes.
const int readDistance = 50;    //Afstand tussen reader en tag in RSSI.
const int scanPower = 500;     //min. 500, max. 2700!!

////////////////////////////////////////////////////////////// CHANGE ABOVE //////////////////////////////

#include <SoftwareSerial.h> //Used for transmitting to the device

SoftwareSerial softSerial(2, 3); //RX, TX

#include "SparkFun_UHF_RFID_Reader.h" //Library for controlling the M6E Nano module

RFID nano; //Create instance

unsigned long previousMillis = 0;
byte tagEPCBytes = 0;
String message;
bool newMessage = false;
bool newError = false;
bool readyToSend = false;

boolean setupNano(long baudRate);
void receiveMessage();
void newMessageReadyCheck();
void sendMessage();


const int BUZZER1 = 9; //buzzer to arduino pin 9
const int BUZZER2 = 10; //buzzer to arduino pin 9


void setup() {


  pinMode(BUZZER1, OUTPUT); // Set buzzer - pin 9 as an output
  pinMode(BUZZER2, OUTPUT); // Set buzzer - pin 9 as an output
  digitalWrite(BUZZER2, LOW);


  Serial.begin(57600);
  while (!Serial); //Wait for the serial port to come online

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    Serial.println(F("Module failed to respond. Please check wiring."));
    while (1); //Freeze!
  }

  nano.setRegion(REGION_EUROPE); //Set to North America

  nano.setReadPower(scanPower); //5.00 dBm. Higher values may caues USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling

  nano.startReading(); //Begin scanning for tags
}

void loop() {
  if (nano.check() == true) //Check to see if any new data has come in from module
  {
    receiveMessage();
  }

  sendMessage();
}

void sendMessage() {


  newMessageReadyCheck();

  if (newMessage && readyToSend) {
    //Serial.print("I want to send a message..  ");
    Serial.println(message);
    newMessage = false;
    readyToSend = false;

    tone(BUZZER1, 2093, 150); //C //gives a tone once tag is detected


  } else if (newError && readyToSend) {
    //Serial.print("message is an error...  ");
    Serial.println(message);
    newMessage = false;
    newError = false;
    readyToSend = false;
    message = "";
    //Serial.println("error is an send...  ");
  }
}

void newMessageReadyCheck() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    readyToSend = true;
    previousMillis = currentMillis;
  }
}

void receiveMessage() {
  byte responseType = nano.parseResponse(); //Break response into tag ID, RSSI, frequency, and timestamp

  if (responseType == RESPONSE_IS_KEEPALIVE)
  {
    //Serial.println(F("Scanning"));
  }
  else if (responseType == RESPONSE_IS_TAGFOUND)
  {
    //Serial.println("tag is found");
    //If we have a full record we can pull out the fun bits
    //int rssi = nano.getTagRSSI(); //Get the RSSI for this tag read

    //long freq = nano.getTagFreq(); //Get the frequency this tag was detected at

    //long timeStamp = nano.getTagTimestamp(); //Get the time this was read, (ms) since last keep-alive message
    message = "";

    byte tagEPCBytes = nano.getTagEPCBytes(); //Get the number of bytes of EPC from response

    //Serial.println(tagEPCBytes);

    for (byte x = 0 ; x < tagEPCBytes ; x++)
    {
      if (nano.msg[31 + x] < 0x10) {
        //message += 0;
      }
      message += nano.msg[31 + x];
    }

    newMessage = true;
  }
  else if (responseType == ERROR_CORRUPT_RESPONSE)
  {
    message = "Tag error";
    newError = true;
  }
  else
  {
    message = "Unknown error";
    newError = true;
  }
}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(softSerial); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  softSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate
  while (softSerial.isListening() == false); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while (softSerial.available()) softSerial.read();

  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    nano.stopReading();

    Serial.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    softSerial.begin(115200); //Start software serial at 115200

    nano.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg

    softSerial.begin(baudRate); //Start the software serial port, this time at user's chosen baud rate

    delay(250);
  }

  //Test the connection
  nano.getVersion();
  if (nano.msg[0] != ALL_GOOD) return (false); //Something is not right

  //The M6E has these settings no matter what
  nano.setTagProtocol(); //Set protocol to GEN2

  nano.setAntennaPort(); //Set TX/RX antenna ports to 1

  return (true); //We are ready to rock
}
