#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WiegandNG.h>
#include <ESP8266_Lib.h>
#include "WiFiEsp.h"

#define W5100_CS  10
#define SDCARD_CS 4
#define ESP8266_Serial Serial3
#define ESP8266_Serial_BNDRT 115200

ESP8266 wifi(&ESP8266_Serial);
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "www.unicrise.com";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient ethernet_client;
// Initialize the Ethernet client object
WiFiEspClient wifi_client;
String target;
/*
char* place = (char*) malloc(64 * sizeof(char));
char* ssid = (char*) malloc(64 * sizeof(char));
char* pass = (char*) malloc(64 * sizeof(char));
*/
char* place = "Üsküdar";
char* ssid = "1.KAT";
char* pass = "1234qwer";


  const char * DEVICE_LABEL = "wiznet-shield"; // Ubidots Device Label
  const char * USER_AGENT = "wiznet";
  const char * VERSION = "1.0";
  const int PORT = 80;
  
// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement
WiegandNG wg;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  pinMode(SDCARD_CS, OUTPUT); 
  digitalWrite(SDCARD_CS, LOW); 
    Serial.println("Initializing SD card...");
    if (!SD.begin(SDCARD_CS)) {
        Serial.println("ERROR - SD card initialization failed!");
    }
    else{
       Serial.println("SUCCESS - SD card initialized.");
        readFromFile("ssid.txt", ssid);
        readFromFile("pass.txt", pass);
        readFromFile("branch.txt", place);
    }
   
  
  ESP8266_Serial.begin(ESP8266_Serial_BNDRT);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  unsigned int pinD0 = 2;
  unsigned int pinD1 = 3;
  unsigned int wiegandbits = 32;
  unsigned int packetGap = 15;   

  digitalWrite(SDCARD_CS, HIGH); // Deselect the SD card
  
  if(!wg.begin(pinD0, pinD1, wiegandbits, packetGap)) {
    Serial.println("Out of memory!");
  }
  InitializeEthernet();
  InitializeWifi();


}

void checkClientDisconnected(){
   // if the server's disconnected, stop the client:
  if (!ethernet_client.connected()) {
    endMicros = micros();
    Serial.println();
    Serial.println("disconnecting.");
    ethernet_client.stop();

    float seconds = (float)(endMicros - beginMicros) / 1000000.0;
    Serial.print(seconds, 4);
    float rate = (float)byteCount / seconds / 1000.0;

  }
}

void InitializeWifi(){
    Serial.println("Initialize Wifi with this credentials:");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("PASS: ");
    Serial.println(pass);
    if(wifi.joinAP(ssid, pass)){
          Serial.println("Status: Connected ");
          Serial.print("Wifi IP Adress: ");
          Serial.println(wifi.getLocalIP());
          delay(500);
          
          Serial.println("Trying connect to server with Wifi");
         if (wifi.createTCP(server, 80)) {
          Serial.println("Connected to server");
          
          char JSON[200];
          char *branchCharArray = place;
          char key[] = "12312";
          
          sprintf(JSON, "?place=%s&key=%s\0", branchCharArray, key);
          Serial.println(JSON);
    
          Serial.println("Begin POST Request");
          /* Build the body to be POST */
          char* body = (char *) malloc(sizeof(char) * 100);
          sprintf(body, "{\"place\":%s,\"key\": %s}", branchCharArray, key);
          Serial.println(JSON);
  
        
          /* Builds the HTTP request to be POST */
          char* data = (char *) malloc(sizeof(char) * 300);
          sprintf(data, "POST /meskenpdks/public/device/log/add%s",JSON);
          sprintf(data, "%s HTTP/1.1\r\n", data);
          sprintf(data, "%sHost: %s\r\n", data, "unicrise.com");
          sprintf(data, "%sUser-Agent: %s/%s\r\n", data, USER_AGENT, VERSION);
          sprintf(data, "%sConnection: close\r\n", data);
          sprintf(data, "%sContent-Type: application/json\r\n", data);
          sprintf(data, "%sContent-Length: %d\r\n\r\n", data, dataLen(body)); 
          sprintf(data, "%s%s\r\n\r\n", data, body);
  
          Serial.println(wifi.send((const uint8_t*)data, strlen(data)));
          Serial.println(data);

            if (wifi.releaseTCP()) {
                Serial.print("release tcp ok\r\n");
            } else {
                Serial.print("release tcp err\r\n");
            }
        }

         
        else{
           Serial.println("Can't Connected to the server with Wifi ");

        }

    }
    else{
          Serial.println("Status: Can't Connected ");
    }

}

