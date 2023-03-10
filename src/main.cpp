#include <LiquidCrystal_I2C.h>
#include <SPI.h> // include libraries
#include <LoRa.h>

#define SS 18   // LoRa radio chip select
#define RST 14  // LoRa radio reset
#define DIO0 26 // change for your board; must be a hardware interrupt pin
#define SCK 5
#define MISO 19
#define MOSI 27
#define LEDRED 14
#define LEDGREEN 13
#define LEDBLUE 25

String LAT, LON, ALT;
String ID = "TEST IMU + HR LORA";
String RSI = "NAN";
String SNR = "NAN";

byte msgCount = 0;    // count of outgoing messages
int localAddress = 0; // address of this device
int Destination = 0;
long lastSendTime = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
// HardwareSerial GPS(1);

void sendMessage(String outgoing);
void onReceive(int packetSize);
void LEDCOLOR(String color);

void setup()
{
  Serial.begin(9600); // initialize serial
  pinMode(LEDRED, OUTPUT);
  pinMode(LEDGREEN, OUTPUT);
  pinMode(LEDBLUE, OUTPUT);
  LEDCOLOR("OFF");
  // put your setup code here, to run once:
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(915E6))
  { // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true)
      ; // if failed, do nothing
  }
  lcd.init();
  lcd.backlight();
}

void loop()
{
  // if (millis() - lastSendTime > 0)
  // {
  //   LEDCOLOR("OFF");
  // }
  if (millis() - lastSendTime > 100)
  {
    LEDCOLOR("OFF");
    lcd.setCursor(0, 0);
    lcd.print(String() + "  " + ID);
    lcd.setCursor(0, 1);
    lcd.print(String() + "RSSI=" + String(LoRa.packetRssi()) + "SNR=" + String(LoRa.packetSnr()));
    // if (Destination == localAddress)
    // {
    //   Serial.println(String() + ID + "," + LAT + "," + LON + "," + ALT + "," + RSI + "," + SNR + ",*");
    //   Destination++;
    // }
    // else if (Destination == 1)
    // {
    //   String message = "REQ,*"; // send a message
    //   sendMessage(message);
    //   Destination++;
    // }
    // else if (Destination >= 2)
    // {
    //   String message = "REQ,*"; // send a message
    //   sendMessage(message);
    //   Destination = 0;
    // }
    lastSendTime = millis(); // timestamp the message
  }
  onReceive(LoRa.parsePacket());
}

void sendMessage(String outgoing)
{
  LoRa.beginPacket();            // start packet
  LoRa.write(Destination);       // add destination address
  LoRa.write(localAddress);      // add sender address
  LoRa.write(msgCount);          // add message ID
  LoRa.write(outgoing.length()); // add payload length
  LoRa.print(outgoing);          // add payload
  LoRa.endPacket();              // finish packet and send it
  msgCount++;                    // increment message ID
}

void onReceive(int packetSize)
{
  if (packetSize == 0)
  {
    // Serial.println("Packet 0");
    return; // if there's no packet, return
  }

  // read packet header bytes:
  int recipient = LoRa.read();       // recipient address
  byte sender = LoRa.read();         // sender address
  byte incomingMsgId = LoRa.read();  // incoming msg ID
  byte incomingLength = LoRa.read(); // incoming msg length

  String incoming = "";

  while (LoRa.available())
  {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length())
  { // check length for error
    Serial.println("error: message length does not match length");
    return; // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF)
  {
    Serial.println("This message is not for me.");
    return; // skip rest of function
  }
  LEDCOLOR("GREEN");
  Serial.println(incoming);
  RSI = String(LoRa.packetRssi());
  SNR = String(LoRa.packetSnr());
}

void LEDCOLOR(String color)
{
  if (color == "RED")
  {
    digitalWrite(LEDRED, HIGH);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, LOW);
  }
  else if (color == "GREEN")
  {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, HIGH);
    digitalWrite(LEDBLUE, LOW);
  }
  else if (color == "BLUE")
  {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, HIGH);
  }
  else if (color == "OFF")
  {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, LOW);
  }
}