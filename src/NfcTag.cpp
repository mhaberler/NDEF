#include <NfcTag.h>
#include <MFRC522Debug.h>

NfcTag::NfcTag(const TagInfo& tag, PICC_Type tagType)
{
    _taginfo = tag;
    _tagType = tagType;
    _ndefMessage = (NdefMessage *)NULL;
    _isFormatted = false;
}

NfcTag::NfcTag(const TagInfo& tag, PICC_Type tagType, bool isFormatted)
{
    _taginfo = tag;
    _tagType = tagType;
    _ndefMessage = (NdefMessage *)NULL;
    _isFormatted = isFormatted;
}

NfcTag::NfcTag(const TagInfo& tag, PICC_Type tagType, NdefMessage &ndefMessage)
{
    _taginfo = tag;
    _tagType = tagType;
    _ndefMessage = new NdefMessage(ndefMessage);
    _isFormatted = true; // If it has a message it's formatted
}

NfcTag::NfcTag(const TagInfo& tag, PICC_Type tagType, const byte *ndefData, const uint16_t ndefDataLength)
{
    _taginfo = tag;
    _tagType = tagType;
    _ndefMessage = new NdefMessage(ndefData, ndefDataLength);
    _isFormatted = true; // If it has a message it's formatted
}

NfcTag::~NfcTag()
{
    delete _ndefMessage;
}

NfcTag &NfcTag::operator=(const NfcTag &rhs)
{
    if (this != &rhs)
    {
        delete _ndefMessage;
        _taginfo = rhs._taginfo;
        _ndefMessage = new NdefMessage(*rhs._ndefMessage);
    }
    return *this;
}

uint8_t NfcTag::getUidLength()
{
    return _taginfo.uid.size;
}

void NfcTag::getUid(byte *uid, uint8_t *uidLength)
{
    memcpy(uid, _taginfo.uid.uidByte, _taginfo.uid.size < *uidLength ? _taginfo.uid.size : *uidLength);
    *uidLength = _taginfo.uid.size;
}

String NfcTag::getUidString()
{
    String uidString = "";
    for (unsigned int i = 0; i < _taginfo.uid.size; i++)
    {
        if (i > 0)
        {
            uidString += " ";
        }

        if (_taginfo.uid.uidByte[i] < 0xF)
        {
            uidString += "0";
        }

        uidString += String((unsigned int)_taginfo.uid.uidByte[i], (unsigned char)HEX);
    }
    uidString.toUpperCase();
    return uidString;
}

size_t NfcTag::getAtsSize()
{
    return _taginfo.ats.size;
}


String NfcTag::getAtsString()
{
    String atsString = "";
    for (unsigned int i = 0; i < _taginfo.ats.size; i++)
    {
        if (i > 0)
        {
            atsString += " ";
        }

        if (_taginfo.ats.data[i] < 0xF)
        {
            atsString += "0";
        }

        atsString += String((unsigned int)_taginfo.ats.data[i], (unsigned char)HEX);
    }
    atsString.toUpperCase();
    return atsString;
}

MFRC522Constants::PICC_Type NfcTag::getTagType()
{
    return _tagType;
}

bool NfcTag::hasNdefMessage()
{
    return (_ndefMessage != NULL);
}

NdefMessage NfcTag::getNdefMessage()
{
    return *_ndefMessage;
}

bool NfcTag::isFormatted()
{
    return _isFormatted;
}

#if NDEF_USE_SERIAL

void NfcTag::print()
{
    Serial.print(F("NFC Tag - "));
    Serial.println(_tagType);
    Serial.print(F("UID "));
    Serial.println(getUidString());
    if (_ndefMessage == NULL)
    {
        Serial.println(F("\nNo NDEF Message"));
    }
    else
    {
        _ndefMessage->print();
    }
}
#endif

bool
NfcTag::toJson (JsonDocument &doc)
{
    if (getUidLength ())
        {
            doc["uid"] = getUidString ();
            doc["sak"] = String (_taginfo.uid.sak, HEX);
            doc["atqa"] = String (_taginfo.atqa, HEX);
            doc["type"] = _tagType;
            doc["picc"] = MFRC522Debug::PICC_GetTypeName (_tagType);
            if (getAtsSize ())
                {
                    doc["ats"] = getAtsString ();
                }
        }
    if (hasNdefMessage ())
        {
            JsonArray ndef = doc.createNestedArray ("ndef");
            getNdefMessage ().toJson (ndef);
        }
    return true;
}
