// Copyright 2021 Gregor Schlechtriem

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//    http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


////////////////////// APP FUNCTION //////////////////////////////
// This programm will put out a PPM signal @ sigPin (default = pin 8)


////////////////////// INCLUDES //////////////////////////////////
#include "protocol.h"

///////////////////// SYSTEM INFORMATION /////////////////////////
#define productID ARD_PPM_ENCODER // identify product 
#define fwVersion "1.0"           // firmware version

///////////////////// PARAMETERS AND SETTINGS ////////////////////
#define PPM_MinLength 1000 // minimal pulse length in µs
#define PPM_MaxLength 2000 // maximum pulse length in µs
#define PPM_FrLen 20000  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PPM_PulseLen 300  //set the pulse length
#define sigPin 8  //set PPM signal output pin on the arduino
#define default_channel_number 8  //set the number of chanels
#define default_onState HIGH  //set polarity of the pulses: HIGH is positive, LOW is negative
#define BUFFSIZE 24

///////////////////// COMMANDS ///////////////////////////////////
typedef enum { start, attention, command, stream, timeout } state;
typedef enum { idle, serial_control_write, serial_control_read, 
        define_startup_val, get_parameter, set_parameter, enable_channel, 
        disable_channel, system_info_read } command_type;


volatile byte _cmdbuffer[BUFFSIZE + 1];
volatile int _buffindex = 0;

volatile bool onState = default_onState;

volatile int _mode = attention;
volatile int _commandtype = idle;

volatile int _channel = 0; 
volatile int _position = 0;
volatile int channel_number = default_channel_number;
volatile int ppm[default_channel_number]; 

// this array holds the servo values for the ppm signal and are used as defaults
// change these values here if you need different values (usually servo values move between 1000 and 2000)

int ppmDefaultChannelValue[default_channel_number] = {
    1500,  // Channel 1 default value
    1500,  // Channel 2 default value
    1500,  // Channel 3 default value
    1500,  // Channel 4 default value
    1500,  // Channel 5 default value
    1500,  // Channel 6 default value
    1500,  // Channel 7 default value
    1500   // Channel 8 default value
};

void setup(){  
  //initiallize default ppm values
  for(int i=0; i<channel_number; i++){
    ppm[i]= verifyConstraints(ppmDefaultChannelValue[i]);
  }

  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, !onState);  //set the PPM signal pin to the default state (off)
  
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;
  
  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();

  Serial.begin(115200);
}

void handleCommand(byte data) {

  switch(_commandtype) {
       
    case serial_control_write:
      // channel data with position data
      if( _buffindex == 0 )
        _channel = data;
      else if( _buffindex == 1 )
        _position = data & 0x7F;
      else if( _buffindex == 2 )
        _position = verifyConstraints(( _position + data * 0x80 ) / 4);
        
      _buffindex++;
      if( _buffindex == 3 ) {
      
        if( _channel < 0 ) _channel = 0;
        if( _channel > channel_number - 1 ) _channel = channel_number - 1;
               
        ppm[_channel] = _position;

        _mode = attention;
        _buffindex = 0;                
      }
      break;

    case system_info_read:

      switch (data) {
        
        case SYSTEMINFORMATION_FIRMWARE_ID:
          Serial.write((byte) productID);
          break;
                   
        case SYSTEMINFORMATION_FIRMWARE_VERSION:
          Serial.write(fwVersion);
          break;    
      }
      _mode = attention;
      break;           

    case get_parameter:

      switch (data) {
        
        case PARAMETER_PPM_POLARITY:
          Serial.write((byte) onState);
          break;
                   
        case PARAMETER_CHANNELS_AVAILABLE:
          Serial.write((byte) channel_number);
          break;    
      }     
      _mode = attention;
      break;      

    case set_parameter:

      switch (data) {
        
        case PARAMETER_PPM_POLARITY:
          onState = GetNextToken();
          break;
                   
        case PARAMETER_CHANNELS_AVAILABLE:
          channel_number = GetNextToken();
          if (channel_number > default_channel_number) {
            channel_number = default_channel_number;
          }
          else if (channel_number < 1) {
            channel_number = 1;
          }
          break;    
      }
      _mode = attention;
      break;           
 
    default:
      _mode = attention;
      _buffindex = 0;                 
  }
}

void loop(){
  for(;;) { 
    if( Serial.available() ) {
      byte b = Serial.read();
      
      switch(_mode) {

        case attention:
          _cmdbuffer[_buffindex] = b;
          _buffindex++; 
          
          switch (_cmdbuffer[0]) {
          
            case COMMAND_SET_TARGET:
              _mode = command;
              _commandtype = serial_control_write;
              break;
              
            case REQUEST_GET_SYSTEMINFORMATION:
              _mode = command;
              _commandtype = system_info_read;
              break; 
              
            case REQUEST_GET_PARAMETER:
              _mode = command;
              _commandtype = get_parameter;
              break;

            case REQUEST_SET_PARAMETER:
              _mode = command;
              _commandtype = set_parameter;
              break;

            default:
              _mode = attention;
          }
          _buffindex = 0;
          break;
                   
        case command:
          handleCommand(b);
          break;
        }
      
    }
  }
}


int verifyConstraints(int PPMValue) {
  if (PPMValue > PPM_MaxLength) {
    return PPM_MaxLength;
  }
  if (PPMValue < PPM_MinLength) {
    return PPM_MinLength;
  }
  return PPMValue;
}


int GetNextToken () {
  while (!Serial.available() ) {
  // wait    
  }
  return Serial.read();
}

ISR(TIMER1_COMPA_vect){  //leave this alone
  static boolean state = true;
  
  TCNT1 = 0;
  
  if(state) {  //start pulse
    digitalWrite(sigPin, onState);
    OCR1A = PPM_PulseLen * 2;
    state = false;
  }
  else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= channel_number){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PPM_PulseLen;// 
      OCR1A = (PPM_FrLen - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR1A = (ppm[cur_chan_numb] - PPM_PulseLen) * 2;
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
}
