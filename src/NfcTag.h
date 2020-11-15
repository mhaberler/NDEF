#ifndef NfcTag_h
#define NfcTag_h

#include <inttypes.h>
#include <Arduino.h>
#include <NdefMessage.h>

class NfcTag
{
    public:
        enum TagType { TYPE_MIFARE_CLASSIC = 0, TYPE_1, TYPE_2, TYPE_3, TYPE_4, TYPE_UNKNOWN = 99 };
        NfcTag();
        NfcTag(byte *uid, unsigned int uidLength);
        NfcTag(byte *uid, unsigned int uidLength, TagType tagType);
        NfcTag(byte *uid, unsigned int uidLength, TagType tagType, NdefMessage& ndefMessage);
        NfcTag(byte *uid, unsigned int uidLength, TagType tagType, const byte *ndefData, const int ndefDataLength);
        ~NfcTag(void);
        NfcTag& operator=(const NfcTag& rhs);
        uint8_t getUidLength();
        void getUid(byte *uid, unsigned int uidLength);
        String getUidString();
        TagType getTagType();
        boolean hasNdefMessage();
        NdefMessage getNdefMessage();
#ifdef NDEF_USE_SERIAL
        void print();
#endif
    private:
        byte *_uid;
        unsigned int _uidLength;
        TagType _tagType; // Mifare Classic, NFC Forum Type {1,2,3,4}, Unknown
        NdefMessage* _ndefMessage;
        // TODO capacity
        // TODO isFormatted
};

#endif