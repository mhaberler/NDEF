// Shows the content of a Mifare Classic tag formatted as an NDEF tag
// This example requires #define NDEF_USE_SERIAL to be uncommented in Ndef.h

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
    Serial.println("NDEF Reader\nPlace an unformatted Mifare Classic tag on the reader to show contents.");
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522
    nfc.begin();
}

void loop(void)
{
    if (nfc.tagPresent())
    {
        Serial.println("Reading NFC tag\n");
        NfcTag tag = nfc.read();
        tag.print();
    }
    delay(5000);
}
