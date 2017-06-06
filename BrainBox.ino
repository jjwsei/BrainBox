#include <Wire.h>
#include <LiquidCrystal_I2C.h> // arduino/libraries/LiquidCrystal
#include <Adafruit_NeoPixel.h>

// Temperature Functions
void tempHowHot(void);
void tempCoolMeDown(void);
void tempTooHot(void);

// Light Functions
void lightHowBright(void);
void lightLightDarkAlarm(void);
void lightAutoLight(void);
void lightLightShow(void);

// Sound Functions
void soundHowLoud(void);
void soundCountClaps(void);
void soundHowFar(void);
void soundTooCloseAlarm(void);

int pitch2Note(int);

#define NUMBER_OF_CARDS  3

// setup the LCD object
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// setup WS2812 LED
#define LED_PIN 13
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, LED_PIN, NEO_GRB + NEO_KHZ800);

typedef void (*ProjectFunctions) ();

struct menuItem
{
  char *itemName;
  int projects;
  char **project_array;
  ProjectFunctions *FP_array;
};

char *temperature_projects[] = {"How Hot?", "Cool Me Off", "Too Hot!"}; 
char *light_projects[] = {"How Bright?", "Light/Dark Alarm", "Automatic Light", "Light Music", "Light Show"};
char *sound_projects[] = {"How Loud?", "Count Claps", "How Far?", "Too Close Alarm"};

ProjectFunctions temperatureFPs[] = {tempHowHot, tempCoolMeDown, tempTooHot};
int temperatureFunctions = 3;
ProjectFunctions lightFPs[] = {lightHowBright, lightLightDarkAlarm, lightAutoLight, lightMusic, lightLightShow};
int lightFunctions = 5;
ProjectFunctions soundFPs[] = {soundHowLoud, soundCountClaps, soundHowFar, soundTooCloseAlarm};
int soundFunctions = 4;

struct menuItem cards[NUMBER_OF_CARDS];

int current_card_index = 0;
int previous_card_index = 0;
int current_project_index = 0;

boolean card_selected = 0;
char *selected_project;

const byte red_LED = 5;
const byte yellow_LED = 6;
const byte green_LED = 7;

const byte piezo = 9;

const byte temp_sensor_input = A0;

const byte Brown_Wire = A1;
const byte Orange_Wire_R = A2;
const byte Blue_Wire = 10;
const byte Green_Wire = 11;
const byte Yellow_Wire = 12;
const byte Orange_Wire_L = 13;

const byte LED1 = Blue_Wire;
const byte LED2 = Green_Wire;
const byte LED3 = Yellow_Wire;
const byte LED4 = Orange_Wire_L;

const byte ultra_trig = Yellow_Wire;
const byte ultra_echo = Blue_Wire;

// Button Variables
const byte scroll_up_button = 2;
const byte scroll_down_button = 3;
const byte start_button = 18;

volatile byte scroll_up_button_pressed = 0;
volatile byte scroll_down_button_pressed = 0;
volatile byte start_button_pressed = 0;

//************************************************************************
// ISRs
void scrollUpISR(void)
{
  noInterrupts();
  delay(50);
  if(digitalRead(scroll_up_button) == 0)
  {
    scroll_up_button_pressed = 1;
  }  
  while(digitalRead(scroll_up_button) == 0);
  interrupts();
}

void scrollDownISR(void)
{
  noInterrupts();
  delay(50);
  if(digitalRead(scroll_down_button) == 0)
  {
    scroll_down_button_pressed = 1;
  }
  while(digitalRead(scroll_down_button) == 0);
  interrupts();
}

void startISR(void)
{
  noInterrupts();
  delay(50);
  if(digitalRead(start_button) == 0)
  {
    start_button_pressed = 1;
  }
  while(digitalRead(start_button) == 0);
  interrupts();
}

/*
ISR(ADC_vect, ISR_BLOCK) { // ADC conversion complete

  // Save old sample from 'in' position to xfade buffer:
  buffer1[nSamples + xf] = buffer1[in];
  buffer2[nSamples + xf] = buffer2[in];
  if(++xf >= XFADE) xf = 0;

  // Store new value in sample buffers:
  buffer1[in] = ADCL; // MUST read ADCL first!
  buffer2[in] = ADCH;
  if(++in >= nSamples) in = 0;
}
*/

