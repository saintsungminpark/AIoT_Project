////////////////////////////////////////////////////////////////////////
/// Copyright (c)2015 Dan Truong
/// Permission is granted to use this software under the MIT
/// licence, with my name and copyright kept in source code
/// http://http://opensource.org/licenses/MIT
///
/// KY039 Arduino Heartrate Monitor V1.0 (April 02, 2015)
////////////////////////////////////////////////////////////////////////
 
// German Comments by Joy-IT
 
 
////////////////////////////////////////////////////////////////////////
/// @param[in] IRSensorPin Analog PI which is connected with the sensor
/// @param[in] delay (msec) The delay between the calls of the sample function
//                   You will get the best results if take 5 sample for each heart beat 
///                  Not slower than 150 mSec for 70 BPM pulse 
///                  Better take 60 mSec for a pulse of 200 BPM.
///
/// @Short description
/// This code is a so called Peak-detection.
/// It doesn't record a heart beat history,
/// instead it will search for peaks in the recorded data, 
/// and show them via LED. While knowing the delays you can calculate the pulse.
////////////////////////////////////////////////////////////////////////
// 
int rawValue;

bool heartbeatDetected(int IRSensorPin, int delay)
{
 static int maxValue = 0;
 static bool isPeak = false;

 bool result = false;

 rawValue = analogRead(IRSensorPin);
 // Hier wird der aktuelle Spannungswert am Fototransistor ausgelesen und inder rawValue-Variable zwischengespeichert
 rawValue *= (1000/delay);

 // Sollte der aktuelle Wert vom letzten maximalen Wert zu weit abweichen
 // (z.B. da der Finger neu aufgesetzt oder weggenommen wurde)
 // So wird der MaxValue resetiert, um eine neue Basis zu erhalten.
 if (rawValue * 4L < maxValue) { maxValue = rawValue * 0.8; } // Detect new peak
 if (rawValue > maxValue - (1000/delay)) {
 // Hier wird der eigentliche Peak detektiert. Sollte ein neuer RawValuegroeßer sein
 // als der letzte maximale Wert, so wird das als Spitze der aufgezeichntenDaten erkannt.
 if (rawValue > maxValue) {
 maxValue = rawValue;
 }
 // Zum erkannten Peak soll nur ein Herzschlag zugewiesen werden
 if (isPeak == false) {
 result = true;
 }
 isPeak = true;
 } else if (rawValue < maxValue - (3000/delay)) {
 isPeak = false;
 // Hierbei wird der maximale Wert bei jeden Durchlauf
 // etwas wieder herabgesetzt. Dies hat den Grund, dass
 // nicht nur der Wert sonst immer stabil bei jedem Schlag
 // gleich oder kleiner werden wuerde, sondern auch,
 // falls der Finger sich minimal bewegen sollte und somit
 // das Signal generell schwaecher werden wuerde.
 maxValue-=(1000/delay);
 }
 return result;
}
 
 
////////////////////////////////////////////////////////////////////////
// Arduino main code
////////////////////////////////////////////////////////////////////////
int ledPin=13;
int analogPin=A0;
 
void setup()
{
  // The included LED of the Arduino (Digital 13), will be used as output here.
  pinMode(ledPin,OUTPUT);
   
  // Serial output initialization
  Serial.begin(9600);
  Serial.println("Heartbeat detection example code");
}
 
const int delayMsec = 60; // 100msec per sample
 
// The main program has two tasks: 
// - The LED will light up after detecting a heart beat
// - Calculating of the pulse and outputting of it at the serial out.
 
void loop()
{
  static int beatMsec = 0;
  int heartRateBPM = 0;
      Serial.println(rawValue);
  if (heartbeatDetected(analogPin, delayMsec)) {
    heartRateBPM = 60000 / beatMsec;
    // LED-output for the heart beat heart beat
    digitalWrite(ledPin,1);
 
    // Output of the serial data
    Serial.print(rawValue);
    Serial.print(", ");
    Serial.println(heartRateBPM);
     
    beatMsec = 0;
  } else {
      digitalWrite(ledPin,0);
  }
  delay(delayMsec);
  beatMsec += delayMsec;
}
//
//int sensorPin = A0;
//double alpha = 0.75;
//int period = 50;
//double change = 0.0;
//double minval = 0.0;
//void setup ()
//{
//  Serial.begin(9600);
//}
//void loop ()
//{
//    static double oldValue = 0;
//    static double oldChange = 0;
// 
//    int rawValue = analogRead(sensorPin);
//    double value = alpha * oldValue + (1 - alpha) * rawValue;
// 
//    Serial.print(rawValue);
//    Serial.print(",");
//    Serial.println(value);
//    oldValue = value;
// 
//    delay(period);
//}
