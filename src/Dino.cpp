/*
  Library for dino ruby gem.
*/

#include "Arduino.h"
#include "Dino.h"

Dino::Dino(){
  reset();
}


void Dino::process(char* request, char* loopResponse) {
  
  // Reset the response.
  strcpy(response, "");
  
  // Parse the request.
  strncpy(cmd, request, 2);         cmd[2] =    '\0';
  strncpy(pinStr, request + 2, 2);  pinStr[2] = '\0';
  strncpy(val, request + 4, 3);     val[3] =    '\0';
  
  // Serial.println(cmd);
  // Serial.println(pin);
  // Serial.println(val);
  // if (debug) Serial.println(request);
  
  convertPin();
  if (pin == -1) return; // Should raise some kind of "bad pin" error.
  
  int cmdid = atoi(cmd);
  switch(cmdid) {
    case 0:  setMode             ();  break;
    case 1:  dWrite              ();  break;
    case 2:  dRead               ();  break;
    case 3:  aWrite              ();  break;
    case 4:  aRead               ();  break;
    case 10: addDigitalListener  ();  break;
    case 11: addAnalogListener   ();  break;
    case 12: removeListener      ();  break;
    case 90: reset               ();  break;
    case 98: setHeartRate        ();  break;
    case 99: toggleDebug         ();  break;
    default:                          break;
  }
  
  // Write local response back to the global var for main loop to handle.
  strcpy(loopResponse, response);
}


int Dino::updateListeners(char* responses) {
  int count = 0;

  // Update digital listeners.
  for (int i = 0; i < 22; i++) {
    if (digitalListeners[i]) {
      pin = i;
      dRead();
      strcpy((responses + (count * 9)), response);;
      count++;
    }
  }

  // Update analog listeners.
  for (int i = 0; i < 8; i++) {
    if (analogListeners[i] != 0) {
      pin = analogListeners[i]; pinStr[0] = 'A';
      pinStr[1] = (char)(((int)'0')+i); pinStr[2] = '\0'; // Should make this suitable for > 9 analog pins.
      aRead();
      strcpy((responses + (count * 9)), response);
      count++;
    }
  }
 
  return count;
}



// CMD = 00 // Pin Mode
void Dino::setMode() {
  if (atoi(val) == 0) {
    pinMode(pin, OUTPUT);
  } else {
    pinMode(pin, INPUT);
  }
}

// CMD = 01 // Digital Write
void Dino::dWrite() {
  removeListener();
  pinMode(pin, OUTPUT);
  if (atoi(val) == 0) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
}

// CMD = 02 // Digital Read
void Dino::dRead() { 
  pinMode(pin, INPUT);
  int oraw = digitalRead(pin);
  char m[8];
  if (analogPin) {
    sprintf(response, "%s::%02d", pinStr, oraw);
  } else {
    sprintf(response, "%02d::%02d", pin, oraw);
  }
}

// CMD = 03 // Analog (PWM) Write
void Dino::aWrite() {
  removeListener();
  pinMode(pin, OUTPUT);
  analogWrite(pin,atoi(val));
}

// CMD = 04 // Analog Read
void Dino::aRead() {
  pinMode(pin, INPUT);
  int rval = analogRead(pin);
  char m[8];
  sprintf(response, "%s::%03d", pinStr, rval);  // Send response with 'A0' formatting, not raw pin number, so pinStr not pin.
}


// CMD = 10
// Listen for a digital signal on any pin.
void Dino::addDigitalListener() {
  if (analogPin) {
    int index = atoi(&pinStr[1]);
    analogListeners[index] = 0; // Disable existing analog listener if any.
  }
  digitalListeners[pin] = true;
}

// CMD = 11
// Listen for an analog signal on analog pins only.
void Dino::addAnalogListener() {
  if (analogPin) {
    int index = atoi(&pinStr[1]);
    analogListeners[index] = pin;
  }
}

// CMD = 12
// Remove analog and digital listeners from any pin.
void Dino::removeListener() {
  if (analogPin) {
    int index = atoi(&pinStr[1]);
    analogListeners[index] = 0;
  }
  digitalListeners[pin] = false;
}


// CMD = 90
void Dino::reset() {
  debug = false;
  heartRate = 5; // Default heart rate is 5ms.
  for (int i = 0; i < 14; i++) digitalListeners[i] = false;
  for (int i = 0; i < 8; i++)  analogListeners[i] = false;
}

// CMD = 98
// Set the heart rate in milliseconds.
void Dino::setHeartRate() {
  heartRate = atoi(val);
}

// CMD = 99
void Dino::toggleDebug() {
  if (atoi(val) == 0) {
    debug = false;
    strcpy(response, "Debug 0");
  } else {
    debug = true;
    strcpy(response, "Debug 1");
  }
}



// Convert the pin received in stringy form to a raw pin as an integer.
// pin is -1 on error.
void Dino::convertPin() {
  pin = -1;
  if(pinStr[0] == 'A' || pinStr[0] == 'a') {
    analogPin = true;
    switch(pinStr[1]) {
      case '0':  pin = A0; break;
      case '1':  pin = A1; break;
      case '2':  pin = A2; break;
      case '3':  pin = A3; break;
      case '4':  pin = A4; break;
      case '5':  pin = A5; break;
      case '6':  pin = A6; break;
      case '7':  pin = A7; break;
      default:             break;
    }
  } else {
    analogPin = false;
    pin = atoi(pinStr);
    if(pin == 0 && (pinStr[0] != '0' || pinStr[1] != '0')) {
      pin = -1;
    }
  }
}
