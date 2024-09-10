//Include libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>

//define OLED parameters 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT  64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3c
#define LED 26
#define LED_HIGH 4
#define LED_LOW 2
#define BUZZER 5
#define PB_CANCEL 13   //red
#define PB_OK 27      //blue
#define PB_UP 25     //yellow
#define PB_DOWN 14  //green
#define DHTPIN 12

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     0
//#define UTC_OFFSET_DST 0

//Declear objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,&Wire , OLED_RESET);
DHTesp dhtSensor;

//global variables
int UTC_OFFSET_DST = 0;
int UTC_hours = 0;
int UTC_minutes = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

unsigned long timeNow = 0;  //always positive 
unsigned long timeLast = 0;

bool alarm_enabled  = true;
int n_alarm = 3;
int alarm_hours[] = {10,1,2};  //initial values
int alarm_minutes[] = {9,10,3};
bool alarm_triggered[] = {false, false ,false};

int n_notes = 7;  //buzzer tones
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494; 
int notes[] = {C,D,E,F,G,A,B};

int current_mode = 0;
int max_modes = 5;
String modes[] = {"1 - set time", "2 - set Alarm 1","3 - set alarm 2", "4 - set alarm 3" , "5 - disable alarms"};

void setup(){
  //runs once
  pinMode(BUZZER , OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(LED_HIGH , OUTPUT);
  pinMode(LED_LOW , OUTPUT);
  pinMode(PB_CANCEL,INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_DOWN, INPUT);

  dhtSensor.setup(DHTPIN ,DHTesp::DHT22);
  Serial.begin(9600);
  //SSD1306_SWITCHCAPVCC = generate display voltage from 3.3v internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(500);
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    print_line("connecting to wifi",0,0,2);
  }
  display.clearDisplay();
  print_line("connected to wifi",0,0,2);
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  //clear the buffer
  display.clearDisplay();
  print_line("Welcome to the MediBox" , 0,0,2);
  delay(500);
  display.clearDisplay();
}

void loop() {
  //put your main code here to run repeatedly
  update_time_with_check_alarm();
  if(digitalRead(PB_OK) == LOW){
    delay(200);
    go_to_menu();
  }
  check_temp();
}

void print_line(String text , int column , int row , int text_size ){
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column,row);      //printing location(coloumn,row)
  display.println(text);
  display.display();
}

void print_time_now(void){       
  display.clearDisplay();
  if((minutes + UTC_minutes) > 59){  
    hours += 1;
  }
    //if((hours + UTC_hours) < 24 && (hours + UTC_hours) >= 0 ){
      //print_line(String(hours + UTC_hours),30,0,2);
    //}
    //else if((hours + UTC_hours) > 24){         //fix hours if hours excedes 24 if days conider should add 1
      //print_line(String((hours + UTC_hours) % 24),30,0,2);
    //}
    //else if((hours + UTC_hours) < 0){         //when hours<12 this can happened 
      //print_line(String(hours - UTC_hours),30,0,2);
    //}
    //print_line(":",40,0,2);                   
    //print_line(String(minutes + UTC_minutes),60,0,2);
  //}
  //else  if((minutes + UTC_minutes) > 59){     //0 < minutes < 60 ; if not add to hours
  
  //reduced two cases to one case 
  //vedio explain both cases descriptively 
  if((hours + UTC_hours) < 24 && (hours + UTC_hours) >= 0 ){
    print_line(String(hours + UTC_hours + 1),30,0,2);
  }
  else if((hours + UTC_hours) > 24){        //keep hours in between 0 and 24
    print_line(String((hours + UTC_hours) % 24),30,0,2);
  }
  else if((hours + UTC_hours) < 0){         //when hours<12 can happened 
    print_line(String(hours - UTC_hours + 1),30,0,2);
  }
  print_line(":",40,0,2);
  print_line(String((minutes + UTC_minutes) % 60),60,0,2);
  
}

void update_time(void){ 
  struct tm timeinfo; //include second, minutes , hours everythinh to define time
  getLocalTime(&timeinfo);//they try to update time using internet if it has not any error
  
  char timeHour[3];
  strftime(timeHour , 3 ,"%H" , &timeinfo);
  hours = atoi(timeHour); //conver to int
  
  char timeMinute[3];
  strftime(timeMinute , 3 ,"%M" , &timeinfo);
  minutes = atoi(timeMinute); //conver to int
} 

