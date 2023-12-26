#include "Ndef.h"

#if NDEF_USE_SERIAL
// Borrowed from Adafruit_NFCShield_I2C
void PrintHex(const byte *data, const long numBytes)
{
  int32_t szPos;
  for (szPos = 0; szPos < numBytes; szPos++)
  {
    Serial.print("0x");
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print("0");
    Serial.print(data[szPos] & 0xff, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(" ");
    }
  }
  Serial.println("");
}

// Borrowed from Adafruit_NFCShield_I2C
void PrintHexChar(const byte *data, const long numBytes)
{
  int32_t szPos;
  for (szPos = 0; szPos < numBytes; szPos++)
  {
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print("0");
    Serial.print(data[szPos], HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(" ");
    }
  }
  Serial.print("  ");
  for (szPos = 0; szPos < numBytes; szPos++)
  {
    if (data[szPos] <= 0x1F)
      Serial.print(".");
    else
      Serial.print((char)data[szPos]);
  }
  Serial.println("");
}

// Note if buffer % blockSize != 0, last block will not be written
void DumpHex(const byte *data, const unsigned long numBytes, const unsigned int blockSize)
{
  unsigned int i;
  for (i = 0; i < (numBytes / blockSize); i++)
  {
    PrintHexChar(data, blockSize);
    data += blockSize;
  }
}
#endif

/**
 * @brief Create a hex representation of data.
 *
 * @param [in] target Where to write the hex string. If this is null, we malloc storage.
 * @param [in] source The start of the binary data.
 * @param [in] length The length of the data to convert.
 * @return A pointer to the formatted buffer.
 */
char* buildHexData(uint8_t* target, const uint8_t* source, uint8_t length) {
    // Guard against too much data.
    if (length > 100) length = 100;

    if (target == nullptr) {
        target = (uint8_t*) malloc(length * 2 + 1);
        if (target == nullptr) {
            log_e("buildHexData: malloc failed");
            return nullptr;
        }
    }
    char* startOfData = (char*) target;

    for (int i = 0; i < length; i++) {
        sprintf((char*) target, "%.2x", (char) *source);
        source++;
        target += 2;
    }

    // Handle the special case where there was no data.
    if (length == 0) {
        *startOfData = 0;
    }

    return startOfData;
} // buildHexData