//*****************************************************************************
//***************************************************************************
void setup() {
  Serial.begin(9600); 
  
  // Initialize the LCD
  lcd.begin(16, 2);   
  lcd.clear();
  
  cards[0].itemName = "Temperature";
  cards[0].projects = temperatureFunctions;
  cards[0].project_array = temperature_projects;  
  cards[0].FP_array = (temperatureFPs);
  
  cards[1].itemName = "Light";
  cards[1].projects = lightFunctions;
  cards[1].project_array = light_projects; 
  cards[1].FP_array = lightFPs;

  cards[2].itemName = "Sound";
  cards[2].projects = soundFunctions;
  cards[2].project_array = sound_projects;
  cards[2].FP_array = soundFPs; 
  
  pinMode(scroll_up_button, INPUT_PULLUP);
  pinMode(scroll_down_button, INPUT_PULLUP);
  pinMode(start_button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(scroll_up_button), scrollUpISR, FALLING);  
  attachInterrupt(digitalPinToInterrupt(scroll_down_button), scrollDownISR, FALLING); 
  attachInterrupt(digitalPinToInterrupt(start_button), startISR, FALLING);   
  
  pinMode(red_LED, OUTPUT);
  pinMode(yellow_LED, OUTPUT);
  pinMode(green_LED, OUTPUT);
  
  pinMode(piezo, OUTPUT);
    
  projectCleanUp();
}

void loop() {
  delay(500);
  
  if(scroll_up_button_pressed == 1)
  {
    if(card_selected == 0)
    {
      if(++current_card_index == NUMBER_OF_CARDS)
        current_card_index = 0;
    }
    else
    {
       if(++current_project_index == cards[current_card_index].projects)
         current_project_index = 0; 
    }
    Serial.println(cards[current_card_index].itemName);
    Serial.println(cards[current_card_index].project_array[current_project_index]);
    updateLCD(current_card_index, current_project_index);
    scroll_up_button_pressed = 0;
  }
  
  if(scroll_down_button_pressed == 1)
  {
    if(card_selected == 0)
    {
      if(--current_card_index == -1)
        current_card_index = NUMBER_OF_CARDS - 1;
    }
    else
    {
      if(--current_project_index == -1)
        current_project_index = cards[current_card_index].projects - 1;  
    }
    Serial.println(cards[current_card_index].itemName);
    Serial.println(cards[current_card_index].project_array[current_project_index]);
    updateLCD(current_card_index, current_project_index);  
    scroll_down_button_pressed = 0;  
  }
  
  if(start_button_pressed == 1)
  {
    if(card_selected == 0)
    {
      card_selected = 1;
      updateLCD(current_card_index, current_project_index);
    }
    else
    {
      // run the selected project
      (cards[current_card_index].FP_array[current_project_index])();  
    }
    start_button_pressed = 0;
  }
}

//*********************************************************************************************
void tempHowHot(void)
{
  float temp_value = 0;
  float max_value = 0;
  float min_value = 60.0;
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    temp_value = (thermistorRead(temp_sensor_input) * 9.0)/ 5.0 + 32.0;
    if(temp_value > max_value)
      max_value = temp_value;
    if(temp_value < min_value)
      min_value = temp_value;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temp: ");
    lcd.print(temp_value,1);
    lcd.print(" F");
    lcd.setCursor(0, 1);
    lcd.print("Mn ");
    lcd.print(min_value,1);
    lcd.setCursor(8, 1);
    lcd.print("Mx ");
    lcd.print(max_value,1); 
    delay(350);      
  }
  projectCleanUp();
}

//*********************************************************************************************
void tempCoolMeDown(void)
{
  int red = 0;
  int green = 0;
  int blue = 0;
  
  pinMode(LED_PIN, OUTPUT);
  strip.begin();
  strip.show();
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {
    red = random(255);
    green = random(255);
    blue = random(255);
    
    colorWipe(strip.Color(red, green, blue), 1);
  }
  colorWipe(strip.Color(0, 0, 0), 1);
  strip.show();
  projectCleanUp();
}

