int SOUND_SENSOR = A0;

//int led = 7;

int threshold = 25; //감도조절

int Sensor_value; 

 

void setup() {                

  Serial.begin(9600); // 시리얼모니터 출력

  pinMode(SOUND_SENSOR, INPUT);

  //pinMode(led, OUTPUT);     

}

 

void loop() {  

  Sensor_value = analogRead(A0);   // Analog PIN A0에서 입력값을 읽어와서 Sensor_value에 저장

  Serial.println(Sensor_value);   // 시리얼모니터에 감도표시

  delay(10);
}
