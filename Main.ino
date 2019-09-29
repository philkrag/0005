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
// PAGE CREATED DATE: 2019-07-20
// DATE       || NAME           || MODIFICATION
// 2019-07-20   || Phillip Kraguljac    || Created.
// 2019-09-28   || Phillip Kraguljac    || Version 1.1
// /////////////////////////////////////////////////////////////////////// VERSION CONTROL
*/

#include <LiquidCrystal.h>
#include <Servo.h>

// /////////////////////////////////////////////////////////////////////// USER DEFINABLE VARIABLES
int Startup_Delay = 60;                                                             // Cycle time allocated for start-up.
int Position_Delay = 100;                                                           // Time allowed for unit to settle in position.
int Measure_Delay = 10;                                                             // Time allowed for measuring the distance.
int Waiting_Delay = 400;                                                            // Cycle time allocated for start-up.
int Serial_Output_Delay = 60;                                                       // Time allowed for sending serial.
int Override_Switch = 53;                                                           // IO Input address.
int Upper_Position = 72;                                                            // Upper position for top of bus bar.
int Lower_Position = 104;                                                           // Lower position for bottom of bus bar.
int Scan_Speed = 10;                                                                // Time duration between samples.
int const Scan_Accuracy = 25;                                                       // Total samples taken for each measurement.
int Scan_Steps = 16;                                                                // Steps between scan positions.


// /////////////////////////////////////////////////////////////////////// SYSTEM VARIABLES
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;                           // Required for LCD.
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                                          // Required for LCD.
Servo Y_Axis_Servo;                                                                 // Requried for Servo.
bool Override_Flag = false;                                                         // Flag used for triggering system override.
int Mode_Time_Indexer = 0;                                                          // Index used for mode cycle timers.
String Mode = "START-UP";                                                           // Mode Indexer.
String Upper_LCD_Line = "";                                                         // LCD upper output string.
String Lower_LCD_Line = "";                                                         // LCD lower output string.
int Sensor_Trigger_A1 = 24;                                                         // Ultra sonic sensor trigger pin.
int Sensor_Echo_A1 = 22;                                                            // Unltra sonic sensor echo pin.
int Current_Sensor_Y_Axis_Position = Upper_Position;                                // Y_Axis sensor position.
long Temp_Measurement_Reading = 0;                                                  // Scan measurement memory.
int Scan_Memory[Scan_Accuracy];                                                     // Scan measurement memory collective.
int Scan_Indexer = 0;                                                               // Scan measurement memory indexer.
int Servo_Measuring_Position = Upper_Position;                                      // Max upper position of servo (reset position).
bool Scan_Completed = false;                                                        // Scan complete flag.


void setup() {

Serial.begin (9600);                                                                // Start serial @ BAUD.
pinMode(LED_BUILTIN, OUTPUT);                                                       // Declare IO's.
lcd.begin(16, 2);                                                                   // Set cursor for LCD screen output.
lcd.clear();                                                                        // Reset LCD screen.
pinMode(Sensor_Trigger_A1, OUTPUT);                                                 // Initiate US trigger pin.
pinMode(Sensor_Echo_A1, INPUT);                                                     // Initiate US echo pin.
Y_Axis_Servo.attach(26);                                                            // Inititae servo pin.

}


void loop() {
//Override_Flag = digitalRead(Override_Switch);                                     // Removed for this application.


// /////////////////////////////////////////////////////////////////////// MODE(S)

// [MODE] => "START-UP"
if(Mode=="START-UP"){                                                               // Start-up mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
Upper_LCD_Line = "Mode: "+Mode;                                                     // Set upper LCD line.
Lower_LCD_Line = Progress_Bar(Startup_Delay, Mode_Time_Indexer);                    // Set lower LCD Line.
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);                                 // Display LCD.
if(Mode_Time_Indexer > Startup_Delay){Mode = "POSITION"; Mode_Restart();}           // Switch mode once complete.
}


// [MODE] => "POSITIONING"
if(Mode=="POSITION"){                                                               // Waiting for pallet mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
Upper_LCD_Line = "Mode: "+Mode;                                                     // Set upper LCD line.
Lower_LCD_Line = Progress_Bar(Position_Delay, Mode_Time_Indexer);                   // Set lower LCD Line.
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);                                 // Display LCD.
if(Current_Sensor_Y_Axis_Position>Lower_Position){Current_Sensor_Y_Axis_Position=Upper_Position;}       // Reeset posistion to highest location @ end.
Y_Axis_Servo.write(Current_Sensor_Y_Axis_Position);                                 // tell servo to go to position in variable 'pos'
if(Mode_Time_Indexer==1){ }
if(Mode_Time_Indexer > Position_Delay){Mode = "MEASURE"; Mode_Restart();}           // Switch mode once complete.
}