//*********************************************************************************************
void tempTooHot(void)
{
  int orange[3] = {255,128,0};
  int purple[3] = {153,51,255};
  int pink[3] = {255,102,178};
  int white[3] = {255,255,255};
  int red[3] = {255,0,0};
  int green[3] = {0,255,0};
  int blue[3] = {0,0,255};
  int cyan[3] = {0,153,153};
  
  int i = 0;
  int *colors[8];
  int *color;
  
  colors[0] = orange;
  colors[1] = green;
  colors[2] = purple;
  colors[3] = cyan;
  colors[4] = white;
  colors[5] = red;
  colors[6] = pink;
  colors[7] = blue;
  
  
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
 
  digitalWrite(12, 1);
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {
    for(i = 0; i < 8; i++)
    {
      color = colors[i];
      analogWrite(11, (255 - color[0]));
      analogWrite(13, (255 - color[1]));
      analogWrite(10, (255 - color[2]));
      delay(200);
    }
  }
  projectCleanUp();
}

//*********************************************************************************************
void lightHowBright(void)
{
  int light_sensor_input = A1;
  int light_value = 0;
  int max_value = 0;
  int min_value = 0;
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {
    light_value = analogRead(light_sensor_input);
    light_value = 1023 - light_value;
    if(light_value > max_value)
      max_value = light_value;
    if(light_value < min_value)
      min_value = light_value;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Light: ");
    lcd.print(light_value);
    lcd.setCursor(0, 1);
    lcd.print("Min ");
    lcd.print(min_value);
    lcd.setCursor(8, 1);
    lcd.print("Max ");
    lcd.print(max_value);  
    delay(350);    
  }
  projectCleanUp();
}

//*********************************************************************************************
void lightLightDarkAlarm(void)
{
  int light_sensor_input = A0;
  int light_value = 0;
  boolean alarm = 0;
  boolean look_for_light = 0;
  int set_point = 100;
  
  lcd.clear();
  lcd.print("Look for Light?");
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {
    if(scroll_down_button_pressed == 1)
    {
      scroll_down_button_pressed = 0;
      look_for_light = 1;
      lcd.clear();
      lcd.print("Look for Light?");
    }
    if(scroll_up_button_pressed == 1)
    {
      scroll_up_button_pressed = 0;
      look_for_light = 0;
      lcd.clear();
      lcd.print("Look for Dark?");
    }
  }  
  lcd.clear();
  lcd.print("Guarding...");
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {
    light_value = analogRead(light_sensor_input);
    light_value = 1023 - light_value;  
    if(look_for_light == 1)
    {
      if(light_value > set_point && alarm == 0)
        alarm = 1;
    }
    else
    {
      if(light_value < set_point && alarm == 0)
        alarm = 1;     
    }
    if(alarm == 1)
    {
      lcd.clear();
      lcd.print("   ALARM!!");      
      soundAlarm(); 
    }   
    delay(500);  
  }
  projectCleanUp();
}

//*********************************************************************************************
void lightAutoLight(void)
{
  int light_sensor_input = A1;
  int light_value = 0;
  int set_point = 100;
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {
    light_value = analogRead(light_sensor_input);
    light_value = 1023 - light_value; 

    if(light_value < set_point)
      digitalWrite(yellow_LED, 1);
    else
      digitalWrite(yellow_LED, 0);    
  }
  projectCleanUp();
}

//*********************************************************************************************
void lightMusic(void)
{
  int light_sensor_input = A0;
  int pitch = 0;
  
  lcd.clear();
  lcd.print("Running...");  
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {  
    pitch = pitch2Note(analogRead(light_sensor_input));
    tone(9,pitch,209);
    delay(100);
  }
  projectCleanUp();
}

//*********************************************************************************************
void lightLightShow(void)
{
  int rand = 0;
  
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {
     rand = random(8192); 
     
     digitalWrite(red_LED, (rand & 0x04));
     digitalWrite(yellow_LED, (rand & 0x02));
     digitalWrite(green_LED, (rand & 0x01));
     digitalWrite(LED1, (rand & 0x08));
     digitalWrite(LED2, (rand & 0x10) >> 4);
     digitalWrite(LED3, (rand & 0x20) >> 5);
     digitalWrite(LED4, (rand & 0x40) >> 6);
     
     delay(100);    
  }
  projectCleanUp();  
}

