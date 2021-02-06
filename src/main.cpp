#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

#define ESP8266_RX D9 //D9 Rx should connect to TX of the Serial MP3 Player module
#define ESP8266_TX D10 //D10 Tx connect to RX of the module

static int8_t Send_buf[8] = {0};
static uint8_t ansbuf[10] = {0};

SoftwareSerial mp3(ESP8266_RX, ESP8266_TX);

const char *ssid = "MP-AP";
const char *password = "12345678"; // password length is important
WiFiServer server(80);

int g_first = true;
int g_currFolder = 0x1;
int g_lastFolder = 0x2;
int g_currSong = 0x01;
static int8_t g_maxSongs[3] = {0, 25, 14};
String g_lastMp3Answ;
//#define DEBUG;

#define CMD_NEXT_SONG 0X01 // Play next song.
#define CMD_PREV_SONG 0X02 // Play previous song.
#define CMD_PLAY_W_INDEX 0X03
#define CMD_VOLUME_UP 0X04
#define CMD_VOLUME_DOWN 0X05
#define CMD_SET_VOLUME 0X06

#define CMD_SNG_CYCL_PLAY 0X08 // Single Cycle Play.
#define CMD_SEL_DEV 0X09
#define CMD_SLEEP_MODE 0X0A
#define CMD_WAKE_UP 0X0B
#define CMD_RESET 0X0C
#define CMD_PLAY 0X0D
#define CMD_PAUSE 0X0E
#define CMD_PLAY_FOLDER_FILE 0X0F

#define CMD_STOP_PLAY 0X16 // Stop playing continuously.
#define CMD_FOLDER_CYCLE 0X17
#define CMD_SHUFFLE_PLAY 0x18  //
#define CMD_SET_SNGL_CYCL 0X19 // Set single cycle.

#define CMD_SET_DAC 0X1A
#define DAC_ON 0X00
#define DAC_OFF 0X01

#define CMD_PLAY_W_VOL 0X22
#define CMD_PLAYING_N 0x4C
#define CMD_QUERY_STATUS 0x42
#define CMD_QUERY_VOLUME 0x43
#define CMD_QUERY_FLDR_TRACKS 0x4e
#define CMD_QUERY_TOT_TRACKS 0x48
#define CMD_QUERY_FLDR_COUNT 0x4f

#define DEV_TF 0X02

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
  Serial.print("Sending: ");
#endif
  for (uint8_t i = 0; i < 8; i++)
  {
    mp3.write(Send_buf[i]);
#ifdef DEBUG
    Serial.print(sbyte2hex(Send_buf[i]));
#endif
  }
#ifdef DEBUG
  Serial.println();
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

  // if the answer format is correct.
  if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF))
  {
    return mp3answer;
  }

  return "???: " + mp3answer;
}

