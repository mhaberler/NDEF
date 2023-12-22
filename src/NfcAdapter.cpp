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

bool
NfcAdapter::tagPresent ()
{
    // If tag has already been authenticated nothing else will work until we
    // stop crypto (shouldn't hurt)
    shield->PCD_StopCrypto1 ();

    if (!(shield->PICC_IsNewCardPresent () && shield->PICC_ReadCardSerial ()))
        {
            return false;
        }

    MFRC522::PICC_Type piccType = shield->PICC_GetType (shield->uid.sak);
    switch (piccType)
        {
        case PICC_Type::PICC_TYPE_ISO_14443_4:
        case PICC_Type::PICC_TYPE_ISO_18092:
        case PICC_Type::PICC_TYPE_MIFARE_MINI:
        case PICC_Type::PICC_TYPE_MIFARE_1K:
        case PICC_Type::PICC_TYPE_MIFARE_4K:
        case PICC_Type::PICC_TYPE_MIFARE_UL:
        case PICC_Type::PICC_TYPE_MIFARE_PLUS:
        case PICC_Type::PICC_TYPE_MIFARE_DESFIRE:
        case PICC_Type::PICC_TYPE_TNP3XXX:
            return true;
        default: // PICC_TYPE_UNKNOWN, PICC_TYPE_NOT_COMPLETE
            return false;
        }
}

bool NfcAdapter::erase()
{
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return write(message);
}

bool
NfcAdapter::format ()
{
    MFRC522::PICC_Type piccType = shield->PICC_GetType (shield->uid.sak);
    switch (piccType)
        {
#if NDEF_SUPPORT_MIFARE_CLASSIC
        case PICC_Type::PICC_TYPE_MIFARE_MINI:
        case PICC_Type::PICC_TYPE_MIFARE_1K:
        case PICC_Type::PICC_TYPE_MIFARE_4K:
            {
                MifareClassic mifareClassic = MifareClassic (shield);
                return mifareClassic.formatNDEF ();
            }
#endif

        case PICC_Type::PICC_TYPE_MIFARE_UL:
            return true;
            break;

        default:
            return false;
        }
}

bool
NfcAdapter::clean ()
{
    MFRC522::PICC_Type piccType = shield->PICC_GetType (shield->uid.sak);
    switch (piccType)
        {
#if NDEF_SUPPORT_MIFARE_CLASSIC
        case PICC_Type::PICC_TYPE_MIFARE_MINI:
        case PICC_Type::PICC_TYPE_MIFARE_1K:
        case PICC_Type::PICC_TYPE_MIFARE_4K:
            {
                MifareClassic mifareClassic = MifareClassic (shield);
                return mifareClassic.formatMifare ();
            }
#endif
        case PICC_Type::PICC_TYPE_MIFARE_UL:
            {
                MifareUltralight ultralight = MifareUltralight (shield);
                return ultralight.clean ();
            }
            break;

        default:
            return false;
        }
}

NfcTag
NfcAdapter::read ()
{
    MFRC522::PICC_Type piccType = shield->PICC_GetType (shield->uid.sak);
    switch (piccType)
        {
#if NDEF_SUPPORT_MIFARE_CLASSIC
        case PICC_Type::PICC_TYPE_MIFARE_MINI:
        case PICC_Type::PICC_TYPE_MIFARE_1K:
        case PICC_Type::PICC_TYPE_MIFARE_4K:
            {
                MifareClassic mifareClassic = MifareClassic (shield);
                return mifareClassic.read ();
            }
#endif
        case PICC_Type::PICC_TYPE_MIFARE_UL:
            {
                MifareUltralight ultralight = MifareUltralight (shield);
                return ultralight.read ();
            }
            break;
        case PICC_Type::PICC_TYPE_ISO_14443_4:
            {
                Type4Tag type4tag = Type4Tag (shield);
                return type4tag.read ();
            }
            break;
        default:
            return NfcTag(shield->uid, PICC_Type::PICC_TYPE_UNKNOWN);
        }
}

bool NfcAdapter::write(NdefMessage& ndefMessage)
{
    PICC_Type type = getTagType();

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

MFRC522Constants::PICC_Type NfcAdapter::getTagType()
{
   return shield->PICC_GetType(shield->uid.sak);
}
