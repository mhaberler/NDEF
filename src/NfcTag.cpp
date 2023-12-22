#include <NfcTag.h>

NfcTag::NfcTag(const Uid &uid, TagType tagType)
{
    _uid = uid;
        _tagType = tagType;
    _ndefMessage = (NdefMessage *)NULL;
    _isFormatted = false;
}

NfcTag::NfcTag(const Uid &uid, TagType tagType, bool isFormatted)
{
    _uid = uid;
        _tagType = tagType;
    _ndefMessage = (NdefMessage *)NULL;
    _isFormatted = isFormatted;
}

NfcTag::NfcTag(const Uid &uid, TagType tagType, NdefMessage &ndefMessage)
{
    _uid = uid;
        _tagType = tagType;
    _ndefMessage = new NdefMessage(ndefMessage);
    _isFormatted = true; // If it has a message it's formatted
}

NfcTag::NfcTag(const Uid &uid, TagType tagType, const byte *ndefData, const uint16_t ndefDataLength)
{
    _uid = uid;
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
        _uid = rhs._uid;
                _ndefMessage = new NdefMessage(*rhs._ndefMessage);
    }
    return *this;
}

uint8_t NfcTag::getUidLength()
{
    return _uid.size;
}

void NfcTag::getUid(byte *uid, uint8_t *uidLength)
{
    memcpy(uid, _uid.uidByte, _uid.size < *uidLength ? _uid.size : *uidLength);
    *uidLength = _uid.size;
}

String NfcTag::getUidString()
{
    String uidString = "";
    for (unsigned int i = 0; i < _uid.size; i++)
    {
        if (i > 0)
        {
            uidString += " ";
        }

        if (_uid.uidByte[i] < 0xF)
        {
            uidString += "0";
        }

        uidString += String((unsigned int)_uid.uidByte[i], (unsigned char)HEX);
    }
    uidString.toUpperCase();
    return uidString;
}

NfcTag::TagType NfcTag::getTagType()
{
    return _tagType;
}

boolean NfcTag::hasNdefMessage()
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
#ifdef NDEF_USE_SERIAL

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

bool NfcTag::toJson(JsonDocument &doc)
{
  if (getUidLength())
  {
    doc["uid"] = getUidString();
    doc["sak"] = _uid.sak;
    doc["type"] = _tagType;
  }
  if (hasNdefMessage())
  {
    JsonArray ndef = doc.createNestedArray("ndef");
    getNdefMessage().toJson(ndef);
  }
  return true;
}
