
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

  Serial.begin(115200);

  setupJacketBLE();

}

void loop() {

  BLEwrap();

}



