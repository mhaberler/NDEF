#ifndef MifareUltralight_h
#define MifareUltralight_h

#include <MFRC522.h>
#include <NfcTag.h>
#include <Ndef.h>

#define MIFARE_ULTRALIGHT_DEBUG 1

#define ULTRALIGHT_PAGE_SIZE 4
#define ULTRALIGHT_READ_SIZE 16

#define ULTRALIGHT_DATA_START_PAGE 4
#define ULTRALIGHT_MESSAGE_LENGTH_INDEX 1
#define ULTRALIGHT_DATA_START_INDEX 2
#define ULTRALIGHT_MAX_PAGE 63

#define NFC_FORUM_TAG_TYPE_2 ("NFC Forum Type 2")

class MifareUltralight
{
    public:
        MifareUltralight(MFRC522 *nfcShield);
        ~MifareUltralight();
        NfcTag read();
        boolean write(NdefMessage& ndefMessage);
        boolean clean();
    private:
        MFRC522 *nfc;
        unsigned int tagCapacity;
        unsigned int messageLength;
        unsigned int bufferSize;
        unsigned int ndefStartIndex;
        boolean isUnformatted();
        void readCapabilityContainer();
        void findNdefMessage();
        void calculateBufferSize();
};

#endif
