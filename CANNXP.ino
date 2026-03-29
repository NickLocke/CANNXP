// Copyright (C) Nick Locke (nick.locke@21jubileepark.com)
// This file is part of CANNXP project on https://github.com/NickLocke/CANNXP
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

// CANNXP is based on CANNX
// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of CANNX project on https://github.com/SvenRosvall/CANNX
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.

// 3rd party libraries needed for compilation: (not for binary-only distributions)
// Streaming   -- C++ stream style output, v5, (http://arduiniana.org/libraries/streaming/)
// VLCB        -- VLCB library for communicating on a CBUS network.
// ACAN2515    -- library to support the MCP2515/25625 CAN controller IC


// 3rd party libraries
#include <Streaming.h>

// VLCB library header files
#include <VLCB.h>
#include <CAN2515.h>    

// forward function declarations
void eventhandler(byte, const VLCB::VlcbMessage *);
void printConfig();

// constants
const byte VER_MAJ = 1;             // code major version
const char VER_MIN = 'a';           // code minor version
const byte VER_BETA = 0;            // code beta sub-version
const byte MANUFACTURER = MANU_DEV; // for boards in development.
const byte MODULE_ID = 102;         // VLCB module type

// These settings assume an Arduino Nano running on the CANGATE hardware from RME
const byte LED_GRN = 4;             // VLCB green Unitialised LED pin
const byte LED_YLW = 5;             // VLCB yellow Normal LED pin
const byte SWITCH0 = 6;             // VLCB push button switch pin

// module name, must be at most 7 characters.
char mname[] = "NXP";

// CAN transport object
VLCB::CAN2515 can2515;                  

// Service objects
VLCB::LEDUserInterface ledUserInterface(LED_GRN, LED_YLW, SWITCH0);
VLCB::SerialUserInterface serialUserInterface;
VLCB::MinimumNodeServiceWithDiagnostics mnService;
VLCB::CanServiceWithDiagnostics canService(&can2515);
VLCB::NodeVariableService nvService;
VLCB::EventConsumerService ecService;
VLCB::EventTeachingService etService;
VLCB::EventProducerService epService;

// Structure of the Event Variables
const int ButtonNumberEV = 1;
const int ButtonTypeEV = 2;              // 1 = Entrance, 2 = Entrance and Exit, 3 = Exit
const int FirstExitButtonNumberEV = 3;
const int FirstRouteNumberEV = 13;
const int NumExitRouteEVs = 10;

// Produced event groupings (the high byte of the event number)
const int PRODUCED_EVENT_CALL_ROUTE = 0;
const int PRODUCED_EVENT_FLASH_LIGHT = 1;
const int PRODUCED_EVENT_STEADY_LIGHT = 2;

// These variables hold details of the currently active entrance button if any. 
int activeEntranceButtonNumber = 0;
long timeEntranceButtonPressed;
byte possibleRoutes[NumExitRouteEVs];
byte possibleExitButtons[NumExitRouteEVs];

// setup - runs once at power on
void setup()
{
  Serial.begin (115200);
  Serial << endl << endl << F("> ** CANNXP ** ") << __FILE__ << endl;

  setupVLCB();

  if (VLCB::readNV(1) == 255)
  {
    // Default value for button press interval is 5s.
    VLCB::writeNV(1, 50);
  }

  // show code version and copyright notice
  printConfig();

  // end of setup
  Serial << F("> ready") << endl << endl;
}


// loop - runs forever
void loop()
{
  // do VLCB message, switch and LED processing
  VLCB::process();

  // Check whether an entrance button needs to be checked for a timeout
  // We only need to process the timeout if there is an active entrance button
  if(activeEntranceButtonNumber > 0)
  {
    int buttonPressInterval = VLCB::readNV(1) * 100;
    long now = millis();
    bool timeOut = now > timeEntranceButtonPressed + buttonPressInterval;

    if(timeOut)
    {
      Serial << F("> Entrance button ") << activeEntranceButtonNumber << F(" timed out.") << endl;
      cancelEntranceButton();
    }
  }

  // bottom of loop()
}

// setup VLCB - runs once at power on from setup()
void setupVLCB()
{
  VLCB::checkStartupAction(LED_GRN, LED_YLW, SWITCH0);

  VLCB::setServices({
    &mnService, &ledUserInterface, &serialUserInterface, &canService, &nvService,
    &ecService, &epService, &etService});

  // set config layout parameters
  VLCB::setNumNodeVariables(10);
  VLCB::setMaxEvents(32);
  VLCB::setNumEventVariables((NumExitRouteEVs * 2) + 2);

  // set module parameters
  VLCB::setVersion(VER_MAJ, VER_MIN, VER_BETA);
  VLCB::setModuleId(MANUFACTURER, MODULE_ID);

  // set module name
  VLCB::setName(mname);

  // register our VLCB event handler, to receive event messages of learned events
  ecService.setEventHandler(eventhandler);

  // configure and start CAN bus and VLCB message processing
  can2515.setNumBuffers(2, 2);      // more buffers = more memory used, fewer = less
  can2515.setOscFreq(8000000UL);    // select the crystal frequency of the CAN module
  can2515.setPins(10, 2);           // select pins for CAN bus CE and interrupt connections

  if (!can2515.begin())
  {
    Serial << F("> error starting VLCB") << endl;
  }

  // initialise and load configuration
  VLCB::begin();

  Serial << F("> mode = (") << _HEX(VLCB::getCurrentMode()) << ") " << VLCB::Configuration::modeString(VLCB::getCurrentMode());
  Serial << F(", CANID = ") << VLCB::getCANID();
  Serial << F(", NN = ") << VLCB::getNodeNum() << endl;

}

