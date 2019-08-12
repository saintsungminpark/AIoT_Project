#include <Wire.h>
#include <DHT.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>      //Esp8266 라이브러리
#include <ArduinoJson.h>      //Arduino에서 Json형식 변환시키는 라이브러리

//DHT센서 : D3, 3.3V, GND
//LED : D2
//Sound : A0, 5V, GND
#define DHTPIN D3
#define LEDPIN D2
#define DHTTYPE DHT11
#define SOUNDPIN A0

//******sound sensor
int threshold = 100; //Change This
float adc_value;
float adc_max=0;
float dB;

//******temp DHT11 sensor
float h,t;

DHT dht(DHTPIN, DHTTYPE); 

void setup() {                
  Serial.begin(9600); // For debugging   
  
  //소리센서 확인용 led setting 
  pinMode(LEDPIN, OUTPUT);
 
  //온도센서 begin
  dht.begin();
  Serial.println(F("DHTxx test!"));
  
  //Wifi 통신 bigin
  WiFi.begin("MS_IPTIME_2G", "hyun0915");
  while (WiFi.status() != WL_CONNECTED) {  
    delay(500);
    Serial.println("Waiting for connection");
  }
  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  sound_sensor();
  temp_sensor();

  http_toServer_temp();
  http_toServer_sound(); //0.1초씩 10번 측정 => 약 1초동안 측정 => 그래서 아래 delay(1000)주석 처리
  //delay(1000);
}

void sound_sensor(){
  int i;
  adc_max = 0;
//  
//  adc_value = analogRead(SOUNDPIN); // Reads the value from the Analog PIN A0
//  dB = 20*log10(adc_value);
//  int light = map(adc_value, 357, 1023, 0, 255);
//  Serial.print("sound : ");
//  Serial.print(adc_value);
//  Serial.print(", ");
//  Serial.println(dB);
//  
//  analogWrite(LEDPIN, light);

  for(i=0;i<10;i++){
    adc_value = analogRead(SOUNDPIN); // Reads the value from the Analog PIN A0
    dB = 50*log10(adc_value);
    if(adc_value > adc_max){
      adc_max = adc_value;
    }
    Serial.print("sound : ");
    Serial.print(adc_value);
    Serial.print(", ");
    Serial.println(dB);
    
    int light = map(adc_value, 330, 1023, 0, 255);
    analogWrite(LEDPIN, light);
    delay(100);
  }

  dB = 50*log10(adc_max);
  Serial.print("############sound : ");
  Serial.print(adc_max);
  Serial.print(", ");
  Serial.println(dB);
}

void temp_sensor(){
  h = dht.readHumidity();
  t = dht.readTemperature();
  
  if (isnan((int)h) || isnan((int)t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print("temp : ");
  Serial.println(t);
}

void http_toServer_temp(){
  if (WiFi.status() == WL_CONNECTED) {
    
    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
    JsonObject& JSONencoder = JSONbuffer.createObject(); 
 
    JSONencoder ["temp"] = t;
    JSONencoder ["hum"] = h;
    JSONencoder ["u_id"] =138;
 
    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
 
    HTTPClient http;    //클래스 정의
 
    http.begin("http://192.168.3.7:5000/sensor/temp/insert");      //Uri 정의
    http.addHeader("Content-Type", "application/json");  //헤더 정의
 
    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
    String payload = http.getString();                                        
 
    Serial.println(httpCode);   //HTTP Status 값
    Serial.println(payload);    //데이터 값
 
    http.end();  //Close connection
 
  } else {
    Serial.println("Error in WiFi connection");
  }
}

void http_toServer_sound(){
  if (WiFi.status() == WL_CONNECTED) {
    
    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
    JsonObject& JSONencoder = JSONbuffer.createObject(); 
 
    JSONencoder ["db"] = dB;
    JSONencoder ["fq"] = adc_max; //frequency 추출은 못했기 때문에 일단은 adc 값 넣는걸로!
    JSONencoder ["u_id"] = 138;
 
    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
 
    HTTPClient http;    //클래스 정의
 
    http.begin("http://192.168.3.7:5000/sensor/sound/insert");      //Uri 정의
    http.addHeader("Content-Type", "application/json");  //헤더 정의
 
    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
    String payload = http.getString();                                        
 
    Serial.println(httpCode);   //HTTP Status 값
    Serial.println(payload);    //데이터 값
 
    http.end();  //Close connection
 
  } else {
    Serial.println("Error in WiFi connection");
  }
}
