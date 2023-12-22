#ifndef MifareClassic_h
#define MifareClassic_h
#include <ndef_config.h>

#if NDEF_SUPPORT_MIFARE_CLASSIC



#define BLOCK_SIZE 16
#define LONG_TLV_SIZE 4
#define SHORT_TLV_SIZE 2

#include <Due.h>
#include <MFRC522v2.h>
#include <MFRC522Debug.h>
#include <MFRC522Extended.h>
#include <Ndef.h>
#include <NfcTag.h>

class MifareClassic
{
public:
    using StatusCode = MFRC522Constants::StatusCode;
    using PICC_Type = MFRC522Constants::PICC_Type;

    MifareClassic(MFRC522Extended *nfcShield);
    ~MifareClassic();
    NfcTag read();
    bool write(NdefMessage &ndefMessage);
    bool formatNDEF();
    bool formatMifare();

private:
    using TagInfo = MFRC522Extended::TagInfo;

    MFRC522Extended *_nfc;
    int getBufferSize(int messageLength);
    int getNdefStartIndex(byte *data);
    bool decodeTlv(byte *data, int *messageLength, int *messageStartIndex);
};

#endif
#endif