void ring_alarm(void){
  display.clearDisplay();
  print_line("Medicine Time!",0,0,2);
  digitalWrite(LED,HIGH);
  bool break_happened = false;
  //RING BUZZER 
  while (break_happened == false && digitalRead(PB_CANCEL)==HIGH){
    for(int i = 0 ; i < n_notes ; i++ ){
      if(digitalRead(PB_CANCEL)==LOW){
        delay(200);
        break_happened =true;
        break;
      }
      tone(BUZZER,notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }
  digitalWrite(LED, LOW);
  display.clearDisplay();
}

void update_time_with_check_alarm(void){
  update_time();
  print_time_now();
  if (alarm_enabled == true){
    for(int i = 0; i < n_alarm ; i++){
      if( alarm_triggered[i] == false){    //if alarm is available then only check the logic
        if ( alarm_hours[i]==hours && alarm_minutes[i]==minutes){
          ring_alarm();
          alarm_triggered[i] = true;
        }
      }
    }
  }
}

int wait_for_button_press(){
  while(true){
   if (digitalRead(PB_UP) == LOW) {
    delay(200);
    return PB_UP;
   }
   else if (digitalRead(PB_DOWN)==LOW) {
    delay(200);
    return PB_DOWN;
   }
   else if (digitalRead(PB_OK)==LOW) {
    delay(200);
    return PB_OK;
   }
   else if (digitalRead(PB_CANCEL)==LOW) {
    delay(200);
    return PB_CANCEL;
   }
    update_time();
  }
}

void go_to_menu(){
  while(digitalRead(PB_CANCEL)== HIGH){
    display.clearDisplay();
    print_line(modes[current_mode],0,0,2);
    int pressed = wait_for_button_press();
    if (pressed == PB_UP){
      delay(200);
      current_mode += 1;
      current_mode = current_mode % max_modes; // keep mode in between 0 to 3
    }
    else if (pressed == PB_DOWN)  {
      delay(200);
      current_mode -= 1;
      if(current_mode < 0){
        current_mode = max_modes - 1; //set -1 as 3
      }
    }
    else if (pressed == PB_OK)  {
      delay(200);
      run_mode(current_mode);
    }
    else if(pressed == PB_CANCEL){
      delay(200);
      break; 
    }
  }
}  

void set_UTC_offset() {
  int temp_offset_hours = 0;
  while (true){
    display.clearDisplay();
    print_line("Enter UTC offset in hours:" + String(temp_offset_hours), 0, 0, 2);
    int pressed = wait_for_button_press();
    if (temp_offset_hours < 12 && pressed == PB_UP){
      delay(200);
      temp_offset_hours += 1;
      if (temp_offset_hours == 12 && pressed == PB_UP){
        temp_offset_hours = -12;
      }
    }
    else if (temp_offset_hours > -12 and pressed == PB_DOWN){
      delay(200);
      temp_offset_hours -= 1;
      if (temp_offset_hours == -12){
        temp_offset_hours = 12;
      }
    }
    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
    else if (pressed == PB_OK) {
      delay(200);
      UTC_hours = temp_offset_hours;
      break;
    }
  }
  
  int temp_offset_minutes = 0 ;
    while (true){
      display.clearDisplay();
      print_line("Enter UTC offset in minutes:" + String(temp_offset_minutes), 0, 0, 2);
      int pressed = wait_for_button_press();
      if (temp_offset_minutes < 59 and pressed == PB_UP){
        delay(200);
        temp_offset_minutes += 1;
      }
      else if (temp_offset_minutes > 0 and pressed == PB_DOWN){
        delay(200);
        temp_offset_minutes -= 1;
      }
      else if (pressed == PB_CANCEL) {
        delay(200);
        break;
      }
      else if (pressed == PB_OK) {
        delay(200);
        UTC_minutes = temp_offset_minutes;
        break;
      }
  }
  if (UTC_hours >= 0) {
    UTC_OFFSET_DST = UTC_hours*3600 + UTC_minutes*60;
  }
  else if (UTC_hours < 0) {
    UTC_OFFSET_DST = UTC_hours*3600 - UTC_minutes*60;
  }
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  display.clearDisplay();
  print_line("UTC time has been set", 0, 0, 2);
  delay(1000);
}

void set_alarm(int alarm){
  int temp_hour = alarm_hours[alarm];
  while(true){
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour),0,0,2); //creat temporary hours untill press ok button
    int pressed = wait_for_button_press();
    if (pressed == PB_UP){
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24 ; //not to go for negative and >24 values
    }  
    else if (pressed == PB_DOWN)  {
      delay(200);
      temp_hour -= 1;
      if(temp_hour < 0){
        temp_hour = 23;   //set -1 as 23
      }
    }
    else if (pressed == PB_OK)  {
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }
    else if(pressed == PB_CANCEL){
      delay(200);
      break; 
    }
  }
  int temp_minutes = alarm_minutes[alarm];
  while(true){
    display.clearDisplay();
    print_line("Enter minutes: " + String(temp_minutes),0,0,2); //creat temporary hours untill press ok button
    int pressed = wait_for_button_press();
    if (pressed == PB_UP){
      delay(200);
      temp_minutes += 1;
      temp_minutes = temp_minutes % 60 ; //not to go for negative and >24 values
    }  
    else if (pressed == PB_DOWN)  {
      delay(200);
      temp_minutes -= 1;
      if(temp_minutes < 0){
        temp_minutes = 59;   //set -1 as 23
      }
    }
    else if (pressed == PB_OK){
      delay(200);
      alarm_minutes[alarm] = temp_minutes;
      break;
    }
    else if(pressed == PB_CANCEL){
      delay(200);
      break; 
    }
  }
  display.clearDisplay();
  print_line("Alarm is set",0,0,2);
  delay(1000);
}

void run_mode(int mode){
  if (mode == 0){
    set_UTC_offset();
  }
  else if(mode == 1|| mode == 2 || mode == 3){
    set_alarm(mode - 1);
  }
  else if(mode == 4){
    alarm_enabled = false;
    display.clearDisplay();
    print_line("alarm cancelled",0,0,2);
    delay(1000);
  }
}

void check_temp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  if(data.temperature > 32){
    display.clearDisplay();
    print_line("Temp HIGH",0,40,1);
    delay(500);
  }
  else if(data.temperature < 26){
    display.clearDisplay();
    print_line("Temp LOW",0,40,1);
    delay(500);
  }
  if(data.humidity >  80){
    display.clearDisplay();
    print_line("Humidity HIGH",0,50,1);
    delay(500);
  }
  else if(data.humidity < 60){
    display.clearDisplay();
    print_line("Humidity LOW",0,50,1);
    delay(500);
  }
  if (data.temperature > 32 or data.humidity > 80){
    digitalWrite(LED_HIGH, HIGH);
    delay(1000);
  }
  else if(data.temperature < 26 or data.humidity < 60){
    digitalWrite(LED_LOW , HIGH);
    delay(1000);
  }
}
