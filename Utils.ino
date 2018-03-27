void updateSerial(){
  if (Serial.available()) {
    DynamicJsonBuffer jsonBuffer(JSON_BUFF_SIZE);
    JsonObject& jsonObj = jsonBuffer.parseObject(Serial);
    if (jsonObj.success()) {
      storeClientMessage(jsonObj);
    } 
    else {
      Serial.println("updateSerial(): ERROR parsing Json object from serial.");
    }
  }
}

void storeClientMessage(JsonObject& jsonObj) {
  //Serial.println("storeClientMessage(jsonObj)");
  if (hasClientMessage) {
    Serial.printf("storeClientMessage WARNING previous unhandled message will be deleted <%s>\n", buffClientMsg);  
  }
  hasClientMessage = true;

  int copySize = CLIENT_MSG_BUFF_SIZE;
  int jsonLength = jsonObj.measureLength() +1;
  if (jsonLength < CLIENT_MSG_BUFF_SIZE) copySize = jsonLength;
  jsonObj.printTo(buffClientMsg, copySize);
  //Serial.printf("storeClientMessage stored <%s>\n", buffClientMsg);
}

void storeClientMessage(std::string & message) {
  //Serial.println("storeClientMessage(string)");
  if (hasClientMessage) {
    Serial.printf("storeClientMessage WARNING previous unhandled message will be deleted <%s>\n", buffClientMsg);  
  }
  hasClientMessage = true;
  
  int copySize = CLIENT_MSG_BUFF_SIZE;
  if (message.length() < CLIENT_MSG_BUFF_SIZE) copySize = message.length();
  memset(buffClientMsg, 0, CLIENT_MSG_BUFF_SIZE);
  memcpy(buffClientMsg, message.c_str(), copySize);
  //Serial.printf("storeClientMessage stored <%s>\n", buffClientMsg);
}

void uLongToFourBytes (unsigned long uLongValue, uint8_t * fourBytes) {
  fourBytes[0] = (uint8_t)(uLongValue);
  fourBytes[1] = (uint8_t)(uLongValue >> 8);
  fourBytes[2] = (uint8_t)(uLongValue >> 16);
  fourBytes[3] = (uint8_t)(uLongValue >> 24);
}
