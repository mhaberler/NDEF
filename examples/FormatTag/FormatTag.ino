// Formats a Mifare Classic tags as an NDEF tag
// This will fail if the tag is already formatted NDEF
// nfc.clean will turn a NDEF formatted Mifare Classic tag back to the Mifare Classic format

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
  Serial.println("NDEF Formatter\nPlace an unformatted Mifare Classic tag on the reader.");
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  nfc.begin();
}

void loop(void)
{
  if (nfc.tagPresent())
  {
    Serial.println("Formatting tag");
    bool success = nfc.format();
    if (success)
    {
      Serial.println("\tSuccess, tag formatted as NDEF.");
      delay(10000);
    }
    else
    {
      Serial.println("\tFormat failed, card may already be formatted.");
    }
  }
  delay(5000);
}
