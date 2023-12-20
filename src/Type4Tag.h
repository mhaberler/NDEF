#pragma once

#include <MFRC522Extended.h>
#include <MFRC522v2.h>
#include <Ndef.h>
#include <NfcTag.h>

using StatusCode = MFRC522Constants::StatusCode;

class Type4Tag
{

  public:
    Type4Tag (MFRC522Extended *nfcShield);
    ~Type4Tag ();
    NfcTag read ();

  private:
    MFRC522Extended *nfc;
    StatusCode SelectNdefTag_Application ();
    StatusCode SelectCapabilityContainer ();
    StatusCode readCCFile (uint16_t &fileId, uint16_t &maxSize,
                           uint8_t &readAccess, uint8_t &writeAccess);
    StatusCode selectFile (const uint16_t fileId);
    StatusCode readFileLength (uint16_t &fileSize);
    StatusCode readFile (uint8_t *response, const size_t fileLength);
};
