
/* YourDuino.com Example: BlueTooth HC-05 Setup
 - WHAT IT DOES: 
   - Sets "Key" pin on HC-05 HIGH to enable command mode
   - THEN applies Vcc from 2 Arduino pins to start command mode
   - SHOULD see the HC-05 LED Blink SLOWLY: 2 seconds ON/OFF 
 
 Sends, Receives AT commands
   For Setup of HC-05 type BlueTooth Module
   NOTE: Set Serial Monitor to 'Both NL & CR' and '9600 Baud' at bottom right   
 - SEE the comments after "//" on each line below
 - CONNECTIONS:
   - GND
   - Pin 2 to HC-05 TXD
   - Pin 3 to HC-05 RXD
   - Pin 4 to HC-05 KEY
   - Pin 5+6 to HC-05 VCC for power control
 - V1.02 05/02/2015
   Questions: terry@yourduino.com */

/*-----( Import needed libraries )-----*/
#include <SoftwareSerial.h>  
#include <SFE_BMP180.h>

/*-----( Declare Constants and Pin Numbers )-----*/
#define HC_05_TXD_ARDUINO_RXD 2
#define HC_05_RXD_ARDUINO_TXD 3
#define HC_05_SETUPKEY        4
#define HC_05_PWR1            5  // Connect in parallel to HC-05 VCC
#define HC_05_PWR2            6  // Connect in parallel to HC-05 VCC
#define CMD_W                 1100 // msec

#define msg(x) Serial.println(x)

/*-----( Declare objects )-----*/
SoftwareSerial BTSerial(HC_05_TXD_ARDUINO_RXD, HC_05_RXD_ARDUINO_TXD); // RX | TX
/*-----( Declare Variables )-----*/
//NONE

enum BTDeviceState {
  error = -2,
  off = -1,
  data = 0,
  at = 1,
};

class BTDevice 
{
private:
  BTDeviceState m_state;
  
public:
  BTDevice(): m_state(off) {  };

  void setState(BTDeviceState state) {
    if (m_state == state) {
      msg("try to set BT to the same state " + state);
      
      return;
    }
    switch (state) {
      case off: {
        
        msg("Power off");
        digitalWrite(HC_05_PWR1, LOW);
        digitalWrite(HC_05_PWR2, LOW);  
        delay(CMD_W);
        m_state = off;
        return;        
      }
      case data: {
        if (m_state == off) {
          digitalWrite(HC_05_SETUPKEY, LOW);  // Set command mode when powering up
          delay(CMD_W);       
          digitalWrite(HC_05_PWR1, HIGH);
          digitalWrite(HC_05_PWR2, HIGH);  
          delay(CMD_W);
                    
          m_state = data;
          return;        
        }
      }
      case at: {
        if (m_state == off) {
          digitalWrite(HC_05_SETUPKEY, HIGH);  // Set command mode when powering up
          delay(CMD_W);       
          digitalWrite(HC_05_PWR1, HIGH);
          digitalWrite(HC_05_PWR2, HIGH);  
          delay(CMD_W);        

          m_state = at;
          return;
        }
        
      }
      
    }
  };

  BTDeviceState state() {return m_state;};
  
};

/*
void BTDevice::initPins() {
  pinMode(HC_05_SETUPKEY, OUTPUT);  // this pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  pinMode(HC_05_PWR1, OUTPUT);      // Connect in parallel to HC-05 VCC
  pinMode(HC_05_PWR2, OUTPUT);      // Connect in parallel to HC-05 VCC
};
*/

BTDevice *bt = new BTDevice();

SFE_BMP180 pressure;

void setup()   /****** SETUP: RUNS ONCE ******/
{

  pinMode(HC_05_SETUPKEY, OUTPUT);  // this pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  pinMode(HC_05_PWR1, OUTPUT);      // Connect in parallel to HC-05 VCC
  pinMode(HC_05_PWR2, OUTPUT);      // Connect in parallel to HC-05 VCC

  pinMode(10, INPUT);
  pinMode(8, INPUT);
  
//  digitalWrite(HC_05_SETUPKEY, HIGH);  // Set command mode when powering up
  
  Serial.begin(9600);   // For the Arduino IDE Serial Monitor
  BTSerial.begin(38400);  // HC-05 default speed in AT command mode

  bt->setState(data);

  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
    Serial.println("BMP180 init failed!!!");

/*
  msg("Pwr on");
  delay(2000);
  
  msg("Applying VCC Power. LED should blink SLOWLY: 2 Seconds ON/OFF");
  
  digitalWrite(HC_05_PWR1, HIGH); // Power VCC
  digitalWrite(HC_05_PWR2, HIGH);  
  
  delay(2000);

  Serial.println("Enter AT commands in top window.");
  BTSerial.begin(38400);  // HC-05 default speed in AT command mode
*/

}

int cnt = 0;
int board = 0;

const int aInPin = A0;
int analogPin = 3;
     
int v0 = 0;           
int v1 = 0;           
int v2 = 0;           
int v3 = 0;
int d10 = 0;           
int d8 = 0;           
double T = -10.0;
double P = -10.0;


void loop()
{
  int btIn = -1;

  if (BTSerial.available() && (btIn = BTSerial.read()) ) {
    Serial.write(btIn);
  }

  if (Serial.available()) {
    char inChar = (char)Serial.read();

    if (inChar == '*') {
      bt->setState(off);
      bt->setState(data);
    }
    if (inChar == '?') {
      bt->setState(off);
      bt->setState(at);
    }
    
    String fromSerialStr = String(inChar);
    
    BTSerial.write(fromSerialStr.c_str());
    Serial.write(fromSerialStr.c_str());
    //BTSerial.print(fromSerialStr);
    //Serial.print(fromSerialStr);
  }


  v0 = analogRead(A0);
  v1 = analogRead(A1);
  v2 = analogRead(A2);
  v3 = analogRead(A3);

  d10 = digitalRead(10);
  d8 = digitalRead(8);

  char status;

  // abirvalg!!!
  
  status = pressure.startTemperature();
  if (status != 0)  {
    delay(status);
    status = pressure.getTemperature(T);
    Serial.println(T);
    if (status != 0) {
      status = pressure.startPressure(3);
      if (status != 0) {
        delay(status);

        status = pressure.getPressure(P,T);
        if (status != 0) {

        } else { T = -1.0; P = -1.0; }
      } else { T = -1.0; P = -1.0; }
    } else { T = -1.0; P = -1.0; }
  } else { T = -1.0; P = -1.0; }


  if (board == 0) {
    char out[80];
    //sprintf(out, "seqid %d;%d;;;\r\n", cnt++, val); // 0.7501
    sprintf(out, "seqid=%d;A0=%d;A1=%d;D8=%d;D10=%d;T=%d;P=%d\r\n", cnt, v0, v1, d8, d10, (int)T, (int)(P*0.7501));
    Serial.print(out); 
    BTSerial.write(out);
    delay(100);
  }
  cnt++;
}