if(Mode=="MEASURE"){                                                                // Waiting for pallet mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
Upper_LCD_Line = "Position: "+String(Current_Sensor_Y_Axis_Position);               // Set upper LCD line.
Lower_LCD_Line = "=> ";                                                             // Set lower LCD Line.
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);                                 // Display LCD.
delay(Scan_Speed);                                                                  // Delay between scan cycles.
if(Current_Sensor_Y_Axis_Position==Upper_Position){                                 // Set complete scan flag.
Scan_Completed=true;                                                                // Trigger scan complete flag.
}else{
Accurate_Scan_Sensor(Sensor_Trigger_A1, Sensor_Echo_A1, Scan_Accuracy);             // Complete sensor scan procedure.
}
if(Scan_Completed = true && Mode_Time_Indexer > Measure_Delay){Mode = "DISPLAY"; Mode_Restart();}        // Switch mode once complete.
}  


// /////////////////////////////////////  DEBUGGING - CONSTANT MEASUREMENT OPTION - BELOW
if(Mode=="TEST"){                                                                   // Waiting for pallet mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
delay(Scan_Speed);                                                                  // Waits for the servo to reach the position
Accurate_Scan_Sensor(Sensor_Trigger_A1, Sensor_Echo_A1, Scan_Accuracy);             // Complete sensor scan procedure. 
Review_Measurement_Array(Scan_Memory);                                              // Review scan samples.
if(Current_Sensor_Y_Axis_Position==Upper_Position){                                 // Reset scan servo position.
Upper_LCD_Line = "**PLEASE WAIT**";                                                 // Set upper LCD line.
Lower_LCD_Line = "";                                                                // Set lower LCD Line.
if(Mode_Time_Indexer==1){Serial.println();}                                         // Display LCD values.
}else{
Upper_LCD_Line = "Position: "+String(Current_Sensor_Y_Axis_Position);               // Set upper LCD line.
Lower_LCD_Line = "=> "+String(Temp_Measurement_Reading)+"mm";                       // Set lower LCD Line.
}
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);                                 // Display LCD.
}  
// /////////////////////////////////////  DEBUGGING - CONSTANT MEASUREMENT OPTION - ABOVE


// [MODE] => "DISPLAY MEASUREMENT"
if(Mode=="DISPLAY"){                                                                // Waiting for pallet mode in operation.
if(Mode_Time_Indexer==0){Review_Measurement_Array(Scan_Memory);}                    // Complete sensor scan procedures.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
Upper_LCD_Line = "Mode: "+Mode;                                                     // Set upper LCD line.
if(Current_Sensor_Y_Axis_Position==Upper_Position){                                 // Determine if servo needs to reset.
Upper_LCD_Line = "**PLEASE WAIT**";                                                 // Set upper LCD line.
Lower_LCD_Line = "";                                                                // Set lower LCD Line.
if(Mode_Time_Indexer==1){Serial.println();}                                         // Send data via serial line.
}else{
String Out_Of_Tolerance_Warning = "";                                               // Declare warning inset string
if(Current_Sensor_Y_Axis_Position==88){if(Within_Margin(Temp_Measurement_Reading, 551, 5)){Out_Of_Tolerance_Warning=" (PASS)";}else{Out_Of_Tolerance_Warning=" (ATTN)";}}
if(Current_Sensor_Y_Axis_Position==104){if(Within_Margin(Temp_Measurement_Reading, 531, 5)){Out_Of_Tolerance_Warning=" (PASS)";}else{Out_Of_Tolerance_Warning=" (ATTN)";}}
Upper_LCD_Line = "Position: "+String(Current_Sensor_Y_Axis_Position);                         // Set upper LCD line.
Lower_LCD_Line = "=> "+String(Temp_Measurement_Reading)+"mm"+Out_Of_Tolerance_Warning;        // Set lower LCD Line.
}
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);                                 // Display LCD.
if(Mode_Time_Indexer > Waiting_Delay){                                              // Once measurement timer has completed.
Mode = "POSITION"; Mode_Restart();                                                  // Set upper LCD line.
Current_Sensor_Y_Axis_Position=Current_Sensor_Y_Axis_Position+Scan_Steps;           // Increment servo position.
Temp_Measurement_Reading = 0;                                                       // Reset temp measurement reading variable.
}
} 


// [MODE] => "WAITING"
if(Mode=="WAITING"){                                                                // Waiting for pallet mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
Upper_LCD_Line = "Mode: "+Mode;                                                     // Set upper LCD line.
Lower_LCD_Line = Progress_Bar(Waiting_Delay, Mode_Time_Indexer);                    // Set lower LCD Line.
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);                                 // Display LCD.
if(Mode_Time_Indexer > Waiting_Delay){Mode = "MEASURING"; Mode_Restart();}          // Switch mode once complete.
}  


