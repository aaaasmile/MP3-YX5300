#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

#define DEBUG

#include "Sequence.h"
#include "commands.h"
#include "web.h"

const int buttonPrev = D1;  // Botton for previous song (D0 is not working, stay always LOW)
const int buttonNext = D2;

#define ESP8266_RX D5  // Rx should connect to TX of the Serial MP3 Player module
#define ESP8266_TX D6  // Tx connect to RX of the module
SoftwareSerial mp3(ESP8266_RX, ESP8266_TX);

#ifdef DEBUG
#define Console Serial  // command processor input/output stream
#endif

static int8_t Send_buf[8] = {0};
static uint8_t ansbuf[10] = {0};
extern int g_currFolder;
extern int8_t g_maxSongs[];
EnState g_state = EnS_Init;

MyWebServer apServer;
Sequence sequence;

String g_lastMp3Answ;

int get_curr_song() {
  return sequence.GetCurrSongIx();
}

String event_to_string(EnEvent event) {
  switch (event) {
    case EnEV_GenericResponse:
      return String("EnEV_GenericResponse");

    case EnEV_SongTerminated:
      return String("EnEV_SongTerminated");

    case EnEV_PlaysongRequest:
      return String("EnEV_PlaysongRequest");

    case EnEV_NextSong:
      return String("EnEV_NextSong");

    case EnEV_PrevSong:
      return String("EnEV_PrevSong");

    case EnEV_FolderSeq:
      return String("EnEV_FolderSeq");

    case EnEV_Initialized:
      return String("EnEV_Initialized");

    default:
      return String(event);
  }
}

String state_to_string(EnState state) {
  switch (state) {
    case EnS_Init:
      return "EnS_Init";

    case EnS_WaitForStartSeq:
      return "EnS_WaitForStartSeq";

    case EnS_WaitForPlayNext:
      return "EnS_WaitForPlayNext";

    case EnS_WaitForPlayPrev:
      return "EnS_WaitForPlayPrev";

    case EnS_Playing:
      return "EnS_Playing";

    case EnS_Idle:
      return "EnS_Idle";

    default:
      return String(state, HEX);
  }
}

void raise_event(EnEvent event) {
  EnState oldState = g_state;

  switch (g_state) {
    case EnS_Init:
      switch (event) {
        case EnEV_GenericResponse:
          g_state = EnS_WaitForStartSeq;
          break;

        default:
          break;
      }
      break;

    case EnS_WaitForPlayPrev:
    case EnS_WaitForPlayNext:
    case EnS_WaitForStartSeq:
      switch (event) {
        case EnEV_PlaysongRequest:
          g_state = EnS_Playing;
          break;

        default:
          break;
      }
      break;

    case EnS_Playing:
      switch (event) {
        case EnEV_SongTerminated:
          g_state = EnS_WaitForPlayNext;
          break;
        case EnEV_NextSong:
          g_state = EnS_WaitForPlayNext;
          break;
        case EnEV_PrevSong:
          g_state = EnS_WaitForPlayPrev;
          break;
        case EnEV_FolderSeq:
          g_state = EnS_WaitForStartSeq;
          break;
        case EnEV_Initialized:
          g_state = EnS_WaitForPlayPrev;
          break;
        case EnEV_GenericResponse:
          // nothing to do
          break;
        default:
#ifdef DEBUG
          Console.print("Ignore event in state EnS_Playing : ");
          Console.print(event);
#endif
          break;
      }
      break;

    case EnS_Idle:
      break;

    default:
      break;
  }
#ifdef DEBUG
  Console.print(state_to_string(oldState));
  Console.print(" old state, event rec ");
  Console.print(event_to_string(event));
  Console.print(" State now is ");
  Console.println(state_to_string(g_state));
#endif
}

String sbyte2hex(uint8_t b) {
  String shex;

  shex = "0X";

  if (b < 16)
    shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}

void sendCommand(byte command, byte dat1, byte dat2) {
  delay(20);
  Send_buf[0] = 0x7E;     //
  Send_buf[1] = 0xFF;     //
  Send_buf[2] = 0x06;     // Len
  Send_buf[3] = command;  //
  Send_buf[4] = 0x01;     // 0x00 NO, 0x01 feedback
  Send_buf[5] = dat1;     // datah
  Send_buf[6] = dat2;     // datal
  Send_buf[7] = 0xEF;     //
#ifdef DEBUG
  Console.print("Sending: ");
#endif
  for (uint8_t i = 0; i < 8; i++) {
    mp3.write(Send_buf[i]);
#ifdef DEBUG
    Console.print(sbyte2hex(Send_buf[i]));
#endif
  }
#ifdef DEBUG
  Console.println();
#endif
}

