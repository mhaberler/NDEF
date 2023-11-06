// Erases a NFC tag by writing an empty NDEF message

#include <SPI.h>
#include <MFRC522.h>
#include "NfcAdapter.h"

#define SS_PIN 5

// MFRC522 setup
MFRC522DriverPinSimple ss_pin(SS_PIN); // Configurable, see typical pin layout above.

MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.

MFRC522 mfrc522{driver}; // Create MFRC522 instance.

NfcAdapter nfc = NfcAdapter(&mfrc522);

void setup(void)
{
    Serial.begin(9600);
    Serial.println("NFC Tag Eraser\nPlace a tag on the NFC reader to erase.");
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522
    nfc.begin();
}

void loop(void)
{
    if (nfc.tagPresent())
    {
        Serial.println("Erasing tag");
        bool success = nfc.erase();
        if (success)
        {
            Serial.println("\tSuccess, tag contains an empty record.");
            delay(10000);
        }
        else
        {
            Serial.println("\tUnable to erase tag.");
        }
    }
    delay(5000);
}
