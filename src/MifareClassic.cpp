#include "MifareClassic.h"
#if NDEF_SUPPORT_MIFARE_CLASSIC

static MFRC522::MIFARE_Key default_key = {{0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7}};

MifareClassic::MifareClassic(MFRC522Extended *nfcShield)
{
    _nfc = nfcShield;
}

MifareClassic::~MifareClassic()
{
}

NfcTag MifareClassic::read(MFRC522::MIFARE_Key *key)
{
    if (key == NULL) {
        key = &default_key;
    }
    int messageStartIndex = 0;
    int messageLength = 0;
    byte dataSize = BLOCK_SIZE + 2;
    byte data[dataSize];
    MFRC522::StatusCode sc = _nfc->PCD_Authenticate(MFRC522Constants::PICC_CMD_MF_AUTH_KEY_A, 4, key, &(_nfc->tag.uid));

    // read first block to get message length
    if (sc == MFRC522Constants::STATUS_OK)
    {
        if ((sc = _nfc->MIFARE_Read(4, data, &dataSize)) != MFRC522Constants::STATUS_OK)
        {
#if NDEF_USE_SERIAL
            Serial.println(F("Error. Failed read block 4"));
#endif
            return NfcTag(_nfc->tag, PICC_Type::PICC_TYPE_MIFARE_1K);
        }

        if (!decodeTlv(data, &messageLength, &messageStartIndex))
        {
#if NDEF_USE_SERIAL
            Serial.println(F("Error. Could not decode TLV"));
#endif
            return NfcTag(_nfc->tag, PICC_Type::PICC_TYPE_UNKNOWN); // TODO should the error message go in NfcTag?
        }
    }
    else
    {
#if NDEF_USE_SERIAL
        Serial.println(F("Tag is not NDEF formatted."));
#endif
//FIXME
        return NfcTag(_nfc->tag, PICC_Type::PICC_TYPE_MIFARE_1K, false);
    }

    int currentBlock = 4;
    // this should be nested in the message length loop
    int index = 0;
    // Add 2 to allow MFRC522 to add CRC
    int bufferSize = getBufferSize(messageLength) + 2;
    uint8_t buffer[bufferSize];

#if MIFARE_CLASSIC_DEBUG
    Serial.print(F("Message Length "));
    Serial.println(messageLength);
    Serial.print(F("Buffer Size "));
    Serial.println(bufferSize);
#endif

    while (index < bufferSize - 2)
    {

        // authenticate on every sector
        if (((currentBlock < 128) && (currentBlock % 4 == 0)) || ((currentBlock >= 128) && (currentBlock % 16 == 0)))
        {

            if ((sc = _nfc->PCD_Authenticate(MFRC522Constants::PICC_CMD_MF_AUTH_KEY_A, currentBlock, key, &(_nfc->tag.uid))) != MFRC522Constants::STATUS_OK)
            {
#if NDEF_USE_SERIAL
                Serial.print(F("Error. Block Authentication failed for "));
                Serial.println(currentBlock);
#endif
                // TODO Nicer error handling
                return NfcTag(_nfc->tag, PICC_Type::PICC_TYPE_MIFARE_1K); //FIXME
            }
        }

        // read the data
        byte readBufferSize = 18;
        if ((sc = _nfc->MIFARE_Read(currentBlock, &buffer[index], &readBufferSize)) == MFRC522Constants::STATUS_OK)
        {
#if MIFARE_CLASSIC_DEBUG
            Serial.print(F("Block "));
            Serial.print(currentBlock);
            Serial.print(" ");
            PrintHexChar(&buffer[index], BLOCK_SIZE);
#endif
        }
        else
        {
#if NDEF_USE_SERIAL
            Serial.print(F("Read failed "));
            Serial.println(currentBlock);
#endif
            // TODO Nicer error handling
            return NfcTag(_nfc->tag, PICC_Type::PICC_TYPE_MIFARE_1K); //FIXME
        }

        index += BLOCK_SIZE;
        currentBlock++;

        // skip the trailer block
        if (((currentBlock < 128) && ((currentBlock + 1) % 4 == 0)) || ((currentBlock >= 128) && ((currentBlock + 1) % 16 == 0)))
        {
#if MIFARE_CLASSIC_DEBUG
            Serial.print(F("Skipping block "));
            Serial.println(currentBlock);
#endif
            currentBlock++;
        }
    }
//FIXME
    return NfcTag(_nfc->tag, PICC_Type::PICC_TYPE_MIFARE_1K, &buffer[messageStartIndex], messageLength);
}

