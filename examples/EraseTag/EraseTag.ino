// Erases a NFC tag by writing an empty NDEF message 

#include <SPI.h>
#include <MFRC522.h>
#include "NfcAdapter.h"

#define SS_PIN 8

MFRC522 mfrc522(SS_PIN, UINT8_MAX); // Create MFRC522 instance

NfcAdapter nfc = NfcAdapter(&mfrc522);

void setup(void) {
    Serial.begin(9600);
    Serial.println("NFC Tag Eraser");
    nfc.begin();
}

void loop(void) {
    Serial.println("\nPlace a tag on the NFC reader to erase.");

    if (nfc.tagPresent()) {

        bool success = nfc.erase();
        if (success) {
            Serial.println("\nSuccess, tag contains an empty record.");        
        } else {
            Serial.println("\nUnable to erase tag.");
        }

    }
    delay(5000);
}
