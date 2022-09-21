#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Bounce.h>     

#define BUTTON1 10
#define BUTTON2 12
#define LED1 A1
#define LED2 A2
#define DIALING_NUMBER 6
#define IMPULSES 2

Adafruit_PCD8544 display = Adafruit_PCD8544(13, 11, 5, 4, 3);
Bounce bouncer = Bounce(2, 40);
Bounce bouncer1 = Bounce(BUTTON1, 5);
Bounce bouncer2 = Bounce(BUTTON2, 5); 
SoftwareSerial SIM800(8, 9);

String s = "";
String _response = "";
String _signal = "";
boolean regim_Vitia = true;
boolean _fon = true;

String sendATCommand(String cmd, bool waiting) {
  String _resp = "";                            
  Serial.println(cmd);                          
  SIM800.println(cmd);                          
  if (waiting) {                                
    _resp = waitResponse();                     
    if (_resp.startsWith(cmd)) {  
      _resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
    }
    Serial.println(_resp);
    s = _resp;                     
  }
  return _resp;                                 
}

String waitResponse() {                         
  String _resp = "";                            
  long _timeout = millis() + 10000;            
  while (!SIM800.available() && millis() < _timeout)  {}; 
  if (SIM800.available()) {                     
    _resp = SIM800.readString();                
  }
  else {                                        
    Serial.println("Timeout...");               
  }
  return _resp;                                 
}

int count_digits = 0;
int first_digit = -1;
unsigned long time_start_1 = 0;
String call_string = "ATD";
String ussd_string = "AT+CUSD=1,\"";
String empty_line = ""; 
String phone_number;
boolean active_call = false;
const unsigned int zvezda = 77;
const unsigned int reshotka = 99;
boolean number_identification = false;
boolean ussd = false;

void setup() {
  Serial.begin(9600);                       
  SIM800.begin(9600);                       
  sendATCommand("AT", true);                
  Serial.println("Start!");
  display.begin();
  display.clearDisplay();   
  display.setTextSize(2);
  display.setCursor(0, 8);
  pinMode(IMPULSES, INPUT);
  pinMode(DIALING_NUMBER, INPUT);
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
  fon();
}

void loop() {
  if (SIM800.available()) {                   
    _response = waitResponse();                 
    Serial.println(_response);                  
  }
  if (Serial.available()) {                    
    SIM800.write(Serial.read());                
  }
  
  if(_fon) fon();
  
  dialing_number();
  
  if(regim_Vitia == true && button1_press() == true && active_call == false) {
    sendATCommand("ATD111;", true);
    active_call = true;
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 8);
    display.print("0667031706");
    display.display();
    digitalWrite(LED1, HIGH);
    _fon = false;
  }
  if(regim_Vitia == true && button2_press() == true && active_call == false) {
    sendATCommand("ATD0800502050;", true);
    active_call = true;
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 8);
    display.print("0665197585");
    display.display();
    digitalWrite(LED2, HIGH);
    _fon = false;
  }
  
  _response.trim();   
  if(active_call == true && (_response.startsWith("NO CARRIER") || _response.startsWith("NO DIAL TONE") || _response.startsWith("BUSY"))) {
    active_call = false;
    Serial.println("responce");
    _response = empty_line;
    digitalWrite(LED1, LOW);
    digitalWrite(LED1, LOW);
    _fon = true;
  }

  if(active_call == true && regim_Vitia == false) call_completion();
  
  if (_response.startsWith("RING") && active_call == false) {
    sendATCommand("ATA", true);
    _response = empty_line;
    active_call = true;
    Serial.println("ring");
    number_identification = true;
    _fon = false;
  }
  
  if(number_identification) {
    int visible_number = _response.indexOf("+CLIP: \"");
    String phone_number = "";
    if(visible_number != -1) {
      phone_number = _response.substring(8, _response.indexOf("\"", 8));
      if(phone_number == "+380667031706") digitalWrite(LED1, HIGH);
      else if(phone_number == "+380665197585") digitalWrite(LED2, HIGH);
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 8); 
      display.print(phone_number);
      display.display();
      Serial.println(phone_number);
      number_identification = false;
    }
  }
  
  if(ussd) {
    if(_response.startsWith("+CUSD: 0, ")) {
      String usssdd = "";
      usssdd = _response.substring(11, _response.indexOf("\"", 11));
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0); 
      display.print(usssdd);
      display.display();
      ussd = false;
      usssdd = "";
    }
  }
}