int MifareClassic::getBufferSize(int messageLength)
{

    int bufferSize = messageLength;

    // TLV header is 2 or 4 bytes, TLV terminator is 1 byte.
    if (messageLength < 0xFF)
    {
        bufferSize += SHORT_TLV_SIZE + 1;
    }
    else
    {
        bufferSize += LONG_TLV_SIZE + 1;
    }

    // bufferSize needs to be a multiple of BLOCK_SIZE
    if (bufferSize % BLOCK_SIZE != 0)
    {
        bufferSize = ((bufferSize / BLOCK_SIZE) + 1) * BLOCK_SIZE;
    }

    return bufferSize;
}

// skip null tlvs (0x0) before the real message
// technically unlimited null tlvs, but we assume
// T & L of TLV in the first block we read
int MifareClassic::getNdefStartIndex(byte *data)
{

    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        if (data[i] == 0x0)
        {
            // do nothing, skip
        }
        else if (data[i] == 0x3)
        {
            return i;
        }
        else
        {
#if NDEF_USE_SERIAL
            Serial.print("Unknown TLV ");
            Serial.println(data[i], HEX);
#endif
            return -2;
        }
    }

    return -1;
}

// Decode the NDEF data length from the Mifare TLV
// Leading null TLVs (0x0) are skipped
// Assuming T & L of TLV will be in the first block
// messageLength and messageStartIndex written to the parameters
// success or failure status is returned
//
// { 0x3, LENGTH }
// { 0x3, 0xFF, LENGTH, LENGTH }
bool MifareClassic::decodeTlv(byte *data, int *messageLength, int *messageStartIndex)
{
    int i = getNdefStartIndex(data);

    if (i < 0 || data[i] != 0x3)
    {
#if NDEF_USE_SERIAL
        Serial.println(F("Error. Can't decode message length."));
#endif
        return false;
    }
    else
    {
        if (data[i + 1] == 0xFF)
        {
            *messageLength = ((0xFF & data[i + 2]) << 8) | (0xFF & data[i + 3]);
            *messageStartIndex = i + LONG_TLV_SIZE;
        }
        else
        {
            *messageLength = data[i + 1];
            *messageStartIndex = i + SHORT_TLV_SIZE;
        }
    }

    return true;
}

