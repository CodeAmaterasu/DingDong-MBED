#include "Rfid.h"
#include "mbed.h"
#include "MFRC522.h"

MFRC522 rfidReader(MBED_CONF_IOTKIT_RFID_MOSI, MBED_CONF_IOTKIT_RFID_MISO, MBED_CONF_IOTKIT_RFID_SCLK, MBED_CONF_IOTKIT_RFID_SS, MBED_CONF_IOTKIT_RFID_RST);

void Rfid::writeData(string *hashData) {
    // TODO: Implement stuff
    rfidReader.PCD_Init();
}

string* Rfid::readData() {
    // TODO: Implement Stuff
    return nullptr;
}

bool Rfid::accessRfidChip(string* hashData, int accessMode) {
    // TODO: Implement stuff
    return true;
}