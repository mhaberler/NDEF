#include <NfcAdapter.h>

NfcAdapter::NfcAdapter(MFRC522 *interface)
{
    shield = interface;
}

NfcAdapter::~NfcAdapter(void)
{
}

void NfcAdapter::begin(boolean verbose)
{
    if (verbose)
    {
#ifdef NDEF_USE_SERIAL
        shield->PCD_DumpVersionToSerial();
#endif
    }
}

boolean NfcAdapter::tagPresent()
{
    return (shield->PICC_IsNewCardPresent() && shield->PICC_ReadCardSerial());
}

boolean NfcAdapter::erase()
{
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return write(message);
}

boolean NfcAdapter::format()
{
    boolean success;
#ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (shield->PICC_GetType(shield->uid.sak) == MFRC522::PICC_TYPE_MIFARE_1K)
    {
        MifareClassic mifareClassic = MifareClassic(shield);
        success = mifareClassic.formatNDEF();
    }
    else
#endif
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("Unsupported Tag."));
#endif
        success = false;
    }
    return success;
}

boolean NfcAdapter::clean()
{
    uint8_t type = guessTagType();

#ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Cleaning Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(shield);
        return mifareClassic.formatMifare();
    }
    else
#endif
    if (type == TAG_TYPE_2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Cleaning Mifare Ultralight"));
        #endif
        MifareUltralight ultralight = MifareUltralight(shield);
        return ultralight.clean();
    }
    else
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("No driver for card type "));Serial.println(type);
#endif
        return false;
    }

}


NfcTag NfcAdapter::read()
{
    uint8_t type = guessTagType();

#ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Reading Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(shield);
        return mifareClassic.read();
    }
    else
#endif
    if (type == TAG_TYPE_2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Reading Mifare Ultralight"));
        #endif
        MifareUltralight ultralight = MifareUltralight(shield);
        return ultralight.read();
    }
    else if (type == TAG_TYPE_UNKNOWN)
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("Can not determine tag type"));
#endif
        return NfcTag(shield->uid.uidByte, shield->uid.size);
    }
    else
    {
        // Serial.print(F("No driver for card type "));Serial.println(type);
        // TODO should set type here
        return NfcTag(shield->uid.uidByte, shield->uid.size);
    }

}

boolean NfcAdapter::write(NdefMessage& ndefMessage)
{
    boolean success;
    uint8_t type = guessTagType();

#ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Writing Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(shield);
        success = mifareClassic.write(ndefMessage);
    }
    else
#endif
    if (type == TAG_TYPE_2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Writing Mifare Ultralight"));
        #endif
        MifareUltralight mifareUltralight = MifareUltralight(shield);
        success = mifareUltralight.write(ndefMessage);
    }
    else if (type == TAG_TYPE_UNKNOWN)
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("Can not determine tag type"));
#endif
        success = false;
    }
    else
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("No driver for card type "));Serial.println(type);
#endif
        success = false;
    }

    return success;
}

// TODO this should return a Driver MifareClassic, MifareUltralight, Type 4, Unknown
// Guess Tag Type by looking at the ATQA and SAK values
// Need to follow spec for Card Identification. Maybe AN1303, AN1305 and ???
unsigned int NfcAdapter::guessTagType()
{

    MFRC522::PICC_Type piccType = shield->PICC_GetType(shield->uid.sak);

    if (piccType == MFRC522::PICC_TYPE_MIFARE_1K)
    {
        return TAG_TYPE_MIFARE_CLASSIC;
    } 
    else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL)
    {
        return TAG_TYPE_2;
    }
    else
    {
        return TAG_TYPE_UNKNOWN;
    }
}
