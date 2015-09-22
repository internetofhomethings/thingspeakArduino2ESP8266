
#include <SoftwareSerial.h>
#include <stdlib.h>


///////////////////////////////////////////////////////////////
//
//  This demo program updates a ThingSpeak channel with 
//  3 DS18B20 Sensor readings. An ESP8266 WIFI interface is
//  used to connect with the ThingSpeak channel.
//
///////////////////////////////////////////////////////////////



#define SERBAUD 115200

#define APIKEY    "ZIIGPGZ3WSLHAZ50"    //ThingSpeak API WRITE KEY
//#define SSID      "WIFISSID"            //WIFI SSID
//#define PASSWORD  "WIFIPASSWORD"        //WIFI Password
#define SSID      "dsa"            //WIFI SSID
#define PASSWORD  "1957090922"        //WIFI Password

//Local Functions
int SendCmd(String Cmd, String Exp, int del); 
void connecttoWifi(void);
void serialTx(String Cmd);
int uploadAlltoThingspeak(String rtctag,String class1,String class2,String class3,String outside);

SoftwareSerial WifiInterface(10, 11); // RX, TX

int icnt=0;
String toSend;
String toSend2;
float temp1=25;
int ifld=0;
String t[5]; 
char cval[8];
String timetag;
unsigned long starttime;

//////////////////////////////////////////////////////
// Setup function 
//////////////////////////////////////////////////////
void setup() {
    // Begin serial communication.
    Serial.begin(SERBAUD);
    // Begin Software Serial
    WifiInterface.begin(9600);
    delay(1000); 
    //Connect to Wifi 
    connecttoWifi();
}

//////////////////////////////////////////////////////
// Sketch loop() function: 
// Case Action
// ---- ----------------------------------------------
//   1. Start 15 second ThingSpeak update timer
//   2. Read Temperature Sensor 1 - Classroom 1
//   3. Read Temperature Sensor 2 - Classroom 2 
//   4. Read Temperature Sensor 3 - Classroom 3 
//   5. Read Temperature Sensor 4 - Outside 
//   6. Save latest Temperature readings to SD Card
//   7. Wait until 15 seconds lapsed since last ThingSpeak update
//   8. Save all temperature reading to ThingSpeak
//   9. Do it again (return to case 1)
//////////////////////////////////////////////////////
void loop() {
    int i,itry;
    switch(++ifld) {
        case 1:
            //get current arduino time (we have 15 seconds to complete everything-max ThingSpeak update rate)
            starttime = millis();
            break;
        case 2:  // Read Temperature Sensor 1: Classroom 1
        case 3:  // Read Temperature Sensor 2: Classroom 2
        case 4:  // Read Temperature Sensor 3: Classroom 3
        case 5:  // Read Temperature Sensor 4: Outside
            //Read DS18B20 Temperature Sensor 1,2,3 or 4
            if(temp1>27) temp1 = 14;  //A fake value for temperature for test purposes
            //t[ifld-1] = temp1;            //Save Temperature Reading
            dtostrf(temp1,4,1,cval);            //Save Temperature Reading
            t[ifld-1] = String(cval);
            //dtostrf(floatVar, minStringWidthIncDecimalPoint, numVarsAfterDecimal, charBuf);
            temp1 += 1.1;
            break;
        case 6:
            //Save latest Temperature readings  to SD Card
            //Get time using RTC here, save in "timetag" variable
            timetag = "12345678";
            /********************
            toSend = "timetag,";  //Add timetag to SD record
            for(i=1;i<5;i++) {    //Add latest 4 sensor readings to SD record
                toSend += ",";
                toSend += t(i); 
            }
            File dataFile = SD.open("templog.txt", FILE_WRITE);
            dataFile.println(toSend);
            dataFile.close();
            *********************/
            break;
        case 7:
            //Wait until 15 seconds elapsed since last update (max ThingSpeak Rate)
            while( (millis() - starttime)<15000 ) {
                delay(500);
            }
            break;
        case 8:
            //Save All temperature readings to ThingSpeak
            itry=0;
            while( (uploadAlltoThingspeak(timetag,t[1],t[2],t[3],t[4])<1)&&(itry++<3) ) { //Up to 3 tries allowed if save fails 
                connecttoWifi(); //Reconnect WIFI if lost...Why? Because last save oo ThingSpeak failed
            }
            break;
        case 9:
            //Start sequence over again
            ifld = 0;
            break;
    }
}
//////////////////////////////////////////////////////
// Send Command to ESP8266
// Notes:
// 1. Transmit FIFO 64 bytes (Max send length)
// 2. Cannot send all sensors to ThingSpeak at same time
// 3. Send 1 char at time commented out below, not working
//////////////////////////////////////////////////////
void serialTx(String Cmd) {
   /*
   int i;
   for(i=0;i<(Cmd.length()-2);i++) {
       WifiInterface.print(Cmd.charAt(i));
       Serial.print(Cmd.charAt(i));
       delay(10);
   }
   delay(100);
   //
   WifiInterface.print("\r\n"); 
   Serial.print("\r\n"); 
   */
   WifiInterface.println(Cmd); 
}
//////////////////////////////////////////////////////
// SendCmd() - Send string to ESP8266
// Cmd       - String to send
// Exp       - Expected response
// del       - Max delay allowed before response
// return    - "-1" if failed, 0 or greater if success
//////////////////////////////////////////////////////
int SendCmd(String Cmd, String Exp, int del) {
    String rcv="";
    int ms=0;
    serialTx(Cmd);
    //WifiInterface.println(Cmd); 
    delay(500);
    while ((WifiInterface.available()<1)&&(ms<del)) {
        delay(100);
        ms += 100;
    } 
    //Read ESP8266 Reply 
    while ((WifiInterface.available()>0)&&(rcv.indexOf(Exp) <0)) {
        rcv += char(WifiInterface.read());
    }
     WifiInterface.flush();
     Serial.println(rcv.indexOf(Exp));
     Serial.println(rcv);
    return (rcv.indexOf(Exp) >=0);
}
//////////////////////////////////////////////////////
// connecttoWifi()
//////////////////////////////////////////////////////
void connecttoWifi(void) {
    //Verify ESP8266 communication (OK returned from AT cmd)
    toSend = "Start #" + String(icnt++);
    Serial.println(toSend);
    SendCmd("AT+RST",".com",6000);          //Verify Communication
    SendCmd("AT","OK",100);                 //Verify Communication
    SendCmd("AT+CIPMUX=0","OK",100);        //Setting CWMODE
    toSend = "AT+CWJAP=";
    toSend += '"';
    toSend += SSID;
    toSend += '"';
    toSend += ",";
    toSend += '"';
    toSend += PASSWORD;
    toSend += '"';
    SendCmd(toSend,"OK",8000);        //Connect to Local WiFi
}


