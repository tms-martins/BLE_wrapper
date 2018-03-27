/*
 * Works with App_BLE_05
 * 
 * Sets up the jacket service and allows to test using the Serial port.
 * Commands received via Serial will trigger the same function as those received
 * via BLE. Likewise, commands/replies will be sent to Serial and BLE.
 *
 * When a client message is received the LED stays on for 500 msec.
 * Some test behaviour is implemented. The server replies to some of the commands
 * and simulates some of the state values - see handleClientCommand() and loop().
 *
 * Client commands are received as Json objects:
 *   void handleClientCommand(JsonObject& jsonObj)
 * at the bottom.
 *
 * Server messages are sent as Json objects:
 *   void sendServerMsg(JsonObject& jsonObj)
 *
 * The following are used to set timestamp and state in the BLE service:
 *   void setTimestamp(unsigned long ulTimestamp)
 *   void setPlanName(String & strPlanName)
 *   void setState(uint8_t planState, uint8_t phase, uint8_t progress, uint8_t ctrlMode, uint8_t battLevel)
 */

#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEUUID.h>

#define JSON_BUFF_SIZE 256
#define CLIENT_MSG_BUFF_SIZE 256

#define PIN_LED     5
#define TIMEOUT_LED 500


///////////////////////////////////////////////////////////////////////////////////////////
/// JACKET BLE GLOBALS ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

BLEServer         *pJacketServer;
BLEService        *pJacketService;
BLECharacteristic *pJacketCharactTimestamp;
BLECharacteristic *pJacketCharactPlanName;
BLECharacteristic *pJacketCharactState;
BLECharacteristic *pJacketCharactClientCmd;
BLECharacteristic *pJacketCharactServerMsg;

bool deviceConnected = false;

// forward declaration, so we can call the function in setup()
// before the actual body is declared (the function's body is in another tab)
void setupJacketBLE();


///////////////////////////////////////////////////////////////////////////////////////////
/// GLOBALS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#define PLAN_STATE_NONE    0
#define PLAN_STATE_READY   1
#define PLAN_STATE_STARTED 2
#define PLAN_STATE_PAUSED  3
#define PLAN_STATE_ENDED   4

#define CTRL_MODE_NONE  0
#define CTRL_MODE_MUSIC 1
#define CTRL_MODE_CALL  2

unsigned long timeLastRx = 0;
unsigned long timeLastProgress = 0;  // for simulation purposes

String  currPlanName  = "";
uint8_t currPlanState = PLAN_STATE_NONE;
uint8_t currPhase     = 0;
uint8_t currProgress  = 0;
uint8_t currCtrlMode  = CTRL_MODE_NONE;

int currBattLevel = 50;

char buffClientMsg [CLIENT_MSG_BUFF_SIZE];
boolean hasClientMessage = false;

///////////////////////////////////////////////////////////////////////////////////////////
/// ARDUINO FUNCTIONS /////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(PIN_LED,    OUTPUT);
  Serial.begin(115200);

  setupJacketBLE();
  Serial.println("Waiting for a client connection...");
}

void loop() {
  unsigned long timeCurr = millis();

  // tries to read a Json object from Serial and triggers
  // the same function as an actual client command from BLE
  updateSerial();

  // the LED stays on for 500 msec when a client message is received
  if (timeCurr - timeLastRx < TIMEOUT_LED)
    digitalWrite(PIN_LED, HIGH);
  else
    digitalWrite(PIN_LED, LOW);

  //if (deviceConnected) {

    if (hasClientMessage) {
      timeLastRx = millis();
      //Serial.println("Processing client message");
      DynamicJsonBuffer jsonBuffer(JSON_BUFF_SIZE);
      JsonObject& jsonObj = jsonBuffer.parseObject(buffClientMsg);
      hasClientMessage = false;
      
      if (jsonObj.success()) {
        handleClientCommand(jsonObj);
      }
      else {
        Serial.println("ERROR parsing client command");
      }
    }

    // set the timestamp
    setTimestamp(timeCurr);

    // update the battery level
    currBattLevel = 100 - ((timeCurr/1000) % 100);

    // update the phase and progression
    if (currPlanState == PLAN_STATE_STARTED) {
      if (timeCurr - timeLastProgress > 200) {
        timeLastProgress = timeCurr;
        currProgress ++;
        if (currProgress >= 100) {
          currProgress = 0;
          currPhase++;
          if (currPhase > 3) {
            currPhase = 0;
            currPlanState = PLAN_STATE_ENDED;
          }
        }
      }
    }

    // update the state on the BLE service
    setState(
      currPlanState,
      currPhase,
      currProgress,
      currCtrlMode,
      (uint8_t)currBattLevel
    );
    
    // not elegant, but we want to avoid flooding the connection
    delay(10);
  //}
}


