#ifndef NfcTag_h
#define NfcTag_h

#include <ndef_config.h>
#include <inttypes.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <NdefMessage.h>
#include <MFRC522Constants.h>
#include <MFRC522Extended.h>

class NfcTag
{

public:
  using Uid = MFRC522Constants::Uid;
  using PICC_Type = MFRC522Constants::PICC_Type;
  using StatusCode = MFRC522Constants::StatusCode;
  using TagInfo = MFRC522Extended::TagInfo;

  NfcTag(const TagInfo& tag, PICC_Type tagType);
  NfcTag(const TagInfo& tag, PICC_Type tagType, bool isFormatted);
  NfcTag(const TagInfo& tag, PICC_Type tagType, NdefMessage &ndefMessage);
  NfcTag(const TagInfo& tag, PICC_Type tagType, const byte *ndefData, const uint16_t ndefDataLength);
  ~NfcTag(void);
  NfcTag &operator=(const NfcTag &rhs);
  uint8_t getUidLength();
  void getUid(byte *uid, uint8_t *uidLength);
  String getUidString();
  size_t getAtsSize();
  String getAtsString();
  PICC_Type getTagType();
  bool hasNdefMessage();
  NdefMessage getNdefMessage();
  bool isFormatted();
  bool toJson(JsonDocument &result);
  StatusCode getStatus() { return _status; };

#if NDEF_USE_SERIAL
  void print();
#endif
private:
  TagInfo _taginfo;
  PICC_Type _tagType;
  StatusCode _status;
  NdefMessage *_ndefMessage;
  /**
   * if tag is not formatted it is most probably in HALTED state as soon as we realize that
   * because authentication failed => We need to call PICC_WakeupA
   */
  bool _isFormatted;
  // TODO capacity
};

#endif
