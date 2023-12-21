#include <MFRC522v2.h>
#include <Type4Tag.h>

Type4Tag::Type4Tag(MFRC522Extended *nfcShield) { nfc = nfcShield; }

Type4Tag::~Type4Tag() {}

NfcTag
Type4Tag::read()
{
  StatusCode sc;
  uint16_t fileId, maxSize;
  uint8_t readAccess, writeAccess;
  uint16_t fileSize;
  uint8_t *buffer;

  sc = SelectNdefTag_Application();
  if (sc != StatusCode::STATUS_OK)
    goto fail;
  sc = SelectCapabilityContainer();
  if (sc != StatusCode::STATUS_OK)
    goto fail;
  sc = readCCFile(fileId, maxSize, readAccess, writeAccess);
  if (sc != StatusCode::STATUS_OK)
    goto fail;
  sc = selectFile(fileId);
  if (sc != StatusCode::STATUS_OK)
    goto fail;
  sc = readFileLength(fileSize);
  if (sc != StatusCode::STATUS_OK)
    goto fail;
  buffer = (uint8_t *)calloc(maxSize, 1);
  sc = readFile(buffer, fileSize);
  if (sc == StatusCode::STATUS_OK)
  {
    NdefMessage msg(buffer, fileSize);
    free(buffer);
    return NfcTag(nfc->uid.uidByte, nfc->uid.size, NfcTag::TYPE_4,
                  msg);
  }
fail:
  return NfcTag(nfc->uid.uidByte, nfc->uid.size, NfcTag::TYPE_4);
}

StatusCode
Type4Tag::SelectNdefTag_Application()
{
  byte sendData[] = {0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76,
                     0x00, 0x00, 0x85, 0x01, 0x01, 0x00};
  byte backData[2];
  byte backLen = 2;

  StatusCode statusCode = nfc->TCL_Transceive(
      &nfc->tag, sendData, sizeof(sendData), backData, &backLen);
  if (statusCode != StatusCode::STATUS_OK)
    return statusCode;
  if (backLen < 2)
    return StatusCode::STATUS_INTERNAL_ERROR;

  if (backData[backLen - 2] != 0x90 || backData[backLen - 1] != 0x00)
    return StatusCode::STATUS_ERROR;
  return StatusCode::STATUS_OK;
}

StatusCode
Type4Tag::SelectCapabilityContainer()
{
  byte sendData[] = {0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x03};
  byte backData[3] = {0};
  byte backLen = 3; // sizeof(backData);

  StatusCode statusCode = nfc->TCL_Transceive(
      &nfc->tag, sendData, sizeof(sendData), backData, &backLen);
  if (statusCode != StatusCode::STATUS_OK)
    return statusCode;
  if (backLen < 2)
    return StatusCode::STATUS_INTERNAL_ERROR;
  if (backData[backLen - 2] != 0x90 || backData[backLen - 1] != 0x00)
    return StatusCode::STATUS_ERROR;

  return StatusCode::STATUS_OK;
}

StatusCode
Type4Tag::readCCFile(uint16_t &fileId, uint16_t &maxSize, uint8_t &readAccess,
                     uint8_t &writeAccess)
{
  uint8_t readCmd[] = {0x00, 0xB0, 0x00, 0x00, 0x0F};
  uint8_t response[17];
  uint8_t responseLen = sizeof(response);

  StatusCode statusCode = nfc->TCL_Transceive(
      &nfc->tag, readCmd, sizeof(readCmd), response, &responseLen);
  if (statusCode != StatusCode::STATUS_OK)
    return statusCode;
  if (responseLen < 2)
    return StatusCode::STATUS_INTERNAL_ERROR;
  if (response[responseLen - 2] != 0x90 || response[responseLen - 1] != 0x00)
    return StatusCode::STATUS_ERROR;

  fileId = (response[9] << 8) | response[10];
  maxSize = (response[11] << 8) | response[12];
  readAccess = response[13];
  writeAccess = response[14];
  return statusCode;
}

StatusCode
Type4Tag::selectFile(const uint16_t fileId)
{
  uint8_t selectCmd[] = {0x00, 0xA4, 0x00, 0x0C, 0x02, (fileId >> 8) & 0xff, fileId & 0xff};
  uint8_t response[3];
  uint8_t responseLen = sizeof(response);

  StatusCode statusCode = nfc->TCL_Transceive(
      &nfc->tag, selectCmd, sizeof(selectCmd), response, &responseLen);
  if (statusCode != StatusCode::STATUS_OK)
    return statusCode;
  if (responseLen < 2)
    return StatusCode::STATUS_INTERNAL_ERROR;

  if (response[responseLen - 2] != 0x90 || response[responseLen - 1] != 0x00)
    return StatusCode::STATUS_ERROR;
  return StatusCode::STATUS_OK;
}

StatusCode
Type4Tag::readFileLength(uint16_t &fileSize)
{
  uint8_t cmd[] = {0x00, 0xB0, 0x00, 0x00, 0x02};
  uint8_t response[5];
  uint8_t responseLen = sizeof(response);

  StatusCode statusCode = nfc->TCL_Transceive(&nfc->tag, cmd, sizeof(cmd),
                                              response, &responseLen);
  if (statusCode != StatusCode::STATUS_OK)
    return statusCode;
  if (responseLen < 2)
    return StatusCode::STATUS_INTERNAL_ERROR;
  if (response[responseLen - 2] != 0x90 || response[responseLen - 1] != 0x00)
    return StatusCode::STATUS_ERROR;

  fileSize = (response[0] << 8) | response[1];
  return statusCode;
}

StatusCode
Type4Tag::readFile(uint8_t *response, const size_t fileLength)
{
  StatusCode statusCode = StatusCode::STATUS_OK;
  uint8_t buffer[fileLength];
  uint8_t responseLen;
  bool success = true;
  size_t mtu = MFRC522::FIFO_SIZE - 6; // leave room for CRC etc
  size_t pos = 0;
  size_t iter = 0;
  while ((pos < fileLength) && success)
  {
    uint8_t read_len = ((fileLength - pos) > mtu) ? mtu : (fileLength - pos);
    uint8_t readstart = pos;
    uint8_t readCmd[5] = {0x00, 0xB0, (readstart >> 8) & 0xff,
                          readstart & 0xff, read_len};
    responseLen = fileLength;
    statusCode = nfc->TCL_Transceive(&nfc->tag, readCmd, sizeof(readCmd),
                                     buffer, &responseLen);
    success = (statusCode == StatusCode::STATUS_OK);
    if (success)
    {
      Serial.printf("responseLen len = %d\n", responseLen);
      HexDump(Serial, (void *)buffer, responseLen);
      if (iter == 0)
      {
        // skip length field at beginning, drop sw1 sw2 at end
        memcpy(response, buffer + 2, responseLen - 4);
        response += (responseLen - 4);
      }
      else
      {
        // drop sw1 sw2 at end
        memcpy(response, buffer, responseLen - 2); // skip length field
        response += (responseLen - 2);
      }
    }
    pos += read_len;
    iter++;
  }
  return success ? StatusCode::STATUS_OK : statusCode;
}
