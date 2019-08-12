//아두이노 Uno 보드 >> 예제 >> SoftwareSerial
#include <SoftwareSerial.h>
#include <Wire.h>
#include <math.h>
#include <ArduinoJson.h>

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24

//*******gyro sensor
// I2C address of the L3G4200D.
// Use I2C scanner to find this value!
int L3G4200D_Address = 0x69;

// Can fine-tune this if you need to
float DPS_MULT = 0.0000085;

// Delta angles (raw input from gyro)
int x = 0;
int y = 0;
int z = 0;

// Actual angles
float angX = 0;
float angY = 0;
float angZ = 0;
float x_ary[20], y_ary[20], z_ary[20];

// Previous angles
float pre_angX = 0;
float pre_angY = 0;
float pre_angZ = 0;

// Differentiate angles
float dif_angX = 0;
float dif_angY = 0;
float dif_angZ = 0;

// Sum of Actual angles
float angX_sum = 0;
float angY_sum = 0;
float angZ_sum = 0;

// Max of Actual angles
float angX_max = 0;
float angY_max = 0;
float angZ_max = 0;

// Previous angles for calculation
float p_angX = 0;
float p_angY = 0;
float p_angZ = 0;

// Calibration values
int gyroLowX = 0;
int gyroLowY = 0;
int gyroLowZ = 0;
int gyroHighX = 0;
int gyroHighY = 0;
int gyroHighZ = 0;

// Used for calculating delta time
unsigned long pastMicros = 0;

SoftwareSerial mySerial(5, 6); // RX, TX
int setupL3G4200D(int scale);
void calibrate();
void updateAngle();
void calc_actual_angle();
void sendJson();
int readRegister(int deviceAddress, byte address);
void writeRegister(int deviceAddress, byte address, byte val);

void setup() {
  //Serial 통신 세팅
  Serial.begin(9600); //PC - NodeMCU(my)
  mySerial.begin(9600); //NodeMCU(your) - NodeMCU(my)

  //gyro 센서 세팅
  Wire.begin(); //Wire.begin(D7,D6) SDA=D7, SCL=D6
  Serial.println("Starting up L3G4200D");
  setupL3G4200D(250); // Configure L3G4200  - 250, 500 or 2000 deg/sec
  delay(1000); // wait for the sensor to be ready 
  calibrate();
  attachInterrupt(0, updateAngle, RISING);
  pastMicros = micros();
}

void loop() {
    int i=0;

    //1초동안 20번 센서값 read ( 0.05s*20 = 1s )
    for(i=0;i<20;i++){
        //이전값 저장
        pre_angX = angX;
        pre_angY = angY;
        pre_angZ = angZ;

       //angX,angY,angZ 계산
        calc_actual_angle();

        //이전값과의 차이 계싼
        dif_angX = angX - pre_angX;
        dif_angY = angY - pre_angY;
        dif_angZ = angZ - pre_angZ;

        x_ary[i]=int(dif_angX*1000);   
        y_ary[i]=int(dif_angX*1000);
        z_ary[i]=int(dif_angX*1000);

        //1초동안 측정값 최대값 계산 - 20개 data
        if(abs(dif_angX) > angX_max){
          angX_max = abs(dif_angX);
        }
      
        if(abs(dif_angY) > angY_max){
          angY_max = abs(dif_angY);
        }
        if(abs(dif_angZ) > angZ_max){
          angZ_max = abs(dif_angZ);
        }
        
        //angX,angY,angZ Serial 출력
        sendJson();

        delay(50);
    }

    //Json형식으로 묶어서 NodeMCU 전송(mySerial 출력) : dif_angX -> dif_angY -> dif_angZ
    StaticJsonBuffer<300> JSONbuffer;   //JSON 버퍼 메모리 할당
    JsonObject& JSONencoder = JSONbuffer.createObject(); 

    //Json array 형식으로 key 생성 "x_gyro", "y_gryo", "z_gyro"
    JsonArray& x_gyro = JSONencoder.createNestedArray("x_gyro"); //JSON array
    JsonArray& y_gyro = JSONencoder.createNestedArray("y_gyro"); //JSON array
    JsonArray& z_gyro = JSONencoder.createNestedArray("z_gyro"); //JSON array

    //20개 값 묶기
    for(i=0;i<20;i++){
      x_gyro.add(x_ary[i]); //Add value to array
      y_gyro.add(y_ary[i]); //Add value to array
      z_gyro.add(z_ary[i]); //Add value to array
    }
    
    JSONencoder ["x_gyro_max"] = angX_max;
    JSONencoder ["y_gyro_max"] = angY_max;
    JSONencoder ["z_gyro_max"] = angZ_max;
    JSONencoder ["u_id"] = 1;

    angX_max = 0;
    angY_max = 0;
    angZ_max = 0;

    //Serial창에 띄우기
    Serial.println("########mySerial 전송");
    JSONencoder.prettyPrintTo(Serial);
    Serial.println();

    //NodeMCU로 전송
    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    mySerial.print(JSONmessageBuffer);


//        JSONencoder ["x"] = (int)((dif_angX)*1000);
//        JSONencoder ["y"] = (int)((dif_angY)*1000);
//        JSONencoder ["z"] = (int)((dif_angZ)*1000);

//        //dif_ang NodeMCU로 전송(mySerial 출력) : dif_angX -> dif_angY -> dif_angZ
//        mySerial.print((int)((dif_angX)*1000));
//        mySerial.print((int)((dif_angY)*1000));
//        mySerial.print((int)((dif_angZ)*1000));

}

