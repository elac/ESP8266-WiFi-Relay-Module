//ESP Relay Web Page by Elac 1/1/18
/*-----------------------------------------------------------------------------------------------------
  Access Point                        Web Server  e.g.
  http://192.168.4.1                  http://192.168.1.252 (depends on your network)


  Works with generic ESP01

  Usage:  ACCESS Point:
         After upload search with your device (Phone, Tablet, etc.) for
         new WiFi. Select ESP_FastLED_Access_Point.
         Open in your webbrowser the URL 192.168.4.1
         Optional print a QR-Code with the URL on your lamp http://goqr.me/

         WEB SERVER:
         After upload open the Serial Monitor in Arduino and see what
         IP address is returned. In my case it is 192.168.1.252
         Open this IP address in a browser (PC or phone)

  From Gyro Gearloose J. Bruegl, Feb 2016

  /*------------------------------------------------------------------------------------------------------
  HTTP 1.1 Webserver for ESP8266 adapted to Arduino IDE

  From Stefan Thesen 04/2015
  https://blog.thesen.eu/http-1-1-webserver-fuer-esp8266-als-accesspoint/
  https://blog.thesen.eu/stabiler-http-1-1-wlan-webserver-mit-dem-esp8266-microcontroller
  -----------------------------------------------------------------------------------------------------*/
#include <ESP8266WiFi.h>

byte crm; // Holds if command was recieved to change relay mode
byte  relayMode; // Holds current relay state
byte relON[] = {0xA0, 0x01, 0x01, 0xA2};  // Hex command to send to serial for open relay
byte relOFF[] = {0xA0, 0x01, 0x00, 0xA1}; // Hex command to send to serial for close relay

// Select EITHER ACCESS-Point  OR  WEB SERVER setup

// ACCESS-Point setup ------------------------------------------------------------------------------------------------------
const char* ssid = "ESP Relay";
const char* password = "";  // set to "" for open access point w/o password; or any other pw (min length = 8 characters)

unsigned long ulReqcount;
// Create an instance of the server on Port 80
WiFiServer server(80);
//IPAddress apIP(192, 168, 10, 1);   // if you want to configure another IP address
void setup()
{
  // Start Serial
  Serial.begin(9600);
  delay(500);
  // setup globals
  relayMode = 0;
  crm = 0;
  ulReqcount = 0;
  // prepare GPIO2
  // pinMode(2, OUTPUT);
  ///14 digitalWrite(2, 0);
  // AP mode
  WiFi.mode(WIFI_AP);
  //  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); // if you want to configure another IP address
  WiFi.softAP(ssid, password);
  server.begin();
}
// End ACCESS-Point setup ---------------------------------------------------------------------------------------------------

// WEB SERVER setup ---------------------------------------------------------------------------------------------------------

/*const char* ssid = ""; // Enter router SSID
  const char* password = ""; // Enter router password
  unsigned long ulReqcount;
  unsigned long ulReconncount;

  WiFiServer server(80);  // Create an instance of the server on Port 80
  void setup()
  {
  // Start Serial
  Serial.begin(9600);
   relayMode = 0;
  crm = 0;
  ulReqcount = 0;       // Setup globals for Webserver
  ulReconncount = 0;
  // Set static IP or comment out // next 4 lines for router to assign
  IPAddress ip(192, 168, 1, 190); // Put the static IP addres you want to assign here
  IPAddress gateway(192, 168, 1, 1); // Put your router gateway here
  IPAddress subnet(255, 255, 255, 0); 
  WiFi.config(ip, gateway, subnet);
  WiFi.mode(WIFI_STA);
  WiFiStart();

  }*/
// End WEB SERVER setup -----------------------------------------------------------------------------------------------------
void WiFiStart()
{
WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // Start the server
  server.begin();
}
/////////////////////////////////////////////////////////////
void loop() {
  Webserver();
  CheckForChange();
 };
///////////////////////////////////////////////////////////
void CheckForChange() {
  if (crm == 1)
  {
    switch (relayMode) {
      case 0: {
          Off();
        }
        break;
      case 1: {
          On();
        }
        break;
    }
    crm = 0;
  }
};
///////////////////////////////////////////////////////////////////////////
void Webserver() {   /// complete web server (same for access point) ////////////////////////////////////////////////////////
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }
  // Wait until the client sends some data
  unsigned long ultimeout = millis() + 250;
  while (!client.available() && (millis() < ultimeout) )
  {
    delay(1);
  }
  if (millis() > ultimeout)
  {
    return;
  }
  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  client.flush();
  // stop client, if request is empty
  if (sRequest == "")
  {
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
    if (iEndSpace > 0)
    {
      if (iEndQuest > 0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart, iEndQuest);
        sParam = sRequest.substring(iEndQuest, iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart, iEndSpace);
      }
    }
  }
  if (sParam.length() > 0)
  {
    int iEqu = sParam.indexOf("=");
    if (iEqu >= 0)
    {
      sCmd = sParam.substring(iEqu + 1, sParam.length());
      char carray[4];                                // values 0..255 = 3 digits; array = digits + 1
      sCmd.toCharArray(carray, sizeof(carray));      // convert char to the array
    }
  }

  //////////////////////////////
  // format the html response //
  //////////////////////////////
  String sResponse, sHeader;
  ///////////////////////////////
  // 404 for non-matching path //
  ///////////////////////////////
  if (sPath != "/")
  {
    sResponse = "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  //////////////////////////
  // format the html page //
  //////////////////////////
  else
  {
    ulReqcount++;
 sResponse  = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>ESP8266 Relay</title></head>";
    sResponse += "<body bgcolor=\"#1F1F1F\" >";
    sResponse += "<div style='text-align:center; height:96%; border:10px groove blue; border-radius: 5px;'>";
    sResponse += "<strong><h1><i><a style=color:#00CCCC>ESP Relay</a></i></h1></strong><br><br><br><br>";
    ///// Buttons
    sResponse += "<form action=\"?sCmd\">";
    sResponse += "<button name=\"sCmd\" value=\"FUNCTION1ON\" style=\"color:yellow; background-color:black; font-size:130%; width:80px; margin-right:20px;\" onclick=\'this.form.submit();'>ON</button>";
    sResponse += "<button name=\"sCmd\" value=\"OFF\" style=\"color:yellow; background-color:black; font-size:130%; width:80px; margin-left:20px;\" onclick=\'this.form.submit();'>OFF</button>";
    sResponse += "</select></form><br><br><br><br>";
    /////////////////////////////
    // react on parameters //
    /////////////////////////////
    if (sCmd.length() > 0)
    {
      // write received command to html page
      // change the effect
      if (sCmd.indexOf("FUNCTION1ON") >= 0)
      {
        relayMode = 1;
        crm = 1;
      }
      else if (sCmd.indexOf("OFF") >= 0)
      {
        relayMode = 0;
        crm = 1;
      }
    };
    sResponse += "<font size=+2><a style=color:#FFFDD2 >Relay State:</a><i>&nbsp;";
    switch (relayMode) {
      case 0:
        sResponse += "<a style=color:#FF0E0E >Off</a>";
        break;
      case 1:
        sResponse += "<a style=color:#00FF00 >On</a></i>";
        break;
    }

    sResponse += "</div></body></html>";
    sHeader  = "HTTP/1.1 200 OK\r\n";
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
};  // End of web server
///////////////////////////////////////////////////////////////////////////
/// END of complete web server /////////////////////////////////////
///Button functions
void Off() {
  Serial.write(relOFF, sizeof(relOFF));
};

void On() {
  Serial.write(relON, sizeof(relON));
};
///End of sketch/////////////////////////////////////////