void InitializeEthernet(){
    // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
     Serial.println("Ethernet can't connected");
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  beginMicros = micros();
}



void postCardInfo(char* &key){
     
     Serial.println("postCardInfo called");

       const char* branchCharArray = place;
       String branchString = String(branchCharArray);
       String keyString = String(key);
        char JSON[200];
        sprintf(JSON, "?place=%s&key=%s\0", branchCharArray, key);
        Serial.println(JSON);
  
        Serial.println("Begin POST Request");
        /* Build the body to be POST */
        char* body = (char *) malloc(sizeof(char) * 100);
        sprintf(body, "{\"place\":%s,\"key\": %s}", branchCharArray, key);
        Serial.println(JSON);

      
        /* Builds the HTTP request to be POST */
        char* data = (char *) malloc(sizeof(char) * 300);
        sprintf(data, "POST /meskenpdks/public/device/log/add%s",JSON);
        sprintf(data, "%s HTTP/1.1\r\n", data);
        sprintf(data, "%sHost: %s\r\n", data, "unicrise.com");
        sprintf(data, "%sUser-Agent: %s/%s\r\n", data, USER_AGENT, VERSION);
        sprintf(data, "%sConnection: close\r\n", data);
        sprintf(data, "%sContent-Type: application/json\r\n", data);
        sprintf(data, "%sContent-Length: %d\r\n\r\n", data, dataLen(body)); 
        sprintf(data, "%s%s\r\n\r\n", data, body);

        if (ethernet_client.connect(server, PORT)) {
            ethernet_client.print(data);
          } else {
            Serial.println("connection failed");
          }
  
        while(ethernet_client.available()){
     
          String line = ethernet_client.readStringUntil('\r');
          Serial.println(line);
         
        }


        free(body);
        free(data);
        branchCharArray= 0;

      Serial.println("Successfully Ended");

}

int dataLen(char* variable) {
  uint8_t dataLen = 0;
  for (int i = 0; i <= 250; i++) {
    if (variable[i] != '\0') {
      dataLen++;
    } else {
      break;
    }
  }
  return dataLen;
}
void ethernetLoop(){
    // if there are incoming bytes available
  // from the server, read them and print them:
  int len = ethernet_client.available();
  if (len > 0) {
    byte buffer[80];
    if (len > 80) len = 80;
    ethernet_client.read(buffer, len);
    if (printWebData) {
      Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    }
    byteCount = byteCount + len;
  }
}


void readFromFile(String filename, char *&willBeReplacedArray){
  File printFile;
  String buffer;
  int stringIndex = 0;
  printFile = SD.open(filename);

  Serial.println("Trying to open file");
  if (printFile) {
    Serial.println("SD.open");
    while (printFile.available()) {
        Serial.println("SD.while");
        char temp = printFile.read();
        Serial.write(temp);
        if(temp != '\n'){
          willBeReplacedArray[stringIndex] = temp;
          stringIndex++;
          willBeReplacedArray[stringIndex] = '\0';
        }
    }
  }
}

void GetBinary(WiegandNG &tempwg, char *&willBeReplacedArray) {
  volatile unsigned char *buffer=tempwg.getRawData();
  unsigned int bufferSize = tempwg.getBufferSize();
  unsigned int countedBits = tempwg.getBitCounted();


  unsigned int countedBytes = (countedBits/8);
  if ((countedBits % 8)>0) countedBytes++;
  // unsigned int bitsUsed = countedBytes * 8;
  int loopCount = 0;
  for (unsigned int i=bufferSize-countedBytes; i< bufferSize;i++) {
    unsigned char bufByte=buffer[i];
    for(int x=0; x<8;x++) {
      if ( (((bufferSize-i) *8)-x) <= countedBits) {
        if((bufByte & 0x80)) {
          willBeReplacedArray[loopCount] = '1';
        }
        else {
          willBeReplacedArray[loopCount] = '0';
        }
        loopCount++;
      }
      bufByte<<=1;
    }
  }
  
  willBeReplacedArray[loopCount+1] = 0;
  Serial.println();
}
char* cardKey = (char*) malloc(64 * sizeof(char));

void loop() {
   ethernetLoop();
   if(wg.available()) {
    wg.pause();             // pause Wiegand pin interrupts
    GetBinary(wg, cardKey);   // display raw data in binary form, raw data inclusive of PARITY
    wg.clear();   
    postCardInfo(cardKey);
    *cardKey = 0;

    // compulsory to call clear() to enable interrupts for subsequent data
    
  }
}
