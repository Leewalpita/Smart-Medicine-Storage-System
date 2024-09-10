#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <ESP32Servo.h>


#define dhtPin 15
#define LDR_LEFT 35
#define LDR_RIGHT 32

char tempArr[6];
char ldrLArr[4]; //LDR left side data array
char ldrRArr[4]; //LDR right side data array


const int servoPin = 18;
double gamma_val = 0.75;  
Servo servo530L;

int servo_position =0;
int pos = 0;
double T_offset = 30;
float minimun_Arr = 0;
int D = 0;
int I = 0;

DHTesp dhtSensor;
WiFiClient espClient;
PubSubClient mqttClient(espClient);


void setup() {
  pinMode(LDR_LEFT, INPUT);
  pinMode(LDR_RIGHT, INPUT);

  Serial.begin(115200);
  setupWifi();
  setupMqtt();
  dhtSensor.setup(dhtPin,DHTesp::DHT22);
   servo530L.attach(servoPin, 500, 2400);
}

void loop() {
  if(!mqttClient.connected()){
    connectToBroker();
  }
  mqttClient.loop();
  updatetemp();
  Serial.println(tempArr);
  mqttClient.publish("DHT_TEMP",tempArr);
  delay(50);

  updateLight();
  Serial.println(ldrLArr);
  mqttClient.publish("LDR_L_VAL", ldrLArr);
  delay(100);

  Serial.println(ldrRArr);
  mqttClient.publish("LDR_R_VAL", ldrRArr);
  delay(150);

  motorAngle();

  if(minimun_Arr>servo_position){
    for (pos = servo_position; pos <= minimun_Arr; pos += 1) {
    servo530L.write(pos);
    delay(10);}
    servo_position= minimun_Arr;
  }

  else if(minimun_Arr<servo_position){
  for (pos = servo_position; pos >= minimun_Arr; pos -= 1) {
    servo530L.write(pos);
    delay(10);}
    servo_position= minimun_Arr;
  }
}

void updatetemp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  String(data.temperature,2).toCharArray(tempArr,6);
}

void updateLight() {

  float left_servor = analogRead(LDR_LEFT) * 1.00;
  float right_servor = analogRead(LDR_RIGHT) * 1.00;

  float left_servor_cha = (float)(left_servor - 4063.00) / (32.00 - 4063.00);
  float right_servor_cha = (float)(right_servor - 4063.00) / (32.00 - 4063.00);

  updateAngle(left_servor_cha, right_servor_cha);

  String(left_servor_cha).toCharArray(ldrLArr, 4);
  String(right_servor_cha).toCharArray(ldrRArr, 4);
}

void updateAngle(float left_servor, float right_servor) {
  float max_I = left_servor;
  float D = 1.5;

  if (right_servor > max_I) {
    max_I = right_servor;
    D = 0.5;
  }

  int theta = T_offset * D + (180 - T_offset) * max_I * gamma_val;
  theta = min(theta, 180);

  servo530L.write(theta);
}

void motorAngle(){
  int sensorL =analogRead(LDR_LEFT);
  int sensorR =analogRead(LDR_RIGHT);
  String(sensorL).toCharArray(ldrLArr,6);
  String(sensorR).toCharArray(ldrRArr,6);

  double D = (sensorR>sensorL) ? 0.5 : 1.5;
  double min_VAL = min(sensorR,sensorL);
  double I = 1-(min_VAL-32)/4031;
  minimun_Arr = min(180.0,((T_offset*D)+(180-T_offset)*I*gamma_val));
}

void recieveCallback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char payloadCharAr[length];
  for(int i=0;i < length; i++){
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.println();
     if(strcmp(topic,"SERVO_ANGLE") == 0){
        T_offset = atof(payloadCharAr);
    }
      else if(strcmp(topic,"SERVO_CONTROL") == 0){
        gamma_val = atof(payloadCharAr);}}



void setupWifi(){
  WiFi.begin("Wokwi-GUEST", "");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("wiFi Connected");
  Serial.println("IP address :");
  Serial.println(WiFi.localIP());
}

void setupMqtt(){
  mqttClient.setServer("test.mosquitto.org",1883);
}

void connectToBroker(){
  while(!mqttClient.connected()){
    Serial.print("Attempting the mqtt connection ");
    if(mqttClient.connect("ESP32-1297534332")){
      Serial.println("Connected");
      //subscribe 
      mqttClient.subscribe("SERVO_ANGLE");
      mqttClient.subscribe("SERVO_CONTROL");
    }else{
      Serial.print("Failed");
      Serial.print(mqttClient.state());
      delay(5000);
    }
  }
}
