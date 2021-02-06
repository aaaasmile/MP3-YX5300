#include "web.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "commands.h"

const char *ssid = "MP-AP";
const char *password = "12345678"; // password length is important
int g_currFolder = 0x1;
int g_lastFolder = 0x2;
int g_currSong = 0x01;
static int8_t g_maxSongs[3] = {0, 25, 14};

extern void sendCommand(byte command, byte dat1, byte dat2);

WiFiServer server(80);

void handleWebRequest(WiFiClient &client, String lastMp3Answ)
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
        sResponse += "<p>I: " + lastMp3Answ + " - ";
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

MyWebServer::MyWebServer()
{
}

void MyWebServer::Setup()
{

    // AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    server.begin();
#ifdef DEBUG
    Serial.print("Start AP: ");
    Serial.println(ssid);
#endif
}

void MyWebServer::Update(String lastMp3Answ)
{
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client)
    {
        return;
    }
#ifdef DEBUG
    Serial.println("new client");
#endif
    handleWebRequest(client, lastMp3Answ);
}