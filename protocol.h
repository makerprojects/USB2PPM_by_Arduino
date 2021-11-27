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


////////////////////// Overview //////////////////////////////
// This C header file defines the constants needed to communicate with the
// Arduino based PPM Encodervia USB.
//
// This header file would be included in the Arduino sketch as well as into 
// Joystick2PPM application
//
// 
// Serial commands, sent on the virtual serial port or over TTL Serial.
// See the Arduino Encoder user's guide at www.pikoder.de for more info.
//

enum ArdPPM_Command
{
    COMMAND_SET_TARGET                  =   0x84, // 3 data bytes
};

// These are the values to put in to bRequest when making a setup packet
// for a control transfer to the USB2PPM.
enum ArdPPM_Request
{
    REQUEST_GET_SYSTEMINFORMATION       =   0x80,
    REQUEST_GET_PARAMETER               =   0x81,
    REQUEST_SET_PARAMETER               =   0x82,
};


// These are the bytes used to refer to the different system information
// in REQUEST_GET_SYSTEMINFORMATION. 
enum ArdPPM_SYSTEMINFORMATION
{
    SYSTEMINFORMATION_FIRMWARE_ID       =   0x1,         
    SYSTEMINFORMATION_FIRMWARE_VERSION  =   0x2,
    SYSTEMINFORMATION_FIRMWARE_CHECKSUM =   0x3,  // reserved for later use
};

enum fwID_Definitions
{
    ARD_PPM_ENCODER                     =   0x1
};


// These are the bytes used to refer to the different parameters
// in REQUEST_GET_PARAMETER and REQUEST_SET_PARAMETER.  
enum ArdPPM_Parameter
{
    PARAMETER_CHANNELS_AVAILABLE        =   1, // 1 byte [1 .. 8] 
    PARAMETER_PPM_POLARITY              =   2, // 1 byte [0 .. 1] with 0 = negative, 1 = positive 
};
