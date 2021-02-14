#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <stdint.h>
#include "web.h"
#include "commands.h"
#include "Sequence.h"

#define DEBUG

#include <SoftwareSerial.h>
#define ESP8266_RX D5 // Rx should connect to TX of the Serial MP3 Player module
#define ESP8266_TX D6 // Tx connect to RX of the module
SoftwareSerial mp3(ESP8266_RX, ESP8266_TX);

#ifdef DEBUG
#define Console Serial // command processor input/output stream
#endif

static int8_t Send_buf[8] = {0};
static uint8_t ansbuf[10] = {0};
extern int g_currFolder;
extern int8_t g_maxSongs[];
EnState g_state;

MyWebServer apServer;
Sequence sequence;

String g_lastMp3Answ;

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
  uint8_t j = 0;
  String mp3answer = "";

  bool write = false;
  while (j < 3 && i < 10)
  {
    while (mp3.available() && (i < 10))
    {
      uint8_t b = mp3.read();
      if (i == 0 && b == 0x7e)
      {
        write = true;
      }
      if (write)
      {
        ansbuf[i] = b;
        i++;
        mp3answer += sbyte2hex(b);
      }
      else
      {
#ifdef DEBUG
        Console.print("Ignore byte");
        Console.println(b);
#endif
      }
    }
    delay(4);
    j++;
  }

  return mp3answer;
}

void raise_event(EnEvent event)
{
  switch (g_state)
  {
  case Init:
    switch (event)
    {
    case GenericResponse:
      g_state = WaitForStartSeq;
      break;

    default:
      break;
    }
    break;

  case WaitForStartSeq:
    switch (event)
    {
    case PlaysongRequest:
      g_state = Playing;
      break;

    default:
      break;
    }
    break;

  case Playing:
    switch (event)
    {
    case SongTerminated:
      g_state = WaitForPlayNext;
      break;

    default:
      break;
    }
    break;

  case Idle:
    break;

  default:
    break;
  }
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
  Console.begin(115200);
  delay(10);
  Console.println("Setup serial communication, send seldev");
#endif
  //mp3.begin(9600, SerialConfig::SERIAL_8N1, SerialMode::SERIAL_FULL);
  mp3.begin(9600);
  //mp3.begin(9600, SerialConfig::SERIAL_8N1, SerialMode::SERIAL_FULL, D10, false);
  delay(10);
  //sendCommand(CMD_RESET, 0, 0);
  //delay(1000);
  // //send the command [Select device] first. Serial MP3 Player
  // // only supports micro sd card, so you should send “ 7E FF 06 09 00 00 02 EF ”.
  sendCommand(CMD_SEL_DEV, 0, DEV_TF);
  delay(500);
  apServer.Setup();
}

void loop()
{
  if (mp3.available())
  {
    g_lastMp3Answ = decodeMP3Answer();
#ifdef DEBUG
    Console.println(g_lastMp3Answ);
#endif
  }
  delay(100);

  bool play_next = false;
  if (g_state == WaitForStartSeq)
  {
    unsigned long ll = micros();
    sequence.CreateSeq(ll, g_maxSongs[g_currFolder]);
#ifdef DEBUG
    Console.println("Time to start to play the folder");
    Console.println(ll);
#endif
    play_next = true;
    //sendCommand(CMD_QUERY_FLDR_TRACKS, g_currFolder, 0x00); // works with antw 0x41?
    //sendCommand(CMD_QUERY_TOT_TRACKS, 0, 0);
    //sendCommand(CMD_QUERY_FLDR_COUNT, 0, 0);
    //sendCommand(CMD_QUERY_STATUS, 0, 0);
    //delay(500);
    //sendCommand(CMD_FOLDER_CYCLE, g_currFolder, 0x00); // start playing the folder (OK)
  }
  if (g_state == WaitForPlayNext)
  {
#ifdef DEBUG
    Console.println("Next song");
#endif
    play_next = true;
  }
  if (play_next)
  {
    int next = sequence.GetNext();
#ifdef DEBUG
    Console.print("Play next ");
    Console.println(next);
#endif
    sendCommand(CMD_PLAY_FOLDER_FILE, g_currFolder, next); // start playing the song with index
    delay(100);
    raise_event(PlaysongRequest);
  }

  apServer.Update(g_lastMp3Answ);
}
