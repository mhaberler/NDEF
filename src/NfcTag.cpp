#include <NfcTag.h>
#include <MFRC522Debug.h>

NfcTag::NfcTag(const TagInfo& tag, PICC_Type tagType) {
    _taginfo = tag;
    _tagType = tagType;
    _ndefMessage = (NdefMessage *)NULL;
    _isFormatted = false;
}

NfcTag::NfcTag(const TagInfo& tag, PICC_Type tagType, bool isFormatted) {
    _taginfo = tag;
    _tagType = tagType;
    _ndefMessage = (NdefMessage *)NULL;
    _isFormatted = isFormatted;
}

NfcTag::NfcTag(const TagInfo& tag, PICC_Type tagType, NdefMessage &ndefMessage) {
    _taginfo = tag;
    _tagType = tagType;
    _ndefMessage = new NdefMessage(ndefMessage);
    _isFormatted = true; // If it has a message it's formatted
}

NfcTag::NfcTag(const TagInfo& tag, PICC_Type tagType, const byte *ndefData, const uint16_t ndefDataLength) {
    _taginfo = tag;
    _tagType = tagType;
    _ndefMessage = new NdefMessage(ndefData, ndefDataLength);
    _isFormatted = true; // If it has a message it's formatted
}

NfcTag::~NfcTag() {
    delete _ndefMessage;
}

NfcTag &NfcTag::operator=(const NfcTag &rhs) {
    if (this != &rhs) {
        delete _ndefMessage;
        _taginfo = rhs._taginfo;
        _ndefMessage = new NdefMessage(*rhs._ndefMessage);
    }
    return *this;
}

uint8_t NfcTag::getUidLength() {
    return _taginfo.uid.size;
}

void NfcTag::getUid(byte *uid, uint8_t *uidLength) {
    memcpy(uid, _taginfo.uid.uidByte, _taginfo.uid.size < *uidLength ? _taginfo.uid.size : *uidLength);
    *uidLength = _taginfo.uid.size;
}

String NfcTag::getUidString() {
    if (_taginfo.uid.size == 0)
        return "";
    char *hd =  buildHexData (NULL, (uint8_t*)_taginfo.uid.uidByte, _taginfo.uid.size);
    String uidString = String(hd, strlen(hd));
    free(hd);
    return uidString;
}

size_t NfcTag::getAtsSize() {
    return _taginfo.ats.size;
}

String
NfcTag::getAtsString () {
    if (_taginfo.ats.size == 0)
        return "";
    char *hd
        = buildHexData (NULL, (uint8_t *)_taginfo.ats.data, _taginfo.ats.size);
    String atsString = String (hd, strlen (hd));
    free (hd);
    return atsString;
}

MFRC522Constants::PICC_Type NfcTag::getTagType() {
    return _tagType;
}

bool NfcTag::hasNdefMessage() {
    return (_ndefMessage != NULL);
}

NdefMessage NfcTag::getNdefMessage() {
    return *_ndefMessage;
}

bool NfcTag::isFormatted() {
    return _isFormatted;
}

#if NDEF_USE_SERIAL

void NfcTag::print() {
    Serial.print(F("NFC Tag - "));
    Serial.println(_tagType);
    Serial.print(F("UID "));
    Serial.println(getUidString());
    if (_ndefMessage == NULL) {
        Serial.println(F("\nNo NDEF Message"));
    } else {
        _ndefMessage->print();
    }
}
#endif

bool
NfcTag::tagToJson (JsonDocument &doc) {
    if (getUidLength ()) {
        doc["uid"] = getUidString ();
        doc["sak"] = String (_taginfo.uid.sak, HEX);
        doc["atqa"] = String (_taginfo.atqa, HEX);
        doc["type"] = _tagType;
        doc["picc"] = MFRC522Debug::PICC_GetTypeName (_tagType);
        if (getAtsSize ()) {
            doc["ats"] = getAtsString ();
        }
        return true;
    }
    return false;
}

bool
NfcTag::ndefToJson (JsonDocument &doc) {

    if (hasNdefMessage ()) {
        JsonArray ndef = doc.createNestedArray ("ndef");
        getNdefMessage ().toJson (ndef);
        return true;
    }
    return false;
}