// [MODE] => "SENDING SERIAL"
if(Mode=="SEND SERIAL"){                                                            // Waiting for pallet mode in operation.
if(Override_Flag){Mode = "OVERRIDE"; Mode_Restart();}                               // Go to override if switched.
Mode_Time_Indexer = Mode_Time_Indexer + 1;                                          // Increment mode counter.
if(Mode_Time_Indexer==1){General_Serial_Output();}                                  // Provide 3rd party feedback via serial (if requried).
Upper_LCD_Line = "Mode: "+Mode;                                                     // Set upper LCD line.
Lower_LCD_Line = Progress_Bar(Serial_Output_Delay, Mode_Time_Indexer);              // Set lower LCD Line.
General_LCD_Output(Upper_LCD_Line, Lower_LCD_Line);                                 // Display LCD.
if(Mode_Time_Indexer > Serial_Output_Delay){Mode = "WAITING"; Mode_Restart();}      // Switch mode once complete.
}
//delay(25);                                                                        // Base cycle throttle. [Do not remove!]
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
//Sensor_mm = (Sensor_Duration/2)*0.303;                                            // Convert to mm. (previous - incorrect)
//Sensor_mm = (Sensor_Duration/2)*0.034;                                            // Convert to cm. (previous - incorrect)
Sensor_mm = (Sensor_Duration/2.9)/2;                                                // Convert to cm.
delayMicroseconds(10);                                                              // Apply reading buffer.
Sensor_mm = Sensor_mm + 70;                                                         // Apply sensor offeset to rail.
return Sensor_mm;                                                                   // Return value.
}


// [FUNCTION] => OBTAING DISTANCE (MM) FROM ULTRASONIC SENSORS (RAW PULSES)
long Accurate_Scan_Sensor(int Sensor_Trigger, int Sensor_Echo, int Scan_Accuracy){
for (int i = 0; i < Scan_Accuracy; i++) {                                           // Loop through measurement samples.
Scan_Memory[i] = Scan_Sensor(Sensor_Trigger_A1, Sensor_Echo_A1);                    // Save measurement to sample array.
Scan_Indexer = i;                                                                   // Increment scan indexer.
Scan_Completed = true;                                                              // *** May need to move to function main loop...
}
}


// [FUNCTION] => REVEIW THE MEASURE MEMORY ARRAY AND DETERMINE MOST COMMON READING
int Review_Measurement_Array(int Scan_Memory[1000]){
int Return_Value = 0;                                                               // Declare return value.                                                     
int Array_Memory[Scan_Accuracy][2];                                                 // Declare measurement array variable.
int b = 0;                                                                          // Declare 1st indexer.
int Determined_Reading = 0;                                                         // Declare determined reading.
int Index_Memory = 0;                                                               // Declare *** May need to remove...
memset(Array_Memory, 0, sizeof(Array_Memory));                                      // Reset measurement array varaible.
for (int i = 0; i < Scan_Accuracy; i++) {                                           // Cycle through scan array.
if(Check_If_Value_In_Array(Scan_Memory[i], Array_Memory)){                          // Check is measurement exists / present.
for(int ii = 0; ii < Scan_Accuracy; ii++){if(Array_Memory[ii][0]==Scan_Memory[i]){Array_Memory[ii][1]=Array_Memory[ii][1]+1;}}
}else{
Array_Memory[b][0]=Scan_Memory[i];                                                  // Load measurement to reading array.
b++;                                                                                // Increment 1st indexer.
}   
}
for (int a = 0; a < Scan_Accuracy; a++) {                                           // Cycle through scan sample inspection.
if(Array_Memory[a][1]>Index_Memory&&Array_Memory[a][0]!=0){                         // Transfer measurement array to memory array.
Index_Memory = Array_Memory[a][1];                                                  // 
Determined_Reading = Array_Memory[a][0];                                            // 
Temp_Measurement_Reading = Array_Memory[a][0];                                      //
}
}
Serial.print(Determined_Reading);                                                   // Transfer reading via serial.
Serial.print(";");                                                                  // Transfer reading via serial.
return Return_Value;                                                                // Return function variable.
}


// [FUNCTION] => CHECK IN VARIABLE IS IN ARRAY.
bool Check_If_Value_In_Array(int Scan_Item, int Scan_Memory[1000][2]){
int Return_Value = false;                                                           // Declare return variable.
for (int i = 0; i < 100; i++) {                                                     // Ccyle through scanned items.
if(Scan_Memory[i][0]==Scan_Item){Return_Value=true;}                                // *** may need to modify...
}
return Return_Value;                                                                // Return functional variable.
}


// [FUNCTION] => CREATE AND RETURN PROGRESS BAR STRING
String Progress_Bar(int Set_Value, int Counter){
String Output_String = "";                                                          // Declare return variable.
long Available_Segments = 16;                                                       // Determine LCD width segments / spaces.
float Real_Progress_Ratio = float(Counter) / float(Set_Value);                      // Determine progress variables.
int Artificial_Progress = int(Real_Progress_Ratio*Available_Segments);              // Determine dispaly value.
for (int i = 0; i < Artificial_Progress; i++){                                      // Cycle through segment loop.
Output_String += ".";                                                               // Add bar segments -> load ratio.
}
return Output_String;                                                               // Return functional variable.
}


// [FUNCTION] => DETERMINE IS READING FALLS WITHIN ACCEPATABLE MARGIN
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
}
