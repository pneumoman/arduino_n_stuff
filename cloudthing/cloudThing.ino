#include <TimeDelay.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <Wheel.h>


#define LED_COUNT 10
#define LED_PIN 4
#define BUFFSZ 1000 //buffer for html communication
#define NSTEP 400 //# of steps to take in wheelbefore starting over at zero

#define OFF -5
#define BOLT 0
#define SOLID 20
#define RAINBOW 21
#define DBGSZ 4000

#undef MAX_RAND
#define MAX_RAND 200




//WIreless and HTML constants
const char WiFiAPPSK[] = "EvilHamster1@3";
const char htmlStart[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
const char badStart[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";

//place to store off debug information tobe recovered by debug url
char debug[DBGSZ+40];
size_t debuglen = 0;
size_t debugpos = 0;



//color store
uint8_t red = 1;
uint8_t grn = 254;
uint8_t blu = 128;
//state control
int8_t state = OFF;
int8_t oldState = -8;
//step position in wheel
uint16_t stepx = 0;

//instantiate WIFI
WiFiServer server(80);
//instantiate NeoPixels
//uint16_t n, uint8_t p=6, neoPixelType t
Adafruit_NeoPixel leds = Adafruit_NeoPixel((uint16_t)LED_COUNT, (uint8_t)LED_PIN, (neoPixelType)(NEO_RGB + NEO_KHZ400));
//setup Wheel class, let it go twice around when doing the wheel
// #steps around wheel, #of spokes
Wheel wheel(NSTEP / 2, LED_COUNT);
//setup timedelay 
TimeDelay timex;


void setup() {
  memset(debug+DBGSZ,0,40);
  //start file system,this is where the index page is stored
  SPIFFS.begin();
  //setup neopixels
  setupLeds();
  //this would start the serial port but currently is commented out
  initSerial();
  //Start AP and webserver
  setupWiFi();
  server.begin();
  char * msg=wheel.getSettings();
  Serial.println(msg);
  free(msg);//delete msg;
}


void loop()
{
  //look for http request and handle it
  chkHttp();
  if (state != OFF)
    if (state == SOLID)
    {
      //set pixels all to selected color
      doSolid();
    }
    else if (state == RAINBOW)
    {
      // run the wheel
      doRainbow2();
    }
    else
    {
      // pretendlightning
      doFlash2();
    }
}

void setupLeds()
{
  leds.begin();
  // start out off
  leds.setBrightness(1);
  setColor(0, 0, 0);
  leds.show();
}


void setupWiFi()
{
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  char name[] = "The-Alex-Cloud-";
  int l = strlen(name) + 6;
  char AP_NameChar[l];
  sprintf(&AP_NameChar[0], "%s %02X%02X", name, mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1]);
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}

void initSerial()
{
    Serial.begin(115200);
}

void chkHttp()
{
  int reqLen;
  uint8_t req[BUFFSZ];
  char dbg[30];
  // Check if a client has connected
  WiFiClient client = server.available();
  if (client)
  {
    IPAddress ip=client.remoteIP();
               //1234567891    1    2         3 4 5 6//+12
    sprintf(dbg,"Connected:%03d.%03d.%03d.%03d\r\n\r\n",ip[0],ip[1],ip[2],ip[3]);
    debugwrite(dbg);
    Serial.println(dbg);
    //Read Up request
    reqLen = client.readBytesUntil('\r', &req[0], BUFFSZ);
    client.flush();
    //process request - send response
    handleReq(client, (char *) req, reqLen);

    // Send the response to the client
    delay(1);
    Serial.println("Client disconnected");
  }
}

uint8_t * handleReq(WiFiClient client, char * req, int reqLen)
{
  //parse out action
  char * action = getAction(req, reqLen);
  size_t actionlen = strlen(action);
  // build and send response, set state
  if (actionlen > 7 && strncmp("/SVC/Bolt", action, 8) == 0)
  { //flash
    doBolt(client);
  }
  else if (actionlen > 9 &&  strncmp("/SVC/Debug", action, 10) == 0)
  {
    doDebug(client);
  }
  else if (actionlen > 9 &&  strncmp("/SVC/Disco", action, 10) == 0)
  { //wheel
    doDisco(client);
  }
  else if (actionlen > 7 &&  strncmp("/SVC/Off", action, 8) == 0)
  {
    doOff(client);
  }
  else if (actionlen > 9 &&  strncmp("/index.html", action, 10) == 0)
  {
    doRoot(client);
  }
  else if (actionlen > 10 && strncmp("/SVC/Color/", action, 11) == 0)
  { // colorize
    doColor(client, action);
  }
  else
  { //bad request
    handleBadReq(client, req, action, reqLen);
  }
  return 0;
}

void doDebug(WiFiClient client)
{
  oldState=state;
  state=-10;
  delay(1000);
  //different header since we are sending a fragment instead of the whole page
  dumbwrite(client, (char*)"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
  dumbwrite(client, (char*)debug, debuglen);
  client.flush();
  state=oldState;
}

void doBolt(WiFiClient client)
{
  //sending a minimal response to Ajax request
  dumbwrite(client, (char*)htmlStart);
  dumbwrite(client, (char*)"<head><title>BOOM</title></head><body><h1>BOOM</h1></body></html>");
  oldState = state;
  state = BOLT;
  client.flush();
}

void doDisco(WiFiClient client)
{
  //sending a minimal response to Ajax request
  dumbwrite(client, (char*)htmlStart);
  dumbwrite(client, (char*)"<head><title>BadaBOOM</title></head><body><h1>BadaBOOM</h1></body></html>");
  client.flush();
  leds.setBrightness(128);
  oldState = state;
  state = RAINBOW;
}


void doOff(WiFiClient client)
{
  //sending a minimal response to Ajax request
  dumbwrite(client, (char*)htmlStart);
  dumbwrite(client, (char*)"<head><title>OFF</title></head><body><h1>OFF</h1></body></html>");
  oldState = state;
  leds.setBrightness(1);
  setColor(0, 0, 0);
  state = OFF;
  client.flush();
}

void doRoot(WiFiClient client)
{
  size_t nRead = 0;
  uint8_t buff[BUFFSZ];
  File f = SPIFFS.open("/index.html", "r");
  if (!f)
  {
    dumbwrite(client, (char*)badStart);
    dumbwrite(client, (char*)"<head><title>nope</title></head><body><h1>No index.html</h1>");
    listRoot(client);
    dumbwrite(client, (char*)"</body></html>");
  }
  else
  {
    dumbwrite(client, (char*)"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    //read up the index file and copy it to output buffer
    do
    {
      nRead = f.readBytes(buff, BUFFSZ);
      dumbwrite(client, (char*)buff, nRead);

    }
    while (nRead == BUFFSZ);
    f.close();
  }
  client.flush();
}

void doColor(WiFiClient client, char * action)
{ // this response is really just debug information, it doesn't get used by the ajax call
  //parse out color
  char* clr = strrchr(action, '/') + 1;
  char buf[20];
  //build response
  dumbwrite(client, (char*)htmlStart);
  dumbwrite(client, (char*)"<body><h1>color:");
  dumbwrite(client, (char*)action);
  dumbwrite(client, (char*)"</h1>");
  dumbwrite(client, (char*)"<h2>");
  dumbwrite(client, clr);
  dumbwrite(client, (char*)"</h2>");
  //We do this now so that we can inclue original value in response
  //calculate colors from Hex Value: RRGGBB
  blu = strtol(clr + 4, 0, 16);
  //blu = blu*blu/256;
  clr[4] = 0;
  grn = strtol(clr + 2, 0, 16);
  //grn = grn*grn/256;
  clr[2] = 0;
  red = strtol(clr, 0, 16);
  //red = red*red/256;
  //add calculated values to response
  sprintf(buf, "<h2red:%4d</h2>", red);
  dumbwrite(client, (char*)buf);
  sprintf(buf, "<h2grn:%4d</h2>", grn);
  dumbwrite(client, (char*)buf);
  sprintf(buf, "<h2blu:%4d</h2>", blu);
  dumbwrite(client, (char*)buf);
  sprintf(buf, "\n<h2>state:%4d</h2>", state);
  dumbwrite(client, (char*)buf);
  dumbwrite(client, (char*)"</body></html>");
  //setup State
  oldState = -10; //force this state to trigger a refresh
  //depending on the previous state this may need a reset
  leds.setBrightness(128); //half power is pretty bright
  state = SOLID;
  client.flush();
}

void handleBadReq(WiFiClient client, char * req, char* action, int reqLen)
{
  char msg[20];
  sprintf(msg, "<p>size is %d</p>", reqLen);
  dumbwrite(client, (char*)badStart, strlen(badStart));
  dumbwrite(client, (char*)"<head><title>");
  dumbwrite(client, req);
  dumbwrite(client, (char *)"</title></head><body><h1>");
  dumbwrite(client, (char*)"<h2>I got something here</h2>");
  dumbwrite(client, msg);
  dumbwrite(client, (char*)"<p>action:<span>");
  dumbwrite(client, req);
  dumbwrite(client, (char*)"</span></p><p>method:<span>");
  dumbwrite(client, action);
  dumbwrite(client, (char*)"</span></p></h1></body></html>");
  client.flush();
}

void dumbwrite(WiFiClient client, char *buf)
{
  /*WIFIClient class doesn't seem to implement this*/
  size_t sizex = strlen(buf);
  dumbwrite(client, buf, sizex);
}


void dumbwrite(WiFiClient client, char *buf, size_t sizex)
{
  /*WIFIClient class doesn't seem to implement this
    Also, had to modify the class to remove the templating
    that was causing compile errors if I didn't use
    the String Class Yeah it's dumb*/
  size_t offset = 0;
  while (offset < sizex)
  {
    offset += client.write(buf + offset, sizex - offset);
  }
}

char * getAction(char *req, int len)
{
  //make sure our string is terminated
  req[len] = 0;
  //move past the action (GET POST etc)
  char * ret = strchr(req, ' ');
  *ret++ = 0;
  //Terminate at end of URL
  *(strchr(ret, ' ')) = 0;
  return ret;
}

void listRoot(WiFiClient client)
{
  //display root directory as a unsorted list
  Dir dir = SPIFFS.openDir("/");
  dumbwrite(client, (char*)"<ul>");
  while (dir.next())
  {
    dumbwrite(client, (char*)"<li>");
    dumbwrite(client, (char*)dir.fileName().c_str());
    dumbwrite(client, (char*)"</li>");
  }
  dumbwrite(client, (char*)"</ul>");
  client.flush();
}

void setColor(int redx, int grnx, int blux)
{
  //sets all leds to same color and then triggers
  //an update
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds.setPixelColor(i, redx, grnx, blux );
  }
  leds.show();
}



void doFlash2()
{
  // Sets all Leds to max brighness and then off
  // (5 times?) then return to previous state
  //really needs to be re-thought
  //should use time not simple counts
  //still it manages to work
  static int nz;
  static int nx;
  char dbg[100];
  int rval=rand()>>23;

  Serial.println(dbg);
  if (state > 11)
  {
    state = oldState;
    oldState = -10;
    leds.setBrightness(128);
  }
  else if (state == 0)
  {
    leds.setBrightness(255);
    setColor(255, 255, 255);
    //set up delay
    timex.setdelay(rval);
    state++;
  }
  else if (timex.chkdelay())
  {
    if ( state % 2 == 0)
    {
      setColor(255, 255, 255);
      sprintf(dbg,"on(%5d)<br/>",rval);
      debugwrite((char *)dbg);
      state++;
    }
    else
    {
      setColor(0, 0, 0);
      sprintf(dbg,"off(%5d)<br/>",rval);
      debugwrite((char *)dbg);
      state++;
    }
    timex.setdelay(rval);
    
  }
}


void doSolid()
{
  //On a state change, actually set the color,
  //otherwise do nothing
  char dbg[20];
  if (state != oldState)
  {
    sprintf(dbg, "Color: oldstate:%d newstate:%d", oldState, state);
    debugwrite(dbg);
    setColor(red, grn, blu);
  }
  oldState = state;
}

void debugwrite(char * txt)
{
  //copy text to debug buffer wrapping it with P tags
  // once buffer full wrap
  size_t offset = 0;
  size_t msglen = strlen(txt) + 40;
  if(debugpos > 33) 
  {
    offset=33;
  }
  size_t len = debugpos + msglen;
  if (len < (DBGSZ - 1))
  {                                //   123  45678911234567892123    45    67893 //+10
    sprintf(debug + debugpos - offset, "<p>%s</p><p>endofpnewdbg(%05d)(%05d)</p>", txt,debugpos,offset);
    debugpos = len-offset;    //                 1234567891123456    78    921234 //+10
    debug[debugpos]=' ';
    if(debuglen < debugpos)
    {
      debuglen = debugpos-1;
    }
    else
    {
      char * p=strstr(debug+debugpos,"</p>");
      if(p > 0)
      {
        Serial.printf("end:%d, found:%d\r\n\r\n",debug+debugpos,p);
        memset(debug+debugpos,' ',p-(debug+debugpos));
      }
    }
  }
  else
  {
    Serial.println("\r\n\r\n\nlog restart\n");
    //debuglen=debugpos;
    debugpos=0;
    sprintf(debug, "<p>%s</p><p>endofpnewdbg</p>", txt);
    debugpos = msglen;
    debug[debugpos]=' ';
  }
  Serial.printf("debugln:%5d debugpos%5d\r\n\r\n",debuglen,debugpos);
}

void doRainbow2()
{
  char msg[300];
  size_t l;
  msg[0]=0;
  // do color wheel
  if (timex.chkdelay(20))
  {
    for (int i = 0; i < LED_COUNT; i++)
    {
      //get color for that spoke at that step
      wheel.getColor(i, stepx, red, grn, blu);
      //l=strlen(msg);
      //if(l > 0)l--;
     // sprintf(msg+l,"<span style=\"background-color:#%02x%02x%02x\">%4d(#%02x%02x%02x)</span>",red,grn,blu,i,red,grn,blu);
     //sprintf(msg+l,"%4d, %4d(#%02x%02x%02x) ",stepx,i,red,grn,blu);
      
      leds.setPixelColor(i, red, grn, blu);
    }
   // Serial.println(msg);
   // debugwrite(msg);
    leds.show();
    //rollover step
    if (!(++stepx < NSTEP))
    {
      stepx = 0;
    }
  }
}



