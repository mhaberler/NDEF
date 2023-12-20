#ifndef NfcAdapter_h
#define NfcAdapter_h

#include <MFRC522v2.h>
#include <MFRC522Extended.h>
#include <NfcTag.h>
#include <Ndef.h>

// Drivers
#include <MifareClassic.h>
#include <MifareUltralight.h>
#include <Type4Tag.h>

//#define NDEF_DEBUG 1

class NfcAdapter {
    using StatusCode = MFRC522Constants::StatusCode;
    using PICC_Command = MFRC522Constants::PICC_Command;
    using PICC_Type = MFRC522Constants::PICC_Type;
    public:
        NfcAdapter(MFRC522Extended *interface);

        ~NfcAdapter(void);
        void begin(bool verbose=true);
        bool tagPresent(); // tagAvailable
        NfcTag read();
        bool write(NdefMessage& ndefMessage);
        // erase tag by writing an empty NDEF record
        bool erase();
        // format a tag as NDEF
        bool format();
        // reset tag back to factory state
        bool clean();
        void haltTag();
    private:
        MFRC522Extended* shield;
        NfcTag::TagType guessTagType();
};

#endif