///////////////////////////////////////////////////////////////////////////////////////////
/// BLE SERVICE WRAPPERS //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// send a message from the server to the client
void sendServerMsg(JsonObject& jsonObj) {
  Serial.println("sendServerMsg()");
  jsonObj.printTo(Serial);
  Serial.println();

  char msgBuffer [JSON_BUFF_SIZE];
  int msgSize = jsonObj.printTo(msgBuffer, JSON_BUFF_SIZE);
  pJacketCharactServerMsg->setValue((uint8_t*)msgBuffer, msgSize);
  pJacketCharactServerMsg->notify();

  // I fail to understand why the following doesn't work :'(
  //   std::string jsonString;
  //   jsonObj.printTo(jsonString);
}

// Defines what happens when a "client command" is received
void handleClientCommand(JsonObject& jsonObj) {
  Serial.print("Received from client: ");
  jsonObj.printTo(Serial);
  Serial.println();

  DynamicJsonBuffer jsonBuffer(JSON_BUFF_SIZE);
  JsonObject& jsonReply = jsonBuffer.createObject();

  String cmd = jsonObj["cmd"].as<String>();
  Serial.printf("Command is <%s>\n", cmd.c_str());
  if (cmd == "spl") {
    String planName = jsonObj["pl"]["na"].as<String>();
    //Serial.printf("Starting plan <%s>\n", planName.c_str());
    currPlanName = planName;
    currPlanState = PLAN_STATE_STARTED;
    currPhase = 1;
    currProgress = 0;
    jsonReply["msg"] = "ack";
    jsonReply["pln"] = planName;
    sendServerMsg(jsonReply);
  }
  else if (cmd == "sph") {
    String phaseName = jsonObj["ph"]["na"].as<String>();
    //Serial.printf("Starting phase <%s>\n", phaseName.c_str());
    currProgress = 0;
    jsonReply["msg"] = "ack";
    jsonReply["phn"] = phaseName;
    sendServerMsg(jsonReply);
  }
  else if (cmd == "skp") {
    Serial.printf("Skipping phase <%d>\n", currPhase);
    currProgress = 0;
    currPhase++;
    if (currPhase > 3) {
      currPhase = 0;
      currPlanState = PLAN_STATE_ENDED;
    }
    jsonReply["msg"] = "ack";
    jsonReply["pln"] = currPlanName;
    jsonReply["phn"] = currPhase;
    sendServerMsg(jsonReply);
  }
  else {
    Serial.println("Usupported command");
  }
}

// set the current timestamp, converting the uLong into a 4-byte array
void setTimestamp(unsigned long ulTimestamp) {
  //Serial.printf("setTimestamp(): %lu\n", ulTimestamp);
  uint8_t bytesTimestamp [4];
  uLongToFourBytes(ulTimestamp, bytesTimestamp);
  pJacketCharactTimestamp->setValue(bytesTimestamp, 4);
}

// set the current plan name
void setPlanName(String & strPlanName) {
  pJacketCharactPlanName->setValue((uint8_t*)strPlanName.c_str(), strPlanName.length());
}

// set the current state chracteristic, packing 5 bytes
void setState(uint8_t planState, uint8_t phase, uint8_t progress, uint8_t ctrlMode, uint8_t battLevel) {
  //Serial.printf("setState() (decimal): %d %d %d %d\n", progType, progState, ctrlMode, battLevel);
  uint8_t byteValue [5] = {
    planState,
    phase,
    progress,
    ctrlMode,
    battLevel
  };
  pJacketCharactState->setValue(byteValue, 5);
}