// Intialized NDEF tag contains one empty NDEF TLV 03 00 FE - AN1304 6.3.1
// We are formatting in read/write mode with a NDEF TLV 03 03 and an empty NDEF record D0 00 00 FE - AN1304 6.3.2
bool MifareClassic::formatNDEF()
{
    MFRC522::MIFARE_Key keya = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
    byte emptyNdefMesg[16] = {0x03, 0x03, 0xD0, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    byte blockbuffer0[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    byte blockbuffer1[16] = {0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
    byte blockbuffer2[16] = {0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
    byte blockbuffer3[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    byte blockbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // TODO use UID from method parameters?
    MFRC522Constants::StatusCode status = _nfc->PCD_Authenticate(MFRC522Constants::PICC_CMD_MF_AUTH_KEY_A, 1, &keya, &(_nfc->tag.uid));
    if (status != MFRC522Constants::STATUS_OK)
    {
#if NDEF_USE_SERIAL
        Serial.println(F("Unable to authenticate block 1 to enable card formatting!"));
        Serial.print(MFRC522Debug::GetStatusCodeName(status));
        Serial.println();
#endif
        return false;
    }

    if (_nfc->MIFARE_Write(1, blockbuffer1, 16) != MFRC522Constants::STATUS_OK)
    {
#if NDEF_USE_SERIAL
        Serial.println(F("Unable to format the card for NDEF: Block 1 failed"));
#endif
        return false;
    }

    if (_nfc->MIFARE_Write(2, blockbuffer2, 16) != MFRC522Constants::STATUS_OK)
    {
#if NDEF_USE_SERIAL
        Serial.println(F("Unable to format the card for NDEF: Block 2 failed"));
#endif
        return false;
    }
    // Write new key A and permissions
    if (_nfc->MIFARE_Write(3, blockbuffer3, 16) != MFRC522Constants::STATUS_OK)
    {
#if NDEF_USE_SERIAL
        Serial.println(F("Unable to format the card for NDEF: Block 3 failed"));
#endif
        return false;
    }
    for (int i = 4; i < 64; i += 4)
    {
        if (_nfc->PCD_Authenticate(MFRC522Constants::PICC_CMD_MF_AUTH_KEY_A, i, &keya, &(_nfc->tag.uid)) != MFRC522Constants::STATUS_OK)
        {
#if NDEF_USE_SERIAL
            Serial.print(F("Unable to authenticate block "));
            Serial.println(i);
#endif
            return false;
        }

        if (i == 4) // special handling for block 4
        {
            if (_nfc->MIFARE_Write(i, emptyNdefMesg, 16) != MFRC522Constants::STATUS_OK)
            {
#if NDEF_USE_SERIAL
                Serial.print(F("Unable to write block "));
                Serial.println(i);
#endif
                return false;
            }
        }
        else
        {
            if (_nfc->MIFARE_Write(i, blockbuffer0, 16) != MFRC522Constants::STATUS_OK)
            {
#if NDEF_USE_SERIAL
                Serial.print(F("Unable to write block "));
                Serial.println(i);
#endif
                return false;
            }
        }
        if (_nfc->MIFARE_Write(i + 1, blockbuffer0, 16) != MFRC522Constants::STATUS_OK)
        {
#if NDEF_USE_SERIAL
            Serial.print(F("Unable to write block "));
            Serial.println(i + 1);
#endif
            return false;
        }
        if (_nfc->MIFARE_Write(i + 2, blockbuffer0, 16) != MFRC522Constants::STATUS_OK)
        {
#if NDEF_USE_SERIAL
            Serial.print(F("Unable to write block "));
            Serial.println(i + 2);
#endif
            return false;
        }
        if (_nfc->MIFARE_Write(i + 3, blockbuffer4, 16) != MFRC522Constants::STATUS_OK)
        {
#if NDEF_USE_SERIAL
            Serial.print(F("Unable to write block "));
            Serial.println(i + 3);
#endif
            return false;
        }
    }
    return true;
}

#define NR_SHORTSECTOR (32)         // Number of short sectors on Mifare 1K/4K
#define NR_LONGSECTOR (8)           // Number of long sectors on Mifare 4K
#define NR_BLOCK_OF_SHORTSECTOR (4) // Number of blocks in a short sector
#define NR_BLOCK_OF_LONGSECTOR (16) // Number of blocks in a long sector

// Determine the sector trailer block based on sector number
#define BLOCK_NUMBER_OF_SECTOR_TRAILER(sector) (((sector) < NR_SHORTSECTOR) ? ((sector) * NR_BLOCK_OF_SHORTSECTOR + NR_BLOCK_OF_SHORTSECTOR - 1) : (NR_SHORTSECTOR * NR_BLOCK_OF_SHORTSECTOR + (sector - NR_SHORTSECTOR) * NR_BLOCK_OF_LONGSECTOR + NR_BLOCK_OF_LONGSECTOR - 1))

bool MifareClassic::formatMifare()
{

    // The default Mifare Classic key
    MFRC522::MIFARE_Key KEY_DEFAULT_KEYAB = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
    byte emptyBlock[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    byte authBlock[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    uint8_t idx = 0;
    uint8_t numOfSector = 16; // Assume Mifare Classic 1K for now (16 4-block sectors)

    for (idx = 0; idx < numOfSector; idx++)
    {
        // Step 1: Authenticate the current sector using key B 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
        if (_nfc->PCD_Authenticate(MFRC522Constants::PICC_CMD_MF_AUTH_KEY_B, BLOCK_NUMBER_OF_SECTOR_TRAILER(idx), &KEY_DEFAULT_KEYAB, &(_nfc->tag.uid)) != MFRC522Constants::STATUS_OK)
        {
#if NDEF_USE_SERIAL
            Serial.print(F("Authentication failed for sector "));
            Serial.println(idx);
#endif
            return false;
        }

        // Step 2: Write to the other blocks
        if (idx == 0)
        {
            if (_nfc->MIFARE_Write((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, emptyBlock, 16) != MFRC522Constants::STATUS_OK)
            {
#if NDEF_USE_SERIAL
                Serial.print(F("Unable to write to sector "));
                Serial.println(idx);
#endif
            }
        }
        else
        {
            // this block has not to be overwritten for block 0. It contains Tag id and other unique data.
            if (_nfc->MIFARE_Write((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, emptyBlock, 16) != MFRC522Constants::STATUS_OK)
            {
#if NDEF_USE_SERIAL
                Serial.print(F("Unable to write to sector "));
                Serial.println(idx);
#endif
            }
            if (_nfc->MIFARE_Write((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, emptyBlock, 16) != MFRC522Constants::STATUS_OK)
            {
#if NDEF_USE_SERIAL
                Serial.print(F("Unable to write to sector "));
                Serial.println(idx);
#endif
            }
        }

        if (_nfc->MIFARE_Write((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 1, emptyBlock, 16) != MFRC522Constants::STATUS_OK)
        {
#if NDEF_USE_SERIAL
            Serial.print(F("Unable to write to sector "));
            Serial.println(idx);
#endif
        }

        // Write the trailer block
        if (_nfc->MIFARE_Write((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)), authBlock, 16) != MFRC522Constants::STATUS_OK)
        {
#if NDEF_USE_SERIAL
            Serial.print(F("Unable to write trailer block of sector "));
            Serial.println(idx);
#endif
        }
    }
    return true;
}

bool MifareClassic::write(NdefMessage &m)
{

    uint8_t encoded[m.getEncodedSize()];
    m.encode(encoded);

    uint8_t buffer[getBufferSize(sizeof(encoded))];
    memset(buffer, 0, sizeof(buffer));

#if MIFARE_CLASSIC_DEBUG
    Serial.print(F("sizeof(encoded) "));
    Serial.println(sizeof(encoded));
    Serial.print(F("sizeof(buffer) "));
    Serial.println(sizeof(buffer));
#endif

    if (sizeof(encoded) < 0xFF)
    {
        buffer[0] = 0x3;
        buffer[1] = sizeof(encoded);
        memcpy(&buffer[2], encoded, sizeof(encoded));
        buffer[2 + sizeof(encoded)] = 0xFE; // terminator
    }
    else
    {
        buffer[0] = 0x3;
        buffer[1] = 0xFF;
        buffer[2] = ((sizeof(encoded) >> 8) & 0xFF);
        buffer[3] = (sizeof(encoded) & 0xFF);
        memcpy(&buffer[4], encoded, sizeof(encoded));
        buffer[4 + sizeof(encoded)] = 0xFE; // terminator
    }

    // Write to tag
    unsigned int index = 0;
    byte currentBlock = 4;
    MFRC522::MIFARE_Key key = {{0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7}};

    while (index < sizeof(buffer))
    {

        if (((currentBlock < 128) && (currentBlock % 4 == 0)) || ((currentBlock >= 128) && (currentBlock % 16 == 0)))
        {
            MFRC522::StatusCode status = _nfc->PCD_Authenticate(MFRC522Constants::PICC_CMD_MF_AUTH_KEY_A, currentBlock, &key, &(_nfc->tag.uid));
            if (status != MFRC522Constants::STATUS_OK)
            {
#if NDEF_USE_SERIAL
                Serial.print(F("Error. Block authentication failed for block "));
                Serial.println(currentBlock);
                Serial.print(MFRC522Debug::GetStatusCodeName(status));
#endif
                return false;
            }
        }

        if (_nfc->MIFARE_Write(currentBlock, &buffer[index], BLOCK_SIZE) != MFRC522Constants::STATUS_OK)
        {
#if NDEF_USE_SERIAL
            Serial.print(F("Write failed "));
            Serial.println(currentBlock);
#endif
            return false;
        }

#if MIFARE_CLASSIC_DEBUG
        Serial.print(F("Wrote block "));
        Serial.print(currentBlock);
        Serial.print(" - ");
        PrintHexChar(&buffer[index], BLOCK_SIZE);
#endif

        index += BLOCK_SIZE;
        currentBlock++;

        if (((currentBlock < 128) && ((currentBlock + 1) % 4 == 0)) || ((currentBlock >= 128) && ((currentBlock + 1) % 16 == 0)))
        {
// can't write to trailer block
#if MIFARE_CLASSIC_DEBUG
            Serial.print(F("Skipping block "));
            Serial.println(currentBlock);
#endif
            currentBlock++;
        }
    }

    return true;
}
#endif
