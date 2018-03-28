#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEUUID.h>


// forward declaration, so we can call the function in setup() or loop()
// before the actual body is declared (the function's body is in another tab)
void setupJacketBLE();
void BLEwrap();

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