void dialing_number()
{
  int pulses = 0; 
  while(digitalRead(6) == HIGH) {  
    if (bouncer.update()) {     
      if (bouncer.read() == 0) {    
        pulses++;                 
        bouncer.rebounce(100);
      }
    }
  }
  
  if(pulses > 0 && pulses <= 10) {
    if(pulses == 10) pulses = 0;
    if (count_digits + 1 == 1) display.clearDisplay();
    display.setTextSize(2);
    display.print(pulses);
    display.display();
    Serial.println(pulses);
    phone_number += pulses;
    count_digits++;
    if (count_digits == 1) first_digit = pulses;
    time_start_1 = millis();
    _fon = false;
  }
  
  if(regim_Vitia == false && active_call == false) {
    if(button1_press() == true) {
      phone_number += "*";
      if (count_digits + 1 == 1) display.clearDisplay();
      display.print("*");
      display.display();
      Serial.println("*");
      count_digits++;
      if (count_digits == 1) first_digit = zvezda;
      time_start_1 = millis();
      _fon = false;
    }
    else if(button2_press() == true) {
      phone_number += "#";
      if (count_digits + 1 == 1) display.clearDisplay();
      display.setTextSize(2);
      display.print("#");
      display.display();
      Serial.println("#");
      count_digits++;
      if (count_digits == 1) first_digit = reshotka;
      time_start_1 = millis();
      _fon = false;
    }
  }
  
  if(time_start_1 != 0) number_is_over_time();
}

void number_is_over_time()
{
  unsigned long time_stop_1 = millis();
  if(time_stop_1 - time_start_1 >= 7000) {
    time_start_1 = 0;
    number_check();
  }
}

void number_check()
{
  if(count_digits >= 3 && count_digits <= 10 && regim_Vitia == false && first_digit < 10) {
    call_string += phone_number;
    call_string += ";";
    sendATCommand(call_string, true);
    active_call = true;
    _fon = false;
    if(call_string.startsWith("111")) digitalWrite(LED1, HIGH);
    else if(call_string.startsWith("0800502050")) digitalWrite(LED2, HIGH);
  }
  else if(count_digits == 1 && first_digit == 0) {
    regim_Vitia = !regim_Vitia;
    Serial.println(regim_Vitia);
    _fon = true;
  }
  else if(count_digits == 1 && first_digit == zvezda && active_call == false) {
    sendATCommand("ATD111;", true);
    active_call = true;
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 8); 
    display.print("0667031706");
    display.display();
    _fon = false;
    digitalWrite(LED1, HIGH);
  }
  else if(count_digits == 1 && first_digit == reshotka && active_call == false) {
    sendATCommand("ATD0800502050;", true);
    active_call = true;
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 8); 
    display.print("0665197585");
    display.display();
    _fon = false;
    digitalWrite(LED2, HIGH);
  }
  else if(count_digits != 1 && first_digit == zvezda && active_call == false) {
    ussd_string += phone_number;
    ussd_string += "\"";
    sendATCommand(ussd_string, true);
    active_call = true;
    ussd = true;
    _fon = false;
  }
  else {
    _fon = true;
  }
    
  phone_number = empty_line;
  call_string = "ATD";
  ussd_string = "AT+CUSD=1,\"";
  count_digits = 0;
  first_digit = -1;
}

void call_completion()
{
  if(button1_press()) {
    sendATCommand("ATH", true);
    active_call = false;
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    _fon = true;
  }
  else if(button2_press()) {
    sendATCommand("ATH", true);
    active_call = false;
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    _fon = true;
  }
}

boolean button1_press()
{
  if(bouncer1.update()) {
    if(bouncer1.read() == HIGH) {
      return true;
    }
  }
  return false;
}

boolean button2_press()
{
  if(bouncer2.update()) {
    if(bouncer2.read() == HIGH) {
      return true;
    }
  }
  return false;
}

void fon()
{
  signal_operator();
  display.setTextSize(1);
  display.setCursor(0, 0);
  if(regim_Vitia) display.print("LOCK");
  else if(!regim_Vitia) display.print("NORMAL");
  display.display();
}

void signal_operator()
{
  static long lastcmd = millis();
  if (millis() - lastcmd > 10000) {  
    lastcmd = millis();             
    sendATCommand("AT+CSQ", true);
  }
   
  int position = s.indexOf("+CSQ: ");
  if(position != -1) {
    position += 6; 
    _signal = s.substring(position, position + 2);
    display.clearDisplay();
    long num = _signal.toInt();
    if(num >= 10) {
      display.setTextSize(5);
      display.setCursor(14, 10);
      display.print(num);
    }
    display.display();
    s = "";
    _signal = "";
  }
}