// An entrance button has been pressed, so save away the possible exit buttons and consequent routes
void saveRoutesFromEvent(byte eventIndex)
{
   // Save routes for this event for next button press.
   for (byte i = 0; i < NumExitRouteEVs; i++)
   {
     possibleExitButtons[i] = VLCB::getEventEVval(eventIndex, i + FirstExitButtonNumberEV);
     possibleRoutes[i] = VLCB::getEventEVval(eventIndex, i + FirstRouteNumberEV);
   }

   // Report the possible buttons and routes
   for (byte i = 0; i < NumExitRouteEVs; i++)
   {
     // Ignore any non-set values
     if(possibleExitButtons[i] != 0 && possibleExitButtons[i] != 255)
     {
       Serial << F("Possible exit button ") << possibleExitButtons[i] << F(" for route ") << possibleRoutes[i] << endl;
     }
   }
}

// user-defined event processing function
// called from the VLCB library when a learned event is received
// it receives the event table index and the CAN frame
void eventhandler(byte eventIndex, const VLCB::VlcbMessage *msg)
{
  Serial << F("> event handler: index = ") << eventIndex << F(", opcode = 0x") << _HEX(msg->data[0]) << endl;

  // Event Off op-codes have odd numbers.
  bool ison = (msg->data[0] & 0x01) == 0;
  if (!ison)
  {
    // Don't react to OFF events. It's probably occuring because of misconfiguration.
    return;
  }
  
  // If there is already an entrance button active then we assume we are dealing with an exit,
  // other wise we are dealing with a new entrance.
  activeEntranceButtonNumber == 0 ? ProcessEntranceButton(eventIndex) : ProcessExitButton(eventIndex);
}

void ProcessEntranceButton(byte eventIndex)
{
  // Get the button type and the button number
  int buttonType = GetButtonTypeFromEvent(eventIndex);
  int buttonNumber = GetButtonNumberFromEvent(eventIndex);
  
  // Get out if the button is not valid as an entrance
  if(buttonType != 1 && buttonType !=2)
  {
    Serial << F("> Button ") << buttonNumber << F(" is not an entrance button - ignored") << endl; 
    return;
  }

  // Set the two global variables which will keep track of the entrance button
  activeEntranceButtonNumber = buttonNumber;
  timeEntranceButtonPressed = millis();

  // Send an event with this node NN and the Entrance Button Number * 100 as EN. 
  // Intended to allow the entrance button light to be flashed.
  sendOnEvent(PRODUCED_EVENT_FLASH_LIGHT, buttonNumber);

  Serial << F("> Button ") << buttonNumber << F(" is active as an entrance button") << endl; 

  // Record the valid exit buttons and routes for this entrance
  saveRoutesFromEvent(eventIndex);
}

void ProcessExitButton(byte eventIndex)
{
  // Get the button type and the button number
  int buttonType = GetButtonTypeFromEvent(eventIndex);
  int buttonNumber = GetButtonNumberFromEvent(eventIndex);
  
  // Get out if the button is not valid as an exit
  if(buttonType != 2 && buttonType !=3)
  {
    Serial << F("> Button ") << buttonNumber << F(" is not an exit button - ignored") << endl; 
    cancelEntranceButton();
    return;
  }

  // Get out if the button is not valid as an exit for the currently selected entrance, otherwise
  // call the requested route.
  for (byte i = 0; i < NumExitRouteEVs; i++)
  {
    if(buttonNumber = possibleExitButtons[i])
    {
      // The button has matched, so call the route
      Serial << F("> Button ") << buttonNumber << F(" is a valid exit, calling route ") << possibleRoutes[i] << endl; 
      steadyEntranceButton();
      sendOnEvent(PRODUCED_EVENT_CALL_ROUTE, possibleRoutes[i]);
      return; // Get out if we have called a route
    }
  }

  // This code is only reached if the exit button isn't valid for the entrance
  Serial << F("Button ") << buttonNumber << F(" is not a valid exit for entrance ") << activeEntranceButtonNumber << endl;
  cancelEntranceButton();
}

int GetButtonTypeFromEvent(byte eventIndex)
{
  int buttonType = VLCB::getEventEVval(eventIndex, ButtonTypeEV);
  return buttonType;
}

int GetButtonNumberFromEvent(byte eventIndex)
{
  int buttonNumber = VLCB::getEventEVval(eventIndex, ButtonNumberEV);
  return buttonNumber;
}

void cancelEntranceButton()
{
  sendOffEvent(PRODUCED_EVENT_FLASH_LIGHT, activeEntranceButtonNumber);
  Serial << F("> Entrance button ") << activeEntranceButtonNumber << F(" action cancelled.") << endl;
  activeEntranceButtonNumber = 0;
}

void steadyEntranceButton()
{
  sendOnEvent(PRODUCED_EVENT_STEADY_LIGHT, activeEntranceButtonNumber);
  Serial << F("> Entrance button ") << activeEntranceButtonNumber << F(" released after route called.") << endl;
  activeEntranceButtonNumber = 0;
}

void sendOnEvent(int eventType, int eventNumber)
{
  unsigned int eventNum = eventNumber;
  VLCB::sendMessageWithNN(OPC_ACON, eventType, eventNumber);
}

void sendOffEvent(int eventType, int eventNumber)
{
  unsigned int eventNum = eventNumber;
  VLCB::sendMessageWithNN(OPC_ACOF, eventType, eventNumber);
}

// print code version config details and copyright notice
void printConfig()
{
  // code version
  Serial << F("> code version = ") << VER_MAJ << VER_MIN << F(" beta ") << VER_BETA << endl;
  Serial << F("> compiled on ") << __DATE__ << F(" at ") << __TIME__ << F(", compiler ver = ") << __cplusplus << endl;

  // copyright
  Serial << F("> © Nick Locke (MERG 3518) 2026") << endl;
}
