#include <Arduino.h>
#include <ets_sys.h>
#include "osapi.h"

#include <ESP8266WiFi.h>
#include <stdint.h>
#include "web.h"
#include "commands.h"

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

String decodeMP3Answer()
{
  String decodedMP3Answer = "";

  decodedMP3Answer += sanswer();
  unsigned long ll = micros();
  
#ifdef DEBUG
  Console.println("[ANT]" + decodedMP3Answer);
  Console.println(ll);
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

byte g_sample = 0;
boolean g_sample_waiting = false;
byte g_current_bit = 0;
byte g_result = 0;

// Rotate bits to the left
// https://en.wikipedia.org/wiki/Circular_shift#Implementing_circular_shifts
byte rotl(const byte value, int shift) {
  if ((shift &= sizeof(value)*8 - 1) == 0)
    return value;
  return (value << shift) | (value >> (sizeof(value)*8 - shift));
}

void wdtSetup() {
  // cli();
  // MCUSR = 0;
  
  // /* Start timed sequence */
  // WDTCSR |= _BV(WDCE) | _BV(WDE);

  // /* Put WDT into interrupt mode */
  // /* Set shortest prescaler(time-out) value = 2048 cycles (~16 ms) */
  // WDTCSR = _BV(WDIE);

  // sei();
  //ESP.wdtFeed(); // reset watchdog
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
  wdtSetup();
}

void loop()
{
  if (g_sample_waiting) {
    g_sample_waiting = false;

    g_result = rotl(g_result, 1); // Spread randomness around
    g_result ^= g_sample; // XOR preserves randomness

    g_current_bit++;
    if (g_current_bit > 7)
    {
      g_current_bit = 0;
      Console.println(g_result); // raw binary
    }
  }

  if (mp3.available())
  {
    g_lastMp3Answ = decodeMP3Answer();
#ifdef DEBUG
    Console.println(g_lastMp3Answ);
#endif
  }
  delay(100);

  if (g_first)
  {
#ifdef DEBUG
    Console.println("Wakeup");
#endif
    //sendCommand(CMD_QUERY_FLDR_TRACKS, g_currFolder, 0x00); // works with antw 0x41?
    //sendCommand(CMD_QUERY_TOT_TRACKS, 0, 0);
    //sendCommand(CMD_QUERY_FLDR_COUNT, 0, 0);
    //sendCommand(CMD_QUERY_STATUS, 0, 0);
    //delay(500);
    //sendCommand(CMD_FOLDER_CYCLE, g_currFolder, 0x00); // start playing the folder (OK)
    sendCommand(CMD_PLAY_FOLDER_FILE, g_currFolder, 0x04); // start playing the song with index
    delay(100);
    g_first = false;
  }

  apServer.Update(g_lastMp3Answ);
}