//*********************************************************************************************
void soundHowLoud(void)
{
  const byte mic_input = A0;
  int result_low = 0;
  int result_high = 0;
  int result = 0;
  int value = 0;
  
  /*
  // Start up ADC in free-run mode for audio sampling:
  DIDR0 |= _BV(ADC0D);  // Disable digital input buffer on ADC0
  ADMUX  = ADC_CHANNEL; // Channel sel, right-adj, AREF to 3.3V regulator
  ADCSRB = 0;           // Free-run mode
  ADCSRA = _BV(ADEN) |  // Enable ADC
    _BV(ADSC)  |        // Start conversions
    _BV(ADATE) |        // Auto-trigger enable
    _BV(ADIE)  |        // Interrupt enable
    _BV(ADPS2) |        // 128:1 prescale...
    _BV(ADPS1) |        //  ...yields 125 KHz ADC clock...
  _BV(ADPS0); //  ...13 cycles/conversion = ~9615 Hz
  */
  
  ADMUX = 0xC0;
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {
     ADCSRA = 0xC0;
     while(ADSC == 1){};
     result_low = ADCL;
     result_high = ADCH;
     result = (result_high << 8) | result_low;
     if(result < 1000)
     {  
       Serial.println(value);
       digitalWrite(red_LED, 1);
       delay(100);
       digitalWrite(red_LED, 0);
     }      
  }
  projectCleanUp();
}

//*********************************************************************************************
void soundCountClaps(void)
{
  start_button_pressed = 0;
  
  lcd.clear();
  lcd.print("Running...");
  
  while(start_button_pressed == 0)
  {
    soundAlarm();
    delay(500);    
  }
  projectCleanUp();
}

