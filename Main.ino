/*
// /////////////////////////////////////////////////////////////////////// COPYRIGHT NOTICE
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with Pip-Project.  If not, see <http://www.gnu.org/licenses/>.
// /////////////////////////////////////////////////////////////////////// COPYRIGHT NOTICE
// /////////////////////////////////////////////////////////////////////// VERSION CONTROL
// PAGE CREATED BY: Phillip Kraguljac
// PAGE CREATED DATE: 2019-09-19
// DATE       || NAME           || MODIFICATION
// 2019-09-19   || Phillip Kraguljac    || Created.
// /////////////////////////////////////////////////////////////////////// VERSION CONTROL
*/

/* STILL UNDER CONSTRUCTION */

#include <LiquidCrystal.h>
#include <Servo.h>

// /////////////////////////////////////////////////////////////////////// USER DEFINABLE VARIABLES
int Startup_Delay = 60;                                                             // Cycle time allocated for start-up.
int Waiting_Delay = 200;                                                            // Cycle time allocated for start-up.
int Serial_Output_Delay = 60;                                                       // .
int Override_Switch = 53;


// /////////////////////////////////////////////////////////////////////// SYSTEM VARIABLES
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;                           // Required for LCD.
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                                          // Required for LCD.
bool Override_Flag = false;                                                         // Flag used for triggering system override.
int Mode_Time_Indexer = 0;                                                          // Index used for mode cycle timers.
String Mode = "START-UP";                                                           // Mode Indexer.
String Upper_LCD_Line = "";
String Lower_LCD_Line = "";

Servo myservo;  // create servo object to control a servo
int pos = 20;    // variable to store the servo position



void setup() {
  
Serial.begin (9600);
pinMode(LED_BUILTIN, OUTPUT);                                                       // Declare IO's.
lcd.begin(16, 2);
lcd.clear();                                                                        // Reset LCD screen.
//Mode_Restart();                                                                   // Complete Mode reset.  

  myservo.attach(26);  // attaches the servo on pin 9 to the servo object

}


void loop() {
Override_Flag = digitalRead(Override_Switch);

// /////////////////////////////////////////////////////////////////////// MODE(S)

// [MODE] => "START-UP"
if(Mode=="START-UP"){                                                               // Start-up mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
Upper_LCD_Line = "Mode: "+Mode;
Lower_LCD_Line = Progress_Bar(Startup_Delay, Mode_Time_Indexer);
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);
if(Mode_Time_Indexer > Startup_Delay){Mode = "MEASURING"; Mode_Restart();}            // Switch mode once complete.
}


// [MODE] => "MEASURING"
if(Mode=="MEASURING"){                                                                // Waiting for pallet mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
//Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
Upper_LCD_Line = "Mode: "+Mode;
Lower_LCD_Line = Progress_Bar(Waiting_Delay, Mode_Time_Indexer);
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);
//

 for (pos = 20; pos <= 160; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }

if(pos >= 160){Mode = "WAITING"; Mode_Restart();}        // Switch mode once complete.

}  









// [MODE] => "WAITING"
if(Mode=="WAITING"){                                                                // Waiting for pallet mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
Upper_LCD_Line = "Mode: "+Mode;
Lower_LCD_Line = Progress_Bar(Waiting_Delay, Mode_Time_Indexer);
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);
if(Mode_Time_Indexer > Waiting_Delay){Mode = "MEASURING"; Mode_Restart();}        // Switch mode once complete.
}  


// [MODE] => "SENDING SERIAL"
if(Mode=="SEND SERIAL"){                                                            // Waiting for pallet mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
if(Mode_Time_Indexer==1){General_Serial_Output();}                                  // Provide 3rd party feedback via serial (if requried).
Upper_LCD_Line = "Mode: "+Mode;
Lower_LCD_Line = Progress_Bar(Serial_Output_Delay, Mode_Time_Indexer);
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);
if(Mode_Time_Indexer > Serial_Output_Delay){Mode = "WAITING"; Mode_Restart();}      // Switch mode once complete.
}  


delay(25);                                                                          // Base cycle throttle. [Do not remove!]
}


// ///////////////////////////////////////////////////////////////// FUNCTION(S)

// [FUNCTION] => OBTAING DISTANCE FROM ULTRASONIC SENSORS (RAW PULSES)
long Scan_Sensor(int Sensor_Trigger, int Sensor_Echo){
long Sensor_Duration = 0;                                                           // Value retrieved from sensor.
long Sensor_mm = 0;                                                                 // Converted measurement.
digitalWrite(Sensor_Trigger, LOW);                                                  // Set output LOW.
delayMicroseconds(5);                                                               // Wait clearance time.
digitalWrite(Sensor_Trigger, HIGH);                                                 // Set output HIGH.
delayMicroseconds(10);                                                              // Sending input trigger.
digitalWrite(Sensor_Trigger, LOW);                                                  // Set output LOW.
pinMode(Sensor_Echo, INPUT);
Sensor_Duration =  pulseIn(Sensor_Echo, HIGH);                                      // Retrieve HCSR04 feedback.
Sensor_mm = (Sensor_Duration/2)*0.303;                                              // Convert to mm.
return Sensor_mm;                                                                   // Return value.
}


// [FUNCTION] => Progress Bar Text Creation.
String Progress_Bar(int Set_Value, int Counter){
String Output_String = "";
long Available_Segments = 16;
float Real_Progress_Ratio = float(Counter) / float(Set_Value);
int Artificial_Progress = int(Real_Progress_Ratio*Available_Segments);
int i;
for (i = 0; i < Artificial_Progress; i++){
Output_String += ".";  
}
return Output_String;  
}


// [FUNCTION] => Determine if value is within margin.
bool Within_Margin(int Measurement, int Set_Distance, int Margin){
bool Output_String = false;
if(Measurement>(Set_Distance-Margin)&&Measurement<(Set_Distance+Margin)){
Output_String = true;  
}
  return Output_String; 
}


// [FUNCTION] => General Serial Output
void General_Serial_Output(){
Serial.print(":");
Serial.print("1");
Serial.print(":");
Serial.print("2");
Serial.print(":");
Serial.print("3");
Serial.print(":");
Serial.println("4");
}


// [FUNCTION] => General LCD Output
void General_LCD_Output(String Upper_LCD_Line, String Lower_LCD_Line){
//lcd.clear();
lcd.setCursor(0, 0);                                                                // Set LCD cursor.
lcd.print(Upper_LCD_Line);                                                          // .
lcd.setCursor(0, 1);                                                                // Set LCD cursor.
lcd.print(Lower_LCD_Line);                                                          // .
}


// [FUNCTION] => Mode Restart
void Mode_Restart(){
Mode_Time_Indexer = 0; // Reset mode indexer.
lcd.clear();
//lcd.setCursor(0, 0);
//lcd.print("Mode: ");
//lcd.setCursor(6, 0);
//lcd.print(Mode);
//Serial.print("Mode: " );
//Serial.println(Mode);
}
