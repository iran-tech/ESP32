  /*
  V0 Ack
  V2 Clock
  V3 Conecct
  V5 Receive  IR
  V6 Send     IR
  V7 Geting data
  */ 
#define  texterr   "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n\
<html>\
<head></head>\
<meta name='viewport' content='width=device-width, initial-scale=1'>\
<body>\
<h1>Iran Tech WiFi Controller</h1>\
<div id='config'>\
<script>\
if (window.location.search.substr(1) != '')\
{\
document.getElementById('config').display = 'none';\
document.body.innerHTML ='<h1>ESP WiFi NAT Router Config</h1>The new settings have been sent to the device...';\
setTimeout(\"location.href = '/'\",10000);\
}\
</script>\
<h2>STA Settings</h2>\
<form action='' method='GET'>\
<table>\
<tr>\
<td>SSID:</td>\
<td><input type='text' name='ssid' value='%s'/></td>\
</tr>\
<tr>\
<td>Password:</td>\
<td><input type='text' name='\tpassword' value='%s'/></td>\
</tr>\
<td>Automesh:</td>\
<td><input type='checkbox' name='am' value='mesh' %s></td>\
</tr>\
<tr>\
<td></td>\
<td><input type='submit' value='Connect'/></td>\
</tr>\
\
</table>\
</form>\
\
<h2>Device Management</h2>\
<form action='' method='GET'>\
<table>\
<tr>\
<td>Reset Device:</td>\
<td><input type='submit' name='reset' value='Restart'/></td>\
</tr>\
</table>\
</form>\
</div>\
</body>\
</html>\
"
#define BLYNK_TEMPLATE_ID "TMPLQEe-FG5B"
#define BLYNK_DEVICE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "x1CkiZ569Ht5e9SRyIcz5hlCAvq3893n"
#define BLYNK_PRINT Serial
#define LENGTH(x) (strlen(x) + 1)   // length of char string
#define EEPROM_SIZE 200             // EEPROM size
#define Red_LED     33
#define Green_LED   26
#define White_LED   13
#define Buttonn     12

#include <Arduino.h>
#include <WiFi.h>
#include "EEPROM.h"
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <BlynkSimpleEsp32.h>
#include <stdint.h>
#include <IRremoteESP8266.h>

#define USE_SERIAL Serial
#include <IRsend.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

#include <HTTPClient.h>

const uint16_t kIrLed   = 25;  // Send GPIO Pin
const uint16_t kRecvPin = 35;  // Receive GPIO Pin
uint8_t freq = 38;             // carrier freq
uint16_t IRCode[200];          // Raw Array to Send
uint8_t  number_raw = 0;       // Num of Send Array
uint8_t wifi_init = 1;         // WiFi status
//uint8_t i = 1;                 // Num of Receive Array
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

const uint16_t kCaptureBufferSize = 1024;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;  // kTolerance is normally 25%

  
#if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t kTimeout = 50;
#else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif  // DECODE_AC

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true); // Set the GPIO to be used to Receiving the message.
decode_results results;  // Somewhere to store Receive results

char auth[] = BLYNK_AUTH_TOKEN;

// Fill WiFi details 
char ssid1[50];
char pass1[50];

// Set these to your desired credentials.
const char *ssid = "Parlar";
const char *password = "Parlar123";
String newssid = "NAN";
String newpassword= "NAN";


WiFiServer server(80);

void getSSIDPasword(String data) // Get SSID from Server
{
    if(!data.startsWith("GET /?ssid="))
        return;
    int i = 11;
    String temp = "";
    while(data[i]!='&'){
        temp+=data[i];
        i++;
        if(i>= data.length())
        break;
    }
    i+=13;
    newssid = String(temp);
    temp = "";
    while(data[i]!=' ')
    {
      temp+=data[i];
      i++;
      if(i>= data.length())
        break;   
    }

    newpassword = String(temp);
}

  void toUINT(String s) // Get Data from Server
  {
    int i = 0;
    String temp = "";
    for(int j = 0; j <= s.length(); j++)
    {
        if(s[j] == ',')
        {
            temp.trim();
            IRCode[i] = (int) temp.toInt();
            temp = "";
            i++;
        }else
        {
            temp += String(s[j]);    
        }
    }
    number_raw = i;
}

String absolute(String s) // Prepare Received IR for Send to Server 
{
     String result = "";
     bool b = false;
     for(int i = 0; i < s.length(); i++)
        if(b){
            if(s[i] == '}')
                return result;
            else
                result += String(s[i]);
        }else
            if(s[i] == '{')
                b = true;
    return result;
 }

void connecter(String s) // Send Array to Server
{
  if ((WiFi.status() == WL_CONNECTED)) 
    { //Check the current connection status

      //WiFiClient client;
      HTTPClient http;

      String URLER = String("https://bsite.net/irantech/ParlarProject/learn.aspx?token=")+ String(BLYNK_AUTH_TOKEN);
    
      // Your Domain name with URL path or IP address with path
      http.begin(URLER);

      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "value="+s;           
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      if (httpResponseCode > 0) { //Check for the returning code
        
        String payload = http.getString();
        Serial.print("IR Array = ");
        Serial.println(s);
        Serial.println(httpResponseCode);
        Serial.println(payload);
      }
  
    else {
      Serial.println("Error on HTTP request");
    }
}
}

void writeStringToFlash(const char* toStore, int startAddr) // Save to EEPROM 
{
  int i = 0;
  for (; i < LENGTH(toStore); i++) {
    EEPROM.write(startAddr + i, toStore[i]);
  }
  EEPROM.write(startAddr + i, '\0');
  EEPROM.commit();
}

String readStringFromFlash(int startAddr) // read from Save to EEPROM 
{
  char in[128]; // char array of size 128 for reading the stored data 
  int i = 0;
  for (; i < 128; i++) {
    in[i] = EEPROM.read(startAddr + i);
  }
  return String(in);
}

BlynkTimer timer;

BLYNK_WRITE(V7) // Get Data
{
  Blynk.virtualWrite(V0, 0);
  String value = param.asStr();  // Use param as usual.
  toUINT(value);
  Serial.println(value);
  Blynk.virtualWrite(V0, 1);
}

BLYNK_WRITE(V6) // Send IR code
{
  Blynk.virtualWrite(V0, 1);
  digitalWrite(LED_BUILTIN, 1);
  freq = param.asInt();
  Serial.println("IR Code Sent");
  irsend.sendRaw(IRCode,number_raw,freq);
  digitalWrite(LED_BUILTIN, 0);
  delay(10);
  // Update state
  Blynk.virtualWrite(V0, 1);    
  }


BLYNK_WRITE(V5)// receive IR Code
{
  Blynk.virtualWrite(V0, 0);
  irrecv.enableIRIn();  // Start the receiver
  delay(500);
  Serial.println("Push the button");
  int value = param.asInt();
  digitalWrite(White_LED, 1);
  while(irrecv.decode(&results)!= true);
  digitalWrite(White_LED, 0);
  Serial.println("IR Code Received");
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
    // Display the tolerance percentage if it has been change from the default.
    if (kTolerancePercentage != kTolerance)
      Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
    #if LEGACY_TIMING_INFO
    // Output legacy RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    yield();  // Feed the WDT (again)
    #endif  // LEGACY_TIMING_INFO
    // Output the results as source code
    Serial.println(resultToSourceCode(&results));
    yield();             // Feed the WDT (again)

    String A = resultToSourceCode(&results);
    String B = absolute(A);
    B.replace(" ","");
    connecter(B);
    delay(500);
    irrecv.disableIRIn();
    Blynk.virtualWrite(V0, 1);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
}

void setup()
{
  pinMode(Red_LED, OUTPUT);
  pinMode(Green_LED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(White_LED, OUTPUT);
  pinMode(Buttonn, INPUT_PULLUP); 
  // Debug console
  irsend.begin();
  #if ESP8266
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  #else  // ESP8266
  Serial.begin(115200, SERIAL_8N1);
  #endif  // ESP8266
  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);

  assert(irutils::lowLevelSanityCheck() == 0);
  
  #if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
  #endif  // DECODE_HASH
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.

     if (!EEPROM.begin(EEPROM_SIZE))  
   { //Init EEPROM
    Serial.println("failed to init EEPROM");
    delay(1000);
   }
  else
  {
    newssid = readStringFromFlash(0); // Read SSID stored at address 0
    Serial.print("SSID = ");
    Serial.println(newssid);
    newpassword = readStringFromFlash(50); // Read Password stored at address 40
    Serial.print("psss = ");
    Serial.println(newpassword);
  }
}

void loop()
{
  if (digitalRead(Buttonn) == LOW)
  {
    uint8_t a=0;
    Serial.println("Reset Button pushed");
    while(digitalRead(Buttonn) == LOW)
    {
      delay(500);
      a++;
      if(a > 5)
        break;
      Serial.print(".");
    }
    if (a > 5)
        {
          digitalWrite(Green_LED, 1);
          digitalWrite(Red_LED, 1);
          Serial.println("Initialize SSID & Pass with IP");
          wifi_init = 0;
          newssid = "NAN";
          newpassword = "NAN";
          WiFi.disconnect();
          delay(100);
          Serial.println("Configuring access point...");
           // You can remove the password parameter if you want the AP to be open.
           WiFi.softAP(ssid, password);
           IPAddress myIP = WiFi.softAPIP();
           Serial.print("AP IP address: ");
           Serial.println(myIP);
           server.begin();
           delay(500);
           Serial.println("Server started");

        }
  }
  
  if ((newssid != "NAN") and (newpassword != "NAN") and (wifi_init == 1))
  {
   digitalWrite(Green_LED, 0);
   digitalWrite(Red_LED, 1);
   uint8_t a=0;
  newssid.toCharArray(ssid1,50); 
  newpassword.toCharArray(pass1,50);
  WiFi.begin(ssid1, pass1);
  while (WiFi.status() != WL_CONNECTED) 
  {
        delay(500);
        Serial.print(a);
        a++;
        if (digitalRead(Buttonn) == LOW)
          break;
        if (a > 100)
        {
          Serial.println("Failed to connect");
          wifi_init = 0;
          break;
        }      
  }
  WiFi.disconnect();
  if((a < 100) and (digitalRead(Buttonn) != LOW))
  {
  Blynk.begin(auth, ssid1, pass1);
  if ((newssid != readStringFromFlash(0)) or (newpassword != readStringFromFlash(50)))
  {
      Serial.println("Store SSID & PSS in Flash");
      writeStringToFlash(newssid.c_str(), 0); // storing ssid at address 0
      writeStringToFlash(newpassword.c_str(), 50); // storing pss at address 40
      Serial.print("SSID = ");
      Serial.println(newssid);
      Serial.print("psss = ");
      Serial.println(newpassword);
  }
      digitalWrite(Green_LED, 1);
      digitalWrite(Red_LED, 0);  
  }
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
   wifi_init = 0;
  }

  while((newssid == "NAN") and (newpassword == "NAN"))
  {
      if (digitalRead(Buttonn) == LOW)
  {
    Serial.println("nazan");
  }
      WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println(texterr);
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
        Serial.println();        
        //Serial.println(currentLine);
        getSSIDPasword(currentLine);
        //Serial.println(newpassword);
        //Serial.println(newssid);
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
 
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
      wifi_init = 1;
  }

  if ((WiFi.status() != WL_CONNECTED) and (wifi_init == 0) and (digitalRead(Buttonn) != LOW))
  {
    digitalWrite(Green_LED, 0);
    digitalWrite(Red_LED, 1);
    wifi_init = 2;
  }
  
  if ((wifi_init == 2) and (digitalRead(Buttonn) != LOW))
  {
    uint8_t b = 1;
    while(WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid1, pass1);
      delay(1000);
      if (digitalRead(Buttonn) == LOW)
        b = 0;
        break;
      delay(1000);
      if (digitalRead(Buttonn) == LOW)
        b = 0;
        break;
      delay(1000);
      Serial.print(".");
    }

    if(WiFi.status() == WL_CONNECTED)
    {
    WiFi.disconnect();
    delay(500);
    Blynk.begin(auth, ssid1, pass1);
    wifi_init = 0;
    digitalWrite(Red_LED, 0);
    digitalWrite(Green_LED, 1);
    }
  }
  
  Blynk.run();
  timer.run();
}
