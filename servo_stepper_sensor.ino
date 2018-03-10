/*******************************************************************************

 Bare Conductive Adafruit_MPR121 library
 ------------------------------
 
 DataStream.ino - prints capacitive sense data from Adafruit_MPR121 to serial port
 
 Based on code by Jim Lindblom and plenty of inspiration from the Freescale 
 Semiconductor datasheets and application notes.
 
 Bare Conductive code written by Stefan Dzisiewski-Smith and Peter Krige.
 
 This work is licensed under a MIT license https://opensource.org/licenses/MIT
 
 Copyright (c) 2016, Bare Conductive
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

*******************************************************************************/

// serial rate
#define baudRate 57600
#define MPR121addr 0x5A
#include <Servo.h>
#include <Adafruit_MPR121.h>
#include <Wire.h>

//Stepper motor pins - Unipolar stepper
#define IN1  9
#define IN2  10
#define IN3  11
#define IN4  12
Servo myservo;  // create servo object to control a servo

///////////////////////////////
int pos = 0; 
int Steps = 0;
int Direction = 1;
unsigned long last_time;
unsigned long currentMillis ;
int steps_left=4076;//4095 
long time;

int led = 11;

/*
******************************************
*/

// this is the touch threshold - setting it low makes it more like a proximity trigger
// default value is 40 for touch
const int touchThreshold = 40;
// this is the release threshold - must ALWAYS be smaller than the touch threshold
// default value is 20 for touch
const int releaseThreshold = 20;

void setup(){  
  myservo.attach(6); // attaches the servo on pin 9 to the servo object
  pinMode(led, OUTPUT);  
  
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(led, HIGH); 

  Wire.begin();
  Adafruit_MPR121.begin(MPR121addr);
  Serial.begin(baudRate);
  while(!Serial); // only needed for Arduino Leonardo or Bare Touch Board 

  // 0x5C is the Adafruit_MPR121 I2C address on the Bare Touch Board
  if(!Adafruit_MPR121.begin(0x5A)){ 
    Serial.println("error setting up Adafruit_MPR121");  
    switch(Adafruit_MPR121.getError()){
      case NO_ERROR:
        Serial.println("no error");
        break;  
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;      
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;      
    }
    while(1);
  }


  Adafruit_MPR121.setTouchThreshold(touchThreshold);
  Adafruit_MPR121.setReleaseThreshold(releaseThreshold);  
}

void loop(){
   readRawInputs();  
   for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  
  while(steps_left>0)
  {
    currentMillis = micros();
    if(currentMillis-last_time>=150)//higher number = slower speed
    {
      stepper(1); 
      delay(1);//could take out this delay??
      time=time+micros()-last_time;
      last_time=micros();
      Serial.println(steps_left);
      steps_left--;
    }
    }
  
  //Serial.println(time);
  Serial.println("Wait...!");
  delay(1000); // DELAY BETWEEN CHANGE IN STEPS
  Direction=-Direction;
  steps_left=4076;
  
}


void readRawInputs(){
    int i;
    
    if(Adafruit_MPR121.touchStatusChanged()) Adafruit_MPR121.updateTouchData();
    Adafruit_MPR121.updateBaselineData();
    Adafruit_MPR121.updateFilteredData();
    
    
    Serial.print("TOUCH: ");
    for(i=0; i<13; i++){          // 13 touch values
      Serial.print(Adafruit_MPR121.getTouchData(i), DEC);
      if(i<12) Serial.print(" ");
    }    
    Serial.println();   
    
    Serial.print("TTHS: ");
    for(i=0; i<13; i++){          // 13 touch thresholds
      Serial.print(touchThreshold, DEC); 
      if(i<12) Serial.print(" ");
    }   
    Serial.println();
    
    Serial.print("RTHS: ");
    for(i=0; i<13; i++){          // 13 release thresholds
      Serial.print(releaseThreshold, DEC); 
      if(i<12) Serial.print(" ");
    }   
    Serial.println();
    
    Serial.print("FDAT: ");
    for(i=0; i<13; i++){          // 13 filtered values
      Serial.print(Adafruit_MPR121.getFilteredData(i), DEC);
      if(i<12) Serial.print(" ");
    } 
    Serial.println();
    
    Serial.print("BVAL: ");
    for(i=0; i<13; i++){          // 13 baseline values
      Serial.print(Adafruit_MPR121.getBaselineData(i), DEC);
      if(i<12) Serial.print(" ");
    } 
    Serial.println();
    
    // the trigger and threshold values refer to the difference between
    // the filtered data and the running baseline - see p13 of 
    // http://www.freescale.com/files/sensors/doc/data_sheet/Adafruit_MPR121.pdf
    
    Serial.print("DIFF: ");
    for(i=0; i<13; i++){          // 13 value pairs
      Serial.print(Adafruit_MPR121.getBaselineData(i)-Adafruit_MPR121.getFilteredData(i), DEC);
      if(i<12) Serial.print(" ");
    }           
    Serial.println();

}

void stepper(int xw){
  for (int x=0;x<xw;x++){
switch(Steps){
   case 0:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   case 1:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, HIGH);
   break; 
   case 2:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 3:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 4:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 5:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
     case 6:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 7:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   default:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
}
SetDirection();
}
} 


void SetDirection(){
if(Direction==1){ Steps++;}
if(Direction==-1){ Steps--; }
if(Steps>7){Steps=0;}
if(Steps<0){Steps=7; }
}