//////////////////////////////////////////////////////////////////
// uploadAlltoThingspeak()
// rtctag    - field1 String value of timetag
// class1    - field2 int value of classroom 1 temperature (C)
// class1    - field3 int value of classroom 2 temperature (C)
// class1    - field4 int value of classroom 3 temperature (C)
// outside   - field5 int value of outside temperature (C)
// return    - "-1" if failed, 0 or greater if success
//////////////////////////////////////////////////////////////////
int uploadAlltoThingspeak(String rtctag,String class1,String class2,String class3,String outside) {
    toSend = "AT+CIPSTART=";
    toSend += '"';
    toSend += "TCP";
    toSend += '"';
    toSend += ",";
    toSend += '"';
    toSend += "api.thingspeak.com";
    toSend += '"';
    toSend += ",80";
    toSend += '"';
    if(SendCmd(toSend,"Linked",2000)<1) return -1;                 //Connect to thingspeak
    //This is what we are going to send
    toSend = "GET /update?api_key=";
    toSend += APIKEY;
    toSend += "&field1=";
    toSend += rtctag;
    toSend += "&field2=";
    toSend += class1;
    toSend += "&field3=";
    toSend += class2;
    toSend += "&field4=";
    toSend += class3;
    toSend += "&field5=";
    toSend += outside;
    toSend += "\r\n\r\n";
    toSend2 = "AT+CIPSEND=";
    toSend2 += String(toSend.length()+2);
    if(SendCmd(toSend2,">",1000)<1) return -1;         //Id # bytes to send
    //SendCmd(toSend,"OK",1000);                  //Connect to thingspeak
    if(SendCmd(toSend,"+",2000)<1) return -1;                  //Connect to thingspeak
    return 1;          //Close connection
}