String sanswer(void) {
  uint8_t i = 0;
  uint8_t j = 0;
  String mp3answer = "";

  bool write = false;
  while (j < 3 && i < 10) {
    while (mp3.available() && (i < 10)) {
      uint8_t b = mp3.read();
      if (i == 0 && b == 0x7e) {
        write = true;
      }
      if (write) {
        ansbuf[i] = b;
        i++;
        mp3answer += sbyte2hex(b);
      } else {
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

String decodeMP3Answer() {
  String decodedMP3Answer = "";

  decodedMP3Answer += sanswer();
#ifdef DEBUG
  Console.println("[ANT]" + decodedMP3Answer);
#endif
  raise_event(EnEV_GenericResponse);

  switch (ansbuf[3]) {
    case 0x3A:
      decodedMP3Answer += " -> Memory card inserted.";
      break;

    case 0x3D:
      decodedMP3Answer += " -> Completed play num " + String(ansbuf[6], DEC);
      if (g_lastMp3Answ != decodedMP3Answer) {
        raise_event(EnEV_SongTerminated);
      }
      break;

    case 0x3F:
      decodedMP3Answer += " -> Initialized " + String(ansbuf[6], DEC);
      if (g_lastMp3Answ != decodedMP3Answer) {
        raise_event(EnEV_Initialized);
      }
      break;

    case 0x40:
      decodedMP3Answer += " -> Error";
      break;

    case 0x41:
      decodedMP3Answer += " -> Data recived correctly. ";
      g_lastMp3Answ = "";
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

void setup() {
#ifdef DEBUG
  Console.begin(115200);
  delay(10);
  Console.println("Setup serial communication, send seldev");
#endif
  pinMode(buttonPrev, INPUT_PULLUP);
  pinMode(buttonNext, INPUT_PULLUP);

  mp3.begin(9600);
  delay(10);
  //send the command [Select device] first. Serial MP3 Player
  sendCommand(CMD_SEL_DEV, 0, DEV_TF);
  delay(500);
  apServer.Setup();
}

void loop() {
  bool bt_pressed = false;
  if (mp3.available()) {
    g_lastMp3Answ = decodeMP3Answer();
#ifdef DEBUG
    Console.println(g_lastMp3Answ);
#endif
  }
  delay(100);

  int song_ix = -1;
  if (g_state == EnS_WaitForStartSeq) {
    unsigned long ll = micros();
    ll = ll * analogRead(0);
    sequence.CreateSeq(ll, g_maxSongs[g_currFolder]);
#ifdef DEBUG
    Console.println("Time to start to play the folder");
#endif
    song_ix = sequence.GetNext();
  } else if (g_state == EnS_WaitForPlayNext) {
#ifdef DEBUG
    Console.println("Next song");
#endif
    song_ix = sequence.GetNext();
  } else if (g_state == EnS_WaitForPlayPrev) {
#ifdef DEBUG
    Console.println("Previous song");
#endif
    song_ix = sequence.GetPrev();
  }
  if (song_ix != -1) {
#ifdef DEBUG
    Console.print("Play song with ix ");
    Console.println(song_ix);
#endif
    sendCommand(CMD_PLAY_FOLDER_FILE, g_currFolder, song_ix);  // start playing the song with index
    delay(100);
    raise_event(EnEV_PlaysongRequest);
  }
  if (digitalRead(buttonPrev) == LOW) {
    if (!bt_pressed) {
      bt_pressed = true;
#ifdef DEBUG
      Serial.println("Button pressed (PREV)");
#endif
      raise_event(EnEV_PrevSong);
      delay(300);
    }
  } else if (digitalRead(buttonNext) == LOW) {
    if (!bt_pressed) {
      bt_pressed = true;
#ifdef DEBUG
      Serial.println("Button pressed (NEXT)");
#endif
      raise_event(EnEV_NextSong);
      delay(300);
    }
  } else {
    bt_pressed = false;
  }
  apServer.Update(g_lastMp3Answ);
}
