#include <NfcAdapter.h>

NfcAdapter::NfcAdapter(MFRC522Extended *interface)
{
    shield = interface;
}

NfcAdapter::~NfcAdapter(void)
{
}

void NfcAdapter::begin(bool verbose)
{
#if NDEF_USE_SERIAL
    if (verbose)
    {
        MFRC522Debug::PCD_DumpVersionToSerial(*shield, Serial);
    }
#endif
}

bool NfcAdapter::tagPresent()
{
    // If tag has already been authenticated nothing else will work until we stop crypto (shouldn't hurt)
    shield->PCD_StopCrypto1();

    if(!(shield->PICC_IsNewCardPresent() && shield->PICC_ReadCardSerial()))
    {
        return false;
    }

    MFRC522::PICC_Type piccType = shield->PICC_GetType(shield->uid.sak);
    switch (piccType) {
        case PICC_Type::PICC_TYPE_MIFARE_1K:
        case PICC_Type::PICC_TYPE_MIFARE_UL:
        case PICC_Type::PICC_TYPE_ISO_14443_4:
        return true;
        default:;
    }
    return false;
}

bool NfcAdapter::erase()
{
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return write(message);
}

bool NfcAdapter::format()
{
#if NDEF_SUPPORT_MIFARE_CLASSIC
    if(shield->PICC_GetType(shield->uid.sak) == PICC_Type::PICC_TYPE_MIFARE_1K)
    {
        MifareClassic mifareClassic = MifareClassic(shield);
        return mifareClassic.formatNDEF();
    }
    else
#endif
    if(shield->PICC_GetType(shield->uid.sak) == PICC_Type::PICC_TYPE_MIFARE_UL)
    {
#if NDEF_USE_SERIAL
        Serial.print(F("No need for formating a UL"));
#endif
        return true;
    }
    else
    {
#if NDEF_USE_SERIAL
        Serial.print(F("Unsupported Tag."));
#endif
        return false;
    }
}

bool NfcAdapter::clean()
{
    NfcTag::PICC_Type type = guessTagType();

#if NDEF_SUPPORT_MIFARE_CLASSIC
    if (type == PICC_Type::PICC_TYPE_MIFARE_1K) // FIXME
    {
        #if NDEF_DEBUG
        Serial.println(F("Cleaning Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(shield);
        return mifareClassic.formatMifare();
    }
    else
#endif
    if (type == PICC_Type::PICC_TYPE_MIFARE_UL) // FIXME
    {
        #if NDEF_DEBUG
        Serial.println(F("Cleaning Mifare Ultralight"));
        #endif
        MifareUltralight ultralight = MifareUltralight(shield);
        return ultralight.clean();
    }
    else
    {
#if NDEF_USE_SERIAL
        Serial.print(F("No driver for card type "));Serial.println(type);
#endif
        return false;
    }

}

NfcTag NfcAdapter::read()
{
    uint8_t type = guessTagType();

#if NDEF_SUPPORT_MIFARE_CLASSIC
    if (type == PICC_Type::PICC_TYPE_MIFARE_1K)
    {
        #if NDEF_DEBUG
        Serial.println(F("Reading Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(shield);
        return mifareClassic.read();
    }
    else
#endif
    if (type == PICC_Type::PICC_TYPE_MIFARE_UL)
    {
        #if NDEF_DEBUG
        Serial.println(F("Reading Mifare Ultralight"));
        #endif
        MifareUltralight ultralight = MifareUltralight(shield);
        return ultralight.read();
    }
    else if (type == PICC_Type::PICC_TYPE_ISO_14443_4)
    {
        #if NDEF_DEBUG
        Serial.println(F("Reading Type4 tag"));
        #endif
        Type4Tag type4tag = Type4Tag(shield);
        return type4tag.read();
    }
    else if (type == PICC_Type::PICC_TYPE_UNKNOWN)
    {
#if NDEF_USE_SERIAL
        Serial.print(F("Can not determine tag type"));
#endif
        return NfcTag(shield->uid, PICC_Type::PICC_TYPE_UNKNOWN);
    }
    else
    {
        // Serial.print(F("No driver for card type "));Serial.println(type);
        // TODO should set type here
        return NfcTag(shield->uid, PICC_Type::PICC_TYPE_UNKNOWN);
    }

}

bool NfcAdapter::write(NdefMessage& ndefMessage)
{
    PICC_Type type = guessTagType();

#if NDEF_SUPPORT_MIFARE_CLASSIC
    if (type == PICC_Type::PICC_TYPE_MIFARE_1K) // FIXME
    {
        #if NDEF_DEBUG
        Serial.println(F("Writing Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(shield);
        return mifareClassic.write(ndefMessage);
    }
    else
#endif
    if (type == PICC_Type::PICC_TYPE_MIFARE_UL)
    {
        #if NDEF_DEBUG
        Serial.println(F("Writing Mifare Ultralight"));
        #endif
        MifareUltralight mifareUltralight = MifareUltralight(shield);
        return mifareUltralight.write(ndefMessage);
    }
    else if (type == PICC_Type::PICC_TYPE_UNKNOWN)
    {
#if NDEF_USE_SERIAL
        Serial.print(F("Can not determine tag type"));
#endif
        return false;
    }
    else
    {
#if NDEF_USE_SERIAL
        Serial.print(F("No driver for card type "));Serial.println(type);
#endif
        return false;
    }
}

// Current tag will not be "visible" until removed from the RFID field
void NfcAdapter::haltTag() {
    shield->PICC_HaltA();
    shield->PCD_StopCrypto1();
}

MFRC522Constants::PICC_Type NfcAdapter::guessTagType()
{

   return shield->PICC_GetType(shield->uid.sak);

    // if (piccType == PICC_Type::PICC_TYPE_MIFARE_1K)
    // {
    //     return NfcTag::PICC_TYPE_MIFARE_1K; // FIXME
    // } 
    // else if (piccType == PICC_Type::PICC_TYPE_MIFARE_UL)
    // {
    //     return NfcTag::TYPE_2;
    // }
    // else if (piccType == PICC_Type::PICC_TYPE_ISO_14443_4)
    // {
    //     return NfcTag::TYPE_4;
    // }
    // else
    // {
    //     return NfcTag::TYPE_UNKNOWN;
    // }
}