String decodeMP3Answer()
{
  String decodedMP3Answer = "";

  decodedMP3Answer += sanswer();
#ifdef DEBUG
  Serial.println("[ANT]" + decodedMP3Answer);
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

void handleWebRequest(WiFiClient &client)
{
  unsigned long ultimeout = millis() + 250;
  while (!client.available() && (millis() < ultimeout))
  {
    delay(1);
  }
  if (millis() > ultimeout)
  {
#ifdef DEBUG
    Serial.println("client connection time-out!");
#endif
    return;
  }

  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
#ifdef DEBUG
  Serial.println(sRequest);
#endif
  client.flush();

  // stop client, if request is empty
  if (sRequest == "")
  {
#ifdef DEBUG
    Serial.println("empty request! - stopping client");
#endif
    client.stop();
    return;
  }

  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath = "", sParam = "", sCmd = "";
  String sGetstart = "GET ";
  int iStart, iEndSpace, iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart >= 0)
  {
    iStart += +sGetstart.length();
    iEndSpace = sRequest.indexOf(" ", iStart);
    iEndQuest = sRequest.indexOf("?", iStart);

    // are there parameters?
    if (iEndSpace > 0)
    {
      if (iEndQuest > 0)
      {
        // there are parameters
        sPath = sRequest.substring(iStart, iEndQuest);
        sParam = sRequest.substring(iEndQuest, iEndSpace);
      }
      else
      {
        // NO parameters
        sPath = sRequest.substring(iStart, iEndSpace);
      }
    }
  }

  if (sParam.length() > 0)
  {
    int iEqu = sParam.indexOf("=");
    if (iEqu >= 0)
    {
      sCmd = sParam.substring(iEqu + 1, sParam.length());
#ifdef DEBUG
      Serial.println(sCmd);
#endif
    }
  }

  String sResponse, sHeader;

  if (sPath != "/")
  {
    sResponse = "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";

    sHeader = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  else
  {
    sResponse = "<html><head><title>Mp3 Player</title></head><body>";
    sResponse += "<font color=\"#000000\"><body bgcolor=\"#d0d0f0\">";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>MP3 Player AP</h1>";
    sResponse += "<FONT SIZE=+1>";
    sResponse += "<p>Next Song <a href=\"?cmd=Next\"><button>Next Song</button></a></p>";
    sResponse += "<p>Prev Song <a href=\"?cmd=Prev\"><button>Prev Song</button></a></p></p>";
    sResponse += "<br/>";
    sResponse += "<p>Next Folder <a href=\"?cmd=FolderNext\"><button>Next Folder</button></a></p>";
    sResponse += "<p>Prev Folder <a href=\"?cmd=FolderPrev\"><button>Prev Folder</button></a></p>";
    sResponse += "<br/>";
    sResponse += "<p>Cycle <a href=\"?cmd=Cycle\"><button>Cycle</button></a>";
    sResponse += "<a href=\"?cmd=VolumeUp\"><button>Up</button></a>";
    sResponse += "<a href=\"?cmd=VolumeDown\"><button>Down</button></a> </p>";

    //////////////////////
    // react on parameters
    //////////////////////
    if (sCmd.length() > 0)
    {
      sResponse += "Command:" + sCmd + "<BR>";

      if (sCmd.indexOf("Next") == 0)
      {
        g_currSong++;
        if (g_currSong > g_maxSongs[g_currFolder])
        {
          g_currSong = 1;
        }
        sendCommand(CMD_NEXT_SONG, 0x00, 0x00);
      }
      else if (sCmd.indexOf("Prev") == 0)
      {
        g_currSong--;
        if (g_currSong < 1)
        {
          g_currSong = g_maxSongs[g_currFolder];
        }
        sendCommand(CMD_PREV_SONG, 0x00, 0x00);
      }
      else if (sCmd.indexOf("FolderPrev") == 0)
      {
        g_currFolder--;
        if (g_currFolder < 1)
        {
          g_currFolder = g_lastFolder;
        }
        sendCommand(CMD_FOLDER_CYCLE, g_currFolder, 0x00);
      }
      else if (sCmd.indexOf("FolderNext") == 0)
      {
        g_currFolder++;
        if (g_currFolder > g_lastFolder)
        {
          g_currFolder = 1;
        }
        sendCommand(CMD_FOLDER_CYCLE, g_currFolder, 0x00);
      }
      else if (sCmd.indexOf("Cycle") == 0)
      {
        g_currSong++;
        if (g_maxSongs[g_currFolder] > g_currSong)
        {
          g_currSong = 1;
        }
        sendCommand(CMD_SNG_CYCL_PLAY, g_currFolder, g_currSong);
        delay(500);
        sendCommand(CMD_PLAY, 0x00, 0x00);
      }
      else if (sCmd.indexOf("VolumeUp") == 0)
      {
        sendCommand(CMD_VOLUME_UP, 0x00, 0x00);
      }
      else if (sCmd.indexOf("VolumeDown") == 0)
      {
        sendCommand(CMD_VOLUME_DOWN, 0x00, 0x00);
      }
    }
    sResponse += "<p>I: " + g_lastMp3Answ + " - "; 
    sResponse += g_currFolder;
    sResponse += " - ";
    sResponse += g_currSong;
    sResponse += "</p>";

    sResponse += "</body></html>";

    sHeader = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }

  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);

  // and stop the client
  client.stop();
}

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
  delay(1);
  Serial.println("Setup serial communication");
#endif
  mp3.begin(9600);
  delay(500);
  // //send the command [Select device] first. Serial MP3 Player
  // // only supports micro sd card, so you should send “ 7E FF 06 09 00 00 02 EF ”.
  sendCommand(CMD_SEL_DEV, 0, DEV_TF);
  delay(500);

  // AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  server.begin();
#ifdef DEBUG
  Serial.print("Start AP: ");
  Serial.println(ssid);
#endif
}

void loop()
{
  if (g_first)
  {
#ifdef DEBUG
    Serial.println("Wakeup, play the first song");
#endif
    sendCommand(CMD_FOLDER_CYCLE, g_currFolder, 0x00); // start playing the folder
    delay(100);
    g_first = false;
  }

  if (mp3.available())
  {
    g_lastMp3Answ = decodeMP3Answer();
#ifdef DEBUG
    Serial.println(g_lastMp3Answ); // never triggered?
#endif
  }
  delay(100);

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }
#ifdef DEBUG
  Serial.println("new client");
#endif
  handleWebRequest(client);
}
