#include <SPI.h>
#include <mcp_can.h>
#include <SdFat.h>

struct CanData {
  long unsigned int canId;
  unsigned char data[8];
  byte length;
};

SdFat sd;
File dataFile;
MCP_CAN CAN(10);
const int chipSelect = 4;
const int greenLed = 9;

void setup() {
  Serial.begin(115200);
  pinMode(chipSelect, OUTPUT);
  pinMode(greenLed, OUTPUT);

  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
    Serial.println("SD-kortet kunde inte initialiseras.");
    return;
  }

  if (CAN.begin(CAN_500KBPS) == CAN_OK) {
    Serial.println("CAN-BUS Shield init ok!");
  } else {
    Serial.println("Fel vid initialisering av CAN-BUS Shield");
  }
}

void loop() {
  CanData canMessage;

  // Transmit CAN message
  sendCanMessage();

  // Receive and process CAN message
  if (CAN_MSGAVAIL == CAN.checkReceive()) { 
    readCanMessage(canMessage); 
    writeDataToSD(canMessage);
    digitalWrite(greenLed, HIGH);
    printCanMessage(canMessage);
    delay(500);
    digitalWrite(greenLed, LOW);
  }
}

void sendCanMessage() {
  unsigned char stmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  stmp[7] = stmp[7] + 1;
  if (stmp[7] == 100) {
    stmp[7] = 0;
    stmp[6] = stmp[6] + 1;
    if (stmp[6] == 100) {
      stmp[6] = 0;
      stmp[5] = stmp[5] + 1;
    }
  }
  CAN.sendMsgBuf(0x00, 0, 8, stmp);
  delay(100);
}

void readCanMessage(CanData &canMessage) {
  byte len = 0;
  CAN.readMsgBuf(&len, canMessage.data);
  canMessage.length = len;
  canMessage.canId = CAN.getCanId();
}

void writeDataToSD(const CanData &canMessage) {
  dataFile = sd.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.print("ID: ");
    dataFile.print(canMessage.canId, HEX);
    dataFile.print(" Data: ");
    for (int i = 0; i < canMessage.length; i++) {
      dataFile.print(canMessage.data[i], HEX);
      dataFile.print(" ");
    }
    dataFile.println();
    dataFile.close();
  } else {
    Serial.println("Kunde inte öppna datalog.txt");
  }
}

void printCanMessage(const CanData &canMessage) {
  Serial.print("ID: ");
  Serial.print(canMessage.canId, HEX);
  Serial.print(" Data: ");
  for (int i = 0; i < canMessage.length; i++) {
    Serial.print(canMessage.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
