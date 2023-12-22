#ifndef NfcTag_h
#define NfcTag_h

#include <ndef_config.h>
#include <inttypes.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <NdefMessage.h>
#include <MFRC522Constants.h>

class NfcTag
{

public:
  using Uid = MFRC522Constants::Uid;
  using PICC_Type = MFRC522Constants::PICC_Type;
  using StatusCode = MFRC522Constants::StatusCode;

  NfcTag(const Uid& uid, PICC_Type tagType);
  NfcTag(const Uid& uid, PICC_Type tagType, bool isFormatted);
  NfcTag(const Uid& uid, PICC_Type tagType, NdefMessage &ndefMessage);
  NfcTag(const Uid& uid, PICC_Type tagType, const byte *ndefData, const uint16_t ndefDataLength);
  ~NfcTag(void);
  NfcTag &operator=(const NfcTag &rhs);
  uint8_t getUidLength();
  void getUid(byte *uid, uint8_t *uidLength);
  String getUidString();
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
  Uid _uid;
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
