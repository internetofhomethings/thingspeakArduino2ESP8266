
<h2><strong>Read DS18B20 Temp Sensors and log to SD Card and ThingSpeak</strong></h2>

The Arduino sketch is dependant on an ESP8266-12 interfacing with
the Arduino as follows:

Arduino Pin 10 jumpered to ESP8266 Serial Transmitt (Tx)
Arduino Pin 11 jumpered to ESP8266 Serial Receive (Rx)
Arduino Pin Gnd connected to ESP8266 Gnd

It is also essential that the ESP8266 firmware is installed from the 
file "v0.9.2.2 AT Firmware.bin" included in this repository. The flashing 
program "esp8266_flasher.exe" is also included in this repository.

This firmware ensures the ESP8266 serial interface operates at 9600 baud. 
Higher baud rates to not work reliably with the Arduino Software Serial 
interface.

Compile and run the sketch "Temp2ThingsSpeak256.ino" to run this demo application
with the ESP8266 running. Before running, the WIFI parameters in the sketch need
to be updated with your local WIFI settings:

__#define APIKEY    "ZIIGPGZ3WSLHAZ50"    //ThingSpeak API WRITE KEY<br>
__#define SSID      "WIFISSID"            //WIFI SSID<br>
__#define PASSWORD  "WIFIPASSWORD"        //WIFI Password

The APIKEY provided is public for this demo application. However, your
own APIKEY should be entered once the demo application has been tested.

To view the ThingSpeak channel data, navigate a web browser to:

https://thingspeak.com/channels/56640

Note: The Arduino core and Software Serial library defaults to a 64 byte buffer 
for transmit and receive. In order to complete the ThingSpeak update in 1 
transaction, the buffer size was increased to 256 bytes. This change requires:

1. Change the <arduino base folder>\hardware\arduino\avr\libraries\SoftwareSerial\SoftwareSerial.h file.

Change:

__#define _SS_MAX_RX_BUFF 64 // RX buffer size

To:

__#define _SS_MAX_RX_BUFF 256 // RX buffer size

2. Change the <arduino base folder>\hardware\arduino\avr\cores\arduino\HardwareSerial.h file.

From:

__#define SERIAL_TX_BUFFER_SIZE 64<br>
__#define SERIAL_RX_BUFFER_SIZE 64

To:

__#define SERIAL_TX_BUFFER_SIZE 256<br>
__#define SERIAL_RX_BUFFER_SIZE 256
