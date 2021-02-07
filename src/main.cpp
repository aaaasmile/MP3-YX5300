#include <Arduino.h>

#include <ESP8266WiFi.h>

#include "web.h"
#include "commands.h"

//#define DEBUG
#define USE_SOFTWARESERIAL 1

#if USE_SOFTWARESERIAL
#include <SoftwareSerial.h>
#define ESP8266_RX D9  // Rx2 should connect to TX of the Serial MP3 Player module
#define ESP8266_TX D10 // Tx2 connect to RX of the module
SoftwareSerial mp3(ESP8266_RX, ESP8266_TX);
#ifdef DEBUG
#define Console Serial // command processor input/output stream
#endif
#else
#include "HardwareSerial.h"
HardwareSerial Serial2(2);
#define mp3 Serial2    // Native serial port - change to suit the application
#define Console Serial // command processor input/output stream

#endif

static int8_t Send_buf[8] = {0};
static uint8_t ansbuf[10] = {0};
extern int g_currFolder;

MyWebServer apServer;

int g_first = true;
String g_lastMp3Answ;
//#define DEBUG;

String sbyte2hex(uint8_t b)
{
  String shex;

  shex = "0X";

  if (b < 16)
    shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}

void sendCommand(byte command, byte dat1, byte dat2)
{
  delay(20);
  Send_buf[0] = 0x7E;    //
  Send_buf[1] = 0xFF;    //
  Send_buf[2] = 0x06;    // Len
  Send_buf[3] = command; //
  Send_buf[4] = 0x01;    // 0x00 NO, 0x01 feedback
  Send_buf[5] = dat1;    // datah
  Send_buf[6] = dat2;    // datal
  Send_buf[7] = 0xEF;    //
#ifdef DEBUG
  Console.print("Sending: ");
#endif
  for (uint8_t i = 0; i < 8; i++)
  {
    mp3.write(Send_buf[i]);
#ifdef DEBUG
    Console.print(sbyte2hex(Send_buf[i]));
#endif
  }
#ifdef DEBUG
  Console.println();
#endif
}

String sanswer(void)
{
  uint8_t i = 0;
  String mp3answer = "";

  // Get only 10 Bytes
  while (mp3.available() && (i < 10))
  {
    uint8_t b = mp3.read();
    ansbuf[i] = b;
    i++;

    mp3answer += sbyte2hex(b);
  }

  return mp3answer;

  // if the answer format is correct.
  // if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF))
  // {
  //   return mp3answer;
  // }

  //return "???: " + mp3answer;
}

String decodeMP3Answer()
{
  String decodedMP3Answer = "";

  decodedMP3Answer += sanswer();
#ifdef DEBUG
  Console.println("[ANT]" + decodedMP3Answer);
#endif

  switch (ansbuf[3])
  {
  case 0x3A:
    decodedMP3Answer += " -> Memory card inserted.";
    break;

  case 0x3D:
    decodedMP3Answer += " -> Completed play num " + String(ansbuf[6], DEC);
    //sendCommand(CMD_NEXT_SONG);
    //sendCommand(CMD_PLAYING_N); // ask for the number of file is playing
    break;

  case 0x40:
    decodedMP3Answer += " -> Error";
    break;

  case 0x41:
    decodedMP3Answer += " -> Data recived correctly. ";
    break;

  case 0x42:
    decodedMP3Answer += " -> Status playing: " + String(ansbuf[6], DEC);
    break;

  case 0x48:
    decodedMP3Answer += " -> File count: " + String(ansbuf[6], DEC);
    break;

  case 0x4C:
    decodedMP3Answer += " -> Playing: " + String(ansbuf[6], DEC);
    break;

  case 0x4E:
    decodedMP3Answer += " -> Folder file count: " + String(ansbuf[6], DEC);
    break;

  case 0x4F:
    decodedMP3Answer += " -> Folder count: " + String(ansbuf[6], DEC);
    break;
  }

  return decodedMP3Answer;
}

void setup()
{
#ifdef DEBUG
  Console.begin(9600);
  delay(10);
  Console.println("Setup serial communication");
#endif
  //mp3.begin(9600, SerialConfig::SERIAL_8N1, SerialMode::SERIAL_FULL);
  mp3.begin(9600);
  //mp3.begin(9600, SerialConfig::SERIAL_8N1, SerialMode::SERIAL_FULL, D10, false);
  delay(10);
  sendCommand(CMD_RESET, 0, 0);
  delay(1000);
  // //send the command [Select device] first. Serial MP3 Player
  // // only supports micro sd card, so you should send “ 7E FF 06 09 00 00 02 EF ”.
  sendCommand(CMD_SEL_DEV, 0, DEV_TF);
  delay(500);
  apServer.Setup();
}

void loop()
{
  if (g_first)
  {
#ifdef DEBUG
    Console.println("Wakeup, play the first song");
#endif
    sendCommand(CMD_FOLDER_CYCLE, g_currFolder, 0x00); // start playing the folder
    delay(100);
    g_first = false;
  }

  if (mp3.available())
  {
    g_lastMp3Answ = "datav: ";
    if (g_lastMp3Answ.length() > 256)
    {
      g_lastMp3Answ = "";
    }
    g_lastMp3Answ += decodeMP3Answer();
  }
  delay(100);

  apServer.Update(g_lastMp3Answ);
}