//*********************************************************************************************
void soundHowFar(void)
{
  float duration;
  
  pinMode(ultra_trig, OUTPUT);
  pinMode(ultra_echo, INPUT);
  digitalWrite(ultra_trig, 0);  

  start_button_pressed = 0;
  lcd.clear();
  
  while(start_button_pressed == 0)
  {   
    digitalWrite(ultra_trig, HIGH);
    delayMicroseconds(10); 
    digitalWrite(ultra_trig, LOW);
    duration = pulseIn(ultra_echo, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(duration / 148.0); //inches
    lcd.print(" in");
    lcd.setCursor(0,1);
    lcd.print(duration / 58.1); // centimeters
    lcd.print(" cm");
    delay(1000);  
  }
  projectCleanUp();
}

//*********************************************************************************************
void soundTooCloseAlarm(void)
{
  float duration;
  int distance = 1;
  
  pinMode(ultra_trig, OUTPUT);
  pinMode(ultra_echo, INPUT);
  digitalWrite(ultra_trig, 0);

  start_button_pressed = 0;
  lcd.clear();
  
  lcd.print("Set distance");
  lcd.setCursor(0,1);
  lcd.print("Distance: ");
   lcd.setCursor(13, 1);
   lcd.print("in");  
  while(start_button_pressed == 0)
  {
     lcd.setCursor(10, 1);
     lcd.print(distance);

     if(scroll_up_button_pressed == 1)
     {
        scroll_up_button_pressed = 0;
        if(++distance > 25)
          distance = 25;
     }
     if(scroll_down_button_pressed == 1)
     {
        scroll_down_button_pressed = 0;
        if(--distance == 0)
          distance = 1;
     }
  }
  
  start_button_pressed = 0;
  while(start_button_pressed == 0)
  {   
    digitalWrite(ultra_trig, HIGH);
    delayMicroseconds(10); 
    digitalWrite(ultra_trig, LOW);
    duration = pulseIn(ultra_echo, HIGH);
    if((duration / 148) <= distance)
    {
      lcd.clear();
      lcd.print("*** ALARM ***");
      digitalWrite(red_LED, 1);
    }
    else
    {
      lcd.clear();
      lcd.print("Monitoring...");
      digitalWrite(red_LED, 0);
    }
    delay(350);  
  }
  
  projectCleanUp();
}

void updateLCD(int card_index, int project_index)
{
  char *iName = cards[card_index].itemName;
  int length = strlen(iName);
  int position = (16 - length) / 2;
  int i = 0;
  
  lcd.clear();
  lcd.setCursor(position, 0);
  lcd.print(iName); 
  if(card_selected == 1)
  {
    lcd.setCursor(0,0);
    for(i = 0; i < position; i++)
      lcd.print(">");
    lcd.print(iName);  
    lcd.setCursor((position + length),0);
    for(;i < 16; i++)
      lcd.print("<");
  }
  lcd.setCursor(0, 1);
  lcd.print(cards[card_index].project_array[project_index]);
}

void projectCleanUp()
{
  lcd.clear();
  lcd.print("   Main Menu   ");
  start_button_pressed = 0;
  scroll_up_button_pressed = 0;
  scroll_down_button_pressed = 0;
  card_selected = 0;
  current_card_index = 0;
  current_project_index = 0;
  digitalWrite(red_LED, 0);
  digitalWrite(yellow_LED, 0);
  digitalWrite(green_LED, 0);
  digitalWrite(LED1, 0);
  digitalWrite(LED2, 0);
  digitalWrite(LED3, 0);
  digitalWrite(LED4, 0);
}

float thermistorRead(int Tpin) 
{
  int   Vo;
  float logRt,Rt,T;
  float R = 10000;          //  fixed resistance, measured with multimeter
  //  c1, c2, c3 are calibration coefficients for a particular thermistor
  float c1 =  1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
  
  Vo = analogRead(Tpin);
  Rt = R*( 1024.0 / float(Vo) - 1.0 );
  logRt = log(Rt);
  T = ( 1.0 / (c1 + c2*logRt + c3*logRt*logRt*logRt ) ) - 273.15;
  return T;
}

void soundAlarm() {
  int piezo = 9;
  byte names[] = {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'};  
  int tones[] = {1915, 1700, 1519, 1432, 1275, 1136, 1014, 956};
  byte melody[] = "3C3d2p3C3d2p3C3d";
  // count length: 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0
  //                                10                  20                  30
  int count = 0;
  int count2 = 0;
  int count3 = 0;
  int MAX_COUNT = 8;
  
  digitalWrite(piezo, 0);
  for (count = 0; count < MAX_COUNT ; count++) {
    for (count3 = 0 ; count3 <= (melody[count*2] - 48) * 30 ; count3++) {
      for (count2 = 0 ; count2 < 8 ; count2++) {
        if (names[count2] == melody[count*2 + 1]) {      
          digitalWrite(piezo, 1);
          delayMicroseconds(tones[count2]);
          digitalWrite(piezo, 0);
          delayMicroseconds(tones[count2]);
        }
        if (melody[count*2 + 1] == 'p') {
          // make a pause of a certain size
          digitalWrite(piezo, 0);
          delayMicroseconds(500);
        }
      }
    }
  }
  digitalWrite(piezo, 0);
}



int pitch2Note(int sensorValue)
{
  float pitch = 0;
  int sensorHigh = 1023;
  int sensorLow = 750; 
  
  pitch = map(sensorValue, sensorLow, sensorHigh, 55, 900);
  if (pitch < 60.205 ) pitch = 55; // A
  if (pitch >= 60.205 && pitch < 69.415) pitch = 65.41; // C
  if (pitch >= 69.415 && pitch < 77.915) pitch = 73.42; // D
  if (pitch >= 77.915 && pitch < 90.205) pitch = 82.41; // E
  if (pitch >= 90.205 && pitch < 104) pitch = 98; // G
  if (pitch >= 104 && pitch < 120.405) pitch = 110; // A
  if (pitch >= 120.405 && pitch < 138.82) pitch = 130.81; // C
  if (pitch >= 138.82 && pitch < 155.82) pitch = 146.83; // D
  if (pitch >= 155.82 && pitch < 180.405) pitch = 164.81; // E
  if (pitch >= 180.405 && pitch < 208) pitch = 196; // G
  if (pitch >= 208 && pitch < 240.815) pitch = 220; // A
  if (pitch >= 240.815 && pitch < 277.645) pitch = 261.63; // C
  if (pitch >= 277.645 && pitch < 311.645) pitch = 293.66; // D
  if (pitch >= 311.645 && pitch < 360.815) pitch = 329.63; // E
  if (pitch >= 360.815 && pitch < 416) pitch = 392; // G
  if (pitch >= 416 && pitch < 481.625) pitch = 440; // A
  if (pitch >= 481.625 && pitch < 555.29) pitch = 523.25; // C
  if (pitch >= 555.29 && pitch < 623.29) pitch = 587.33; // D
  if (pitch >= 623.29 && pitch < 721.62) pitch = 659.25; // E
  if (pitch >= 721.62 && pitch < 831.995) pitch = 783.99; // G
  if (pitch >= 831.995 && pitch <= 880) pitch = 880; // A
  if (pitch > 880)pitch = 0;
  
  return(pitch);
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}


