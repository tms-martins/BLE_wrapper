#define DEVICE_NAME              "Smart Jacket Test"

#define UUID_SERVICE             "E708EB00-AD98-4158-B7C2-A748744694AB"
#define UUID_CHARACT_TIMESTAMP   "E708EB01-AD98-4158-B7C2-A748744694AB"
#define UUID_CHARACT_PLAN_NAME   "E708EB02-AD98-4158-B7C2-A748744694AB"
#define UUID_CHARACT_STATE       "E708EB03-AD98-4158-B7C2-A748744694AB"
#define UUID_CHARACT_CLIENT_CMD  "E708EB04-AD98-4158-B7C2-A748744694AB"
#define UUID_CHARACT_SERVER_MSG  "E708EB05-AD98-4158-B7C2-A748744694AB"


///////////////////////////////////////////////////////////////////////////////////////////
/// CALLBACKS /////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

class JacketServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connected to client");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnected from client");
      deviceConnected = false;
    }
};

class JacketCharactCallbacks : public BLECharacteristicCallbacks {
    /*
    char * receiveBuff;
    int receiveBuffSize;

    JacketCharactCallbacks(char * receiveBuff, int receiveBuffSize) {
      this->receiveBuff = receiveBuff;
      this->receiveBuffSize = receiveBuffSize;
    }
    */
  
    void onRead(BLECharacteristic *pCharacteristic) {
      Serial.print("S");
      //Serial.printf("onRead(): [%s]\n", pCharacteristic->getUUID().toString().c_str());
    };

    void onWrite(BLECharacteristic *pCharacteristic) {
      Serial.printf("onWrite(): [%s]\n", pCharacteristic->getUUID().toString().c_str());

      if (pCharacteristic == pJacketCharactClientCmd) {
        std::string stringValue = pCharacteristic->getValue();
        storeClientMessage(stringValue);
      }
      else {
        Serial.println("JacketCharactCallbacks::onWrite() ERROR: no case defined for this characteristic");
      }
    }
};

JacketServerCallbacks  jacketServerCallbacks;
JacketCharactCallbacks jacketCharactCallbacks;

///////////////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS /////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Convenience method to handle creation of characteristics
BLECharacteristic * createCharacteristic (
  BLEService* pService,
  const char* strUUID,
  uint32_t    properties,
  BLECharacteristicCallbacks *pCallbacks,
  BLEDescriptor *pDescriptor
  ) {

  BLECharacteristic* pCharacteristic = new BLECharacteristic(BLEUUID(strUUID), properties);

  if (pDescriptor != NULL)
    pCharacteristic->addDescriptor(pDescriptor);

  if (pCallbacks != NULL)
    pCharacteristic->setCallbacks(pCallbacks);

  pService->addCharacteristic(pCharacteristic);

  return pCharacteristic;
}

void setupJacketBLE() {
  // Create the BLE Device
  BLEDevice::init(DEVICE_NAME);

  // Create the BLE Server
  pJacketServer = BLEDevice::createServer();
  pJacketServer->setCallbacks(&jacketServerCallbacks);

  // Create the BLE Service
  pJacketService = pJacketServer->createService(UUID_SERVICE);

  // Create BLE Characteristics using
  //
  // createCharacteristic(
  //   service,
  //   UUID,
  //   property,
  //   callbacks, (can be NULL)
  //   descriptor (can be NULL)
  // );

  pJacketCharactServerMsg = createCharacteristic(
    pJacketService,
    UUID_CHARACT_SERVER_MSG,
    BLECharacteristic::PROPERTY_NOTIFY,
    &jacketCharactCallbacks,
    new BLE2902()
  );

  pJacketCharactPlanName = createCharacteristic(
    pJacketService,
    UUID_CHARACT_PLAN_NAME,
    BLECharacteristic::PROPERTY_READ,
    &jacketCharactCallbacks,
    new BLE2902()
  );

  pJacketCharactState = createCharacteristic(
    pJacketService,
    UUID_CHARACT_STATE,
    BLECharacteristic::PROPERTY_READ,
    &jacketCharactCallbacks,
    new BLE2902()
  );

  pJacketCharactClientCmd = createCharacteristic(
    pJacketService,
    UUID_CHARACT_CLIENT_CMD,
    BLECharacteristic::PROPERTY_WRITE,
    &jacketCharactCallbacks,
    new BLE2902()
  );

  pJacketCharactTimestamp = createCharacteristic(
    pJacketService,
    UUID_CHARACT_TIMESTAMP,
    BLECharacteristic::PROPERTY_READ,
    &jacketCharactCallbacks,
    new BLE2902()
  );

  // Start the service
  pJacketService->start();

  // Start advertising
  pJacketServer->getAdvertising()->start();
}
