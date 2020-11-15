#ifndef NfcAdapter_h
#define NfcAdapter_h

#include <NfcTag.h>
#include <Ndef.h>
#include <MFRC522.h>

// Drivers
#include <MifareClassic.h>
#include <MifareUltralight.h>

#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

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