void calc_actual_angle(){
  getGyroValues();

  // Calculate delta time
  float dt;
  if(micros() > pastMicros) //micros() overflows every ~70 minutes
    dt = (float) (micros()-pastMicros)/1000000.0;
  else
    dt = (float) ((4294967295-pastMicros)+micros())/1000000.0;


  // Calculate angles
  if(x >= gyroHighX || x <= gyroLowX) {
    angX += ((p_angX + (x * DPS_MULT))/2) * dt;
    p_angX = x * DPS_MULT;
  } else {
    p_angX = 0;
  }

  if(y >= gyroHighY || y <= gyroLowY) {
    angY += ((p_angY + (y * DPS_MULT))/2) * dt;
    p_angY = y * DPS_MULT;
  } else {
    p_angY = 0;
  }

  if(z >= gyroHighZ || z <= gyroLowZ) {
    angZ += ((p_angZ + (z * DPS_MULT))/2) * dt;
    p_angZ = z * DPS_MULT;
  } else {
    p_angZ = 0;
  }  
  pastMicros = micros();
}

void updateAngle()
{
  Serial.println("Interrupt!!");
  getGyroValues();
}

void calibrate()
{
  Serial.println("Calibrating gyro, don't move!");
  for(int i = 0; i < 200; i++) {
    getGyroValues();

    if(x > gyroHighX)
      gyroHighX = x;
    else if(x < gyroLowX)
      gyroLowX = x;

    if(y > gyroHighY)
      gyroHighY = y;
    else if(y < gyroLowY)
      gyroLowY = y;

    if(z > gyroHighZ)
      gyroHighZ = z;
    else if(z < gyroLowZ)
      gyroLowZ = z;
    
    delay(10);
  }
  Serial.println("Calibration complete.");
}

// Print angles to Serial (for use in Processing, for example)
void sendJson() {
    char json[40];
    sprintf(json, 
    "{\"x\":%d,\"y\":%d,\"z\":%d,\"dif_x\":%d,\"dif_y\":%d,\"dif_z\":%d}", 
    (int)(angX*1000), (int)(angY*1000), (int)(angZ*1000),(int)((dif_angX)*1000), (int)((dif_angY)*1000), (int)((dif_angZ)*1000));
    
    //sprintf(json, "{\"x\":%d,\"y\":%d,\"z\":%d}", (int)((dif_angX)*1000), (int)((dif_angY)*1000), (int)((dif_angZ)*1000));
    Serial.println(json);
}

void getGyroValues() {
  byte xMSB = readRegister(L3G4200D_Address, 0x29);
  byte xLSB = readRegister(L3G4200D_Address, 0x28);
  x = ((xMSB << 8) | xLSB);

  byte yMSB = readRegister(L3G4200D_Address, 0x2B);
  byte yLSB = readRegister(L3G4200D_Address, 0x2A);
  y = ((yMSB << 8) | yLSB);

  byte zMSB = readRegister(L3G4200D_Address, 0x2D);
  byte zLSB = readRegister(L3G4200D_Address, 0x2C);
  z = ((zMSB << 8) | zLSB);
}

int setupL3G4200D(int scale) {
  //From  Jim Lindblom of Sparkfun's code

  // Enable x, y, z and turn off power down:
  writeRegister(L3G4200D_Address, CTRL_REG1, 0b00001111);

  // If you'd like to adjust/use the HPF, you can edit the line below to configure CTRL_REG2:
  writeRegister(L3G4200D_Address, CTRL_REG2, 0b00000000);

  // Configure CTRL_REG3 to generate data ready interrupt on INT2
  // No interrupts used on INT1, if you'd like to configure INT1
  // or INT2 otherwise, consult the datasheet:
  writeRegister(L3G4200D_Address, CTRL_REG3, 0b00001000);

  // CTRL_REG4 controls the full-scale range, among other things:

  if(scale == 250) {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00000000);
  } else if(scale == 500) {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00010000);
  } else {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00110000);
  }

  // CTRL_REG5 controls high-pass filtering of outputs, use it
  // if you'd like:
  writeRegister(L3G4200D_Address, CTRL_REG5, 0b00000000);
}

void writeRegister(int deviceAddress, byte address, byte val)
{
    Wire.beginTransmission(deviceAddress); // start transmission to device 
    Wire.write(address);       // send register address
    Wire.write(val);         // send value to write
    Wire.endTransmission();     // end transmission
}

int readRegister(int deviceAddress, byte address)
{
    int v;
    Wire.beginTransmission(deviceAddress);
    Wire.write(address); // register to read
    Wire.endTransmission();

    Wire.requestFrom(deviceAddress, 1); // read a byte

    while(!Wire.available()) {
        // waiting
    }
    v = Wire.read();
    return v;
}
