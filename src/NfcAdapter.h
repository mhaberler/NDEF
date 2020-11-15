#ifndef NfcAdapter_h
#define NfcAdapter_h

#include <MFRC522.h>
#include <NfcTag.h>
#include <Ndef.h>

// Drivers
#include <MifareClassic.h>
#include <MifareUltralight.h>

class NfcAdapter {
    public:
        NfcAdapter(MFRC522 *interface);

        ~NfcAdapter(void);
        void begin(boolean verbose=true);
        boolean tagPresent(); // tagAvailable
        NfcTag read();
        boolean write(NdefMessage& ndefMessage);
        // erase tag by writing an empty NDEF record
        boolean erase();
        // format a tag as NDEF
        boolean format();
        // reset tag back to factory state
        boolean clean();
    private:
        MFRC522* shield;
        NfcTag::TagType guessTagType();
};

#endif
