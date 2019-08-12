//아두이노 Uno 보드 >> 예제 >> SoftwareSerial
#include <SoftwareSerial.h>

SoftwareSerial mySerial(D5, D6); // RX, TX
int transfer_count = 0;

#include <ArduinoJson.h>      //Arduino에서 Json형식 변환시키는 라이브러리
StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
JsonObject& JSONencoder = JSONbuffer.createObject(); 
char JSONmessageBuffer[300];


#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>      //Esp8266 라이브러리

 

void setup() {
  Serial.begin(9600); //PC - NodeMCU(my)
  mySerial.begin(9600); //NodeMCU(your) - NodeMCU(my)
  WiFi.begin("MS_HOME_SECOND_FLOOR_2G", "MS19960331!");  
  while (WiFi.status() != WL_CONNECTED) {  
 
    delay(500);
    Serial.println("Waiting for connection");
  }
}

void loop() { // run over and over
  if (WiFi.status() == WL_CONNECTED) {
  if (mySerial.available()) {
    int BPM;
    BPM = mySerial.read();
    //Serial.print(BPM);


    
    JSONencoder ["bpm"] = BPM;    //평균값 Json 변환
    JSONencoder ["u_id"] = 1; 
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    
    transfer_count++;
    Serial.print("NodeMcu : Attempting to receive Json file ");
    Serial.println(transfer_count);
    Serial.println("...");
    Serial.println("...");
    Serial.println("NodeMcu : BPM Request Complete ");
    Serial.println(JSONmessageBuffer);

    Serial.println("");
    delay(500);
   
      HTTPClient http;    //클래스 정의
      http.begin("http://192.168.1.199:5000/hanger_info/insert");      //Uri 정의
      http.addHeader("Content-Type", "application/json");  //헤더 정의
      int httpCode = http.POST(JSONmessageBuffer);   //Send the request
      String payload = http.getString();    
      Serial.println(httpCode);   //HTTP Status 값
      Serial.println(payload);    //데이터 값      
      http.end();  //Close connection
         
    }
    
  }

  
}
