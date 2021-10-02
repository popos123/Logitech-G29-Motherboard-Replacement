//#define Software // software or hardware SPI
//#define debug 1 // serial debug (1 - Serial2, 0 - Serial)
// Buttons serial input
#define SO PC15
#define SH_LD PC14
#define CLK PC13
// LEDs serial output
#define SI PB10
#define RCLK PB11
// Emulated Encoder pins
#define pinA PA0
#define pinB PA1
// Buttons serial output
#define btnPS PB4 // ID 20 Play Station [Btn 25]
// Analog read for LEDs
#define LED_read PB1

#include <Arduino.h>
#include <HardwareSerial.h>
#include "MLX90363.h"

#if defined(debug) && (debug == 1)
//HardwareSerial Serial2(PA3, PA2); // RX, TX
//HardwareSerial Serial1(PA10, PA9); // RX, TX
#elif defined(debug) && (debug == 0)
#define Serial2 Serial
#endif
Magnet* MLX90363;

// Virtual encoder variables
int32_t enc_old = 0;
int32_t enc_pos = 0, cnt = 0;
int32_t total_position = 0,  last_total_position = 0, once = 0;
uint8_t hysteresis = 3; // 2 is the minimum, 3 is recommended
float resolution = 10; // 10 is worst, 1 is the best accuracy
PinName PinA = digitalPinToPinName(pinA); // Pin A of the virtil encoder (pin D10)
PinName PinB = digitalPinToPinName(pinB); // Pin B of the virtil encoder (pin D11)

// buttons variables
bool Parallel_data[2][24];
bool v_enc_pinA_Last = 0;
bool v_enc_pinA = 0, v_enc_pinB = 0;
int8_t v_enc_pos = 0; // position of the encoder form the steering wheel
char pinsName[2][5] = { 
  { PB12, PB13, PB14, PB15 }, 
  { PB9, PB8, PB7, PB6, PB5 } 
}; // matrix pins
const uint8_t RegPinsSize = 6;
bool Register[RegPinsSize]; // only one a 74HC595 with 6 LEDs
bool busy_button = 0;
uint8_t set_cnt = 0;
bool state_ = 0;
bool on_ = 0; // just for fix
uint8_t LED_old = 0;

void setState(int id, int value);
void clearRegister();
void SendSerial();
void buttons_read();
void emu_encoder();
void statement();
void write_LED();
void t_right();
void t_left();

