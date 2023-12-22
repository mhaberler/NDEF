#ifndef NdefRecord_h
#define NdefRecord_h

#include <Due.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ndef.h>

class NdefRecord
{
public:
    enum TNF
    {
        TNF_EMPTY,
        TNF_WELL_KNOWN,
        TNF_MIME_MEDIA,
        TNF_ABSOLUTE_URI,
        TNF_EXTERNAL_TYPE,
        TNF_UNKNOWN,
        TNF_UNCHANGED,
        TNF_RESERVED
    };
    enum RTD
    {
        RTD_TEXT = 0x54,
        RTD_URI = 0x55
    };
    enum URIScheme
    {
        EMPTY = 0x00,
        HTTPWWW,
        HTTPSWWW,
        HTTP,
        HTTPS,
        TEL,
        MAILTO,
        FTP_ANONYMOUS,
        FTP_FTP,
        FTPS,
        SFTP,
        NFS,
        FTP,
        DAV,
        NEWS,
        TELNET,
        IMAP,
        RTSP,
        URN,
        POP,
        SIP,
        SIPS,
        TFTP,
        BTSPP,
        BTL2CAP,
        BTGOEP,
        TCPOBEX,
        IRDAOBEX,
        FILE,
        URN_EPC_ID,
        URN_EPC_TAG,
        URN_EPC_PAT,
        URN_EPC_RAW,
        URN_EPC,
        URN_NFC,
    };

    NdefRecord();
    NdefRecord(const NdefRecord &rhs);
    ~NdefRecord();
    NdefRecord &operator=(const NdefRecord &rhs);

    unsigned int getEncodedSize();
    void encode(byte *data, bool firstRecord, bool lastRecord);

    unsigned int getTypeLength();
    unsigned int getPayloadLength();
    unsigned int getIdLength();

    NdefRecord::TNF getTnf();

    const byte *getType();
    const byte *getPayload();
    const byte *getId();

    void setTnf(NdefRecord::TNF tnf);
    void setType(const byte *type, const unsigned int numBytes);
    void setPayload(const byte *payload, const int numBytes);
    void setPayload(const byte *header, const int headerLength, const byte *payload, const int payloadLength);
    void setId(const byte *id, const unsigned int numBytes);

  bool toJson(JsonObject &result, bool detail = true);

    static const char *getURIScheme(NdefRecord::URIScheme scheme);

#ifdef NDEF_USE_SERIAL
    void print();
#endif
private:
    byte _getTnfByte(bool firstRecord, bool lastRecord);
    TNF _tnf; // 3 bit
    unsigned int _typeLength;
    unsigned int _payloadLength;
    unsigned int _idLength;
    byte *_type;
    byte *_payload;
    byte *_id;
};

#endif