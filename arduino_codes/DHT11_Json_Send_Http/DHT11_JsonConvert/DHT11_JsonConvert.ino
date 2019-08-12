//////////////////////////////////////////////////////////////////////////////
//온습도 센서의 온도 , 습도 값을 Flask 서버에 Json 파일 형식으로 변환해 Post 해주는 코드 //
//ArduinoJson 5버전을 이용해야 하며, 6버전 이용시 StaticJsonBuffer 함수 오류가 남    //
//////////////////////////////////////////////////////////////////////////////



#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>      //Esp8266 라이브러리
#include <ArduinoJson.h>      //Arduino에서 Json형식 변환시키는 라이브러리

#include <DHT.h>
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);     //DHT11 관련 라이브러리 및 기본선언



void setup() {
  
  
  dht.begin();

  
  Serial.begin(115200);                           
  WiFi.begin("MS_IPTIME_2G", "hyun0915");  
 
  // WiFi.begin("AP_MS_2G", "hyun0915");  
  
  while (WiFi.status() != WL_CONNECTED) {  
 
    delay(500);
    Serial.println("Waiting for connection");
 
  }
 
}
 
void loop() {

  
  
//  float h = dht.readHumidity();
//  float t = dht.readTemperature();
//  float f = dht.readTemperature(true);
//  if (isnan(h) || isnan(t) || isnan(f)) {
//    Serial.println(F("Failed to read from DHT sensor!"));
//    return;
//  }
    float h=1.1;
    float t=1.2;

 
  if (WiFi.status() == WL_CONNECTED) {
    
    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
    JsonObject& JSONencoder = JSONbuffer.createObject(); 
 
    JSONencoder ["hum"] = h;
    JSONencoder ["temp"] = t;
    JSONencoder ["u_id"] = 1;
    
    
    //JsonArray& hum = JSONencoder.createNestedArray("hum"); //JSON array
    //hum.add(h); //Add value to array
    
    //2개이상의 값을 가지는 JSON형식을 정의하기 위한 코드
 
    //JsonArray& temp = JSONencoder.createNestedArray("temp"); //JSON array
    //temp.add(t); //Add value to array

 
 
    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
 
    HTTPClient http;    //클래스 정의
 
    http.begin("http://192.168.3.4:5000/sensor/temp/insert");      //Uri 정의
    http.addHeader("Content-Type", "application/json");  //헤더 정의
 
    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
    String payload = http.getString();                                        
 
    Serial.println(httpCode);   //HTTP Status 값
    Serial.println(payload);    //데이터 값
 
    http.end();  //Close connection
 
  } else {
 
    Serial.println("Error in WiFi connection");
 
  }
 
  delay(5000);  
 
}