void setup() {
  MLX90363 = new Magnet();
  MLX90363->begin(PA7, PA6, PA5, PA4); // MOSI, MISO, SCK, CS
  #ifdef debug
  Serial2.begin(1000000);
  #endif
  // Emulated Encoder pins
  pinMode(PinA, INPUT); // just for init G29 (if not use STM32F401 - just delete it)
  delay(1000); // as above 
  pinMode(PinA, OUTPUT);
  pinMode(PinB, OUTPUT);
  // Buttons serial input
  pinMode(SH_LD, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(SO, INPUT);
  // LEDs serial output
  pinMode(RCLK, OUTPUT);
  pinMode(SI, OUTPUT);
  // Buttons serial output
  for (int i = 0; i < 5; i++) {
    if (i < 4) pinMode(pinsName[0][i], OUTPUT); // read x [4 pins]
    pinMode(pinsName[1][i], OUTPUT); // read y [5 pins]
  }
  for (int i = 0; i < 5; i++) {
    if (i < 4) digitalWrite(pinsName[0][i], LOW); // read x [4 pins]
    digitalWrite(pinsName[1][i], LOW); // read y [5 pins]
  }
}

void loop() {
  // encoder emu functions - total time in software SPI mode is < 2 ms ( > 500Hz) [without Serial port]
  // in hardware SPI mode total time is < 1.5 ms ( > 667Hz) [without Serial port]
  MLX90363->ReadData();
  emu_encoder();
  buttons_read();
  write_LED();
}

void write_LED() {
  uint8_t LED = map(analogRead(LED_read), 1023, 160, 0, 5);
  if (LED != LED_old) {
    if (LED == 0) setState(0, 0), setState(1, 0), setState(2, 0), setState(3, 0), setState(4, 0);
    if (LED == 1) setState(0, 1), setState(1, 0), setState(2, 0), setState(3, 0), setState(4, 0);
    if (LED == 2) setState(0, 1), setState(1, 1), setState(2, 0), setState(3, 0), setState(4, 0);
    if (LED == 3) setState(0, 1), setState(1, 1), setState(2, 1), setState(3, 0), setState(4, 0);
    if (LED == 4) setState(0, 1), setState(1, 1), setState(2, 1), setState(3, 1), setState(4, 0);
    if (LED == 5) setState(0, 1), setState(1, 1), setState(2, 1), setState(3, 1), setState(4, 1);
    SendSerial();
    LED_old = LED;
  }
}

void clearRegister() {
  for(int i=0; i<RegPinsSize; i++)
    Register[i]=LOW;
}

void SendSerial() {
  if (busy_button == 0) {
    busy_button = 1;
    digitalWrite(RCLK, LOW); 
    for(int i=RegPinsSize-1; i>=0; i--){
      digitalWrite(CLK, LOW);
      digitalWrite(SI, Register[i]);
      digitalWrite(CLK, HIGH); 
    }
    digitalWrite(RCLK, HIGH);
    busy_button = 0;
  }
  else SendSerial();
}

void setState(int id, int value) {
  Register[id]=value;
}

// buttons read
void buttons_read() {
  // read button state
  if (busy_button == 0) {
    busy_button = 1;
    digitalWrite(CLK,LOW);
    // Parallel data that will be entered through D0 - D7 Pins of 74HC165
    digitalWrite(SH_LD,LOW);
    // stores the d0-d23 pins parallel data
    digitalWrite(SH_LD,HIGH);
    //Read 24-bit data from 3 x 74HC165
    for(int i=0;i<24;i++) {
      bool PinState = digitalRead(SO); // read the state of the SO:
      digitalWrite(CLK, LOW);
      digitalWrite(CLK, HIGH);
      Parallel_data[0][i] = PinState;
    }
    busy_button = 0;
  }
  else buttons_read();
  // emulate button matrix and send data
  int i2 = 0, j = 0;
  for(int i=0;i<24;i++) {
    if (i == 16 || i == 17 || i == 18 || i == 19 || i == 20) {
      if (i == 18) {
        // TODO - unuse SHARE button :(
      }
      if (i == 19) {
        // TODO - unuse OPTIONS button :(
      }
      if (i == 20) {
        digitalWriteFast(digitalPinToPinName(btnPS), Parallel_data[0][i]); // x [4 pins]
        #ifdef debug
        if (Parallel_data[1][i] != Parallel_data[0][i]) {
          if (Parallel_data[0][i] > 0) Serial2.println("ID: " + String(i) + " PRESSED, " + String(i2) + " " + String(j));
          else Serial2.println("ID: " + String(i) + " RELEASED");
        }
        #endif
      }
    } else {
      if (Parallel_data[1][i] != Parallel_data[0][i]) { // if changed state - send data
        // encoder read from steering wheel
        if (i == 22) if (Parallel_data[0][i] == 1) v_enc_pinA = 1; else v_enc_pinA = 0;
        if (i == 23) if (Parallel_data[0][i] == 1) v_enc_pinB = 1; else v_enc_pinB = 0;
        if ((v_enc_pinA_Last == 0) && (v_enc_pinA == 1)) {
          if (v_enc_pinB == 0) {
            v_enc_pos = 1;
            #ifdef debug
            Serial2.println("CW");// turn right (CW)
            #endif
          } else {
            v_enc_pos = -1;
            #ifdef debug
            Serial2.println("CCW");// turn left (CCW)
            #endif
          }
        }
        if (i != 23) { // if not encoder pinB
          if (i != 22) { // if not encoder pinA
            bool state = 0;
            digitalWriteFast(digitalPinToPinName(pinsName[0][i2]), Parallel_data[0][i]); // x [4 pins]
            if (Parallel_data[0][i] > 0) state = 1; else state = 0;
            digitalWriteFast(digitalPinToPinName(pinsName[1][j]), state); // y [5 pins]
          }
        } else { // if encoder pinA or pinB
          bool state = 0;
          bool state_enc_1 = 0, state_enc_2 = 0;
          if (v_enc_pos == 1) state_enc_1 = 1, state_enc_2 = 0;
          else if (v_enc_pos == -1) state_enc_1 = 0, state_enc_2 = 1;
          digitalWriteFast(digitalPinToPinName(pinsName[0][i2]), state_enc_1); // x [4 pins]
          digitalWriteFast(digitalPinToPinName(pinsName[0][i2+1]), state_enc_2); // x [4 pins]
          if (Parallel_data[0][i] > 0) state = 1; else state = 0;
          digitalWriteFast(digitalPinToPinName(pinsName[1][j]), state); // y [5 pins]
        }
        if ((i == 22 || i == 23) && (Parallel_data[0][i] == 0)) { // set 0 after press in encoder emu output pinA nad pinB
          digitalWriteFast(digitalPinToPinName(pinsName[0][i2]), 0); // x [4 pins]
          digitalWriteFast(digitalPinToPinName(pinsName[0][i2+1]), 0); // x [4 pins]
        }
        #ifdef debug
        if (Parallel_data[0][i] > 0) Serial2.println("ID: " + String(i) + " PRESSED, " + String(i2) + " " + String(j));
        else Serial2.println("ID: " + String(i) + " RELEASED");
        for (int i = 0; i < 4; i++) {
          Serial2.print(String(digitalRead(pinsName[0][i])) + ", "); // read x [4 pins]
          for (int j = 0; j < 5; j++) {
            Serial2.print(String(digitalRead(pinsName[1][j])) + " "); // read y [5 pins]
          }
          Serial2.println("");
        }
        #endif
        v_enc_pinA_Last = v_enc_pinA;
        Parallel_data[1][i] = Parallel_data[0][i];
      }
    }
    if (i < 15 || i == 20 || i == 22) if (i2 < 3) i2++; else i2 = 0; // for max 16 keys, for 21 key, for 23 key
    if (((i+1) % 4 == 0) && ((i+1) <= 16)) if (j < 4) j++; else j = 0;
  }
}

// emulate an encoder tick
void emu_encoder() {
  int32_t enc_curr = abs(round(MLX90363->AngleLSB() / 10.0)), Set = 0;
  if (enc_curr != enc_old && enc_curr >= 0) { // if new value and > 0 (some errors was < 0)
    if (enc_curr < 100 && enc_old > 1500) enc_old = enc_curr + hysteresis, cnt++; // turn right fix
    if (enc_curr > 1500 && enc_old < 100) enc_old = enc_curr - hysteresis, cnt--; // turn left fix
    if (enc_curr - hysteresis > enc_old) enc_pos = 1, Set = 1; // turn right (CW)
    if (enc_curr + hysteresis < enc_old) enc_pos = -1, Set = 1; // turn left (CCW)
    while (last_total_position != total_position) {
      if (enc_pos == 1) t_right(); // turn right (CW)
      if (enc_pos == -1) t_left(); // turn left (CCW)
    }
    if (Set == 1) {
      int32_t min_val = 1193; // min 1193, max 66500
      total_position = round(((cnt * 1624) + enc_curr - min_val)/resolution);
      if (once == 0) last_total_position = total_position, once = 1; // first run calibrate
      enc_old = enc_curr;
      Set = 0;
      #ifdef debug
      Serial2.print("Angle_(Lsb): " + String(MLX90363->AngleLSB() - min_val)); // debug
      Serial2.print("  Encoder_Val: " + String(enc_pos)); // debug
      Serial2.print("  Total_Pos: " + String(total_position)); // debug
      Serial2.print("  Pos_deg: " + String(map(total_position, 0, (66500/resolution), -450, 450))); // debug
      Serial2.println(""); // debug
      #endif
    }
  }
}

// emulate an encoder CW
void t_right() {
  digitalWriteFast(PinA, HIGH); // pin A
  //delayMicroseconds(1);
  digitalWriteFast(PinB, HIGH); // pin B
  //delayMicroseconds(1);
  digitalWriteFast(PinA, LOW);
  //delayMicroseconds(1);
  digitalWriteFast(PinB, LOW);
  //delayMicroseconds(1);
  statement();
}

// emulate an encoder CCW
void t_left() {
  digitalWriteFast(PinB, HIGH);
  //delayMicroseconds(1);
  digitalWriteFast(PinA, HIGH);
  //delayMicroseconds(1);
  digitalWriteFast(PinB, LOW);
  //delayMicroseconds(1);
  digitalWriteFast(PinA, LOW);
  //delayMicroseconds(1);
  statement();
}

// how many times the encoder tick must be sent to reach a certain position
void statement() {
  if (total_position >= 0) {
    if (last_total_position > total_position) last_total_position--;
    else last_total_position++;
  }
  else {
    if (last_total_position < total_position) last_total_position++;
    else last_total_position--;
  }
}