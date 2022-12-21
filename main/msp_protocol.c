/// @file msp_protocol.c

/*
 * idf header files
 */
#include "esp_spp_api.h"
#include "esp_system.h"
#include "esp_log.h"

/*
 * basic header files
 */
#include "define.h"
#include "handle.h"

/*
 * module header files
 */
#include "module.h"
#include "mode.h"
#include "msp_protocol.h"

/*
 * service header files
 */
#include "ttl.h"
#include "udp_server.h"

typedef enum {
    MSP_IDLE                               = 0,
    MSP_HEADER_START                       = 1,
    MSP_HEADER_M                           = 2,
    MSP_HEADER_X                           = 3,

    MSP_HEADER_V1                          = 4,
    MSP_PAYLOAD_V1                         = 5,
    MSP_CHECKSUM_V1                        = 6,

    MSP_HEADER_V2_OVER_V1                  = 7,
    MSP_PAYLOAD_V2_OVER_V1                 = 8,
    MSP_CHECKSUM_V2_OVER_V1                = 9,

    MSP_HEADER_V2_NATIVE                   = 10,
    MSP_PAYLOAD_V2_NATIVE                  = 11,
    MSP_CHECKSUM_V2_NATIVE                 = 12,

    MSP_COMMAND_RECEIVED                   = 13
} mspState_e;

#define MSP_V2_FRAME_ID                      255
#define MSP_PORT_INBUF_SIZE                  192
#define JUMBO_FRAME_SIZE_LIMIT               255

#define MSP_VERSION_MAGIC_INITIALIZER { 'M', 'M', 'X' }

// return positive for ACK, negative on error, zero for no reply
typedef enum {
    MSP_RESULT_ACK = 1,
    MSP_RESULT_ERROR = -1,
    MSP_RESULT_NO_REPLY = 0
} mspResult_e;

typedef enum {
    MSP_FLAG_DONT_REPLY           = (1 << 0),
} mspFlags_e;

typedef enum {
    MSP_V1          = 0,
    MSP_V2_OVER_V1  = 1,
    MSP_V2_NATIVE   = 2,
    MSP_VERSION_COUNT
} mspVersion_e;

typedef struct mspPort_s {
    mspState_e c_state;
    uint8_t headBuf[MSP_PORT_INBUF_SIZE];
    uint8_t inBuf[MSP_PORT_INBUF_SIZE];
    uint_fast16_t offset;
    uint_fast16_t dataSize;
    mspVersion_e mspVersion;
    uint8_t cmdFlags;
    uint16_t cmdMSP;
    uint8_t checksum1;
    uint8_t checksum2;
} mspPort_t;

typedef struct __attribute__((packed)) {
    uint8_t size;
    uint8_t cmd;
} mspHeaderV1_t;

typedef struct __attribute__((packed)) {
    uint16_t size;
} mspHeaderJUMBO_t;

typedef struct __attribute__((packed)) {
    uint8_t  flags;
    uint16_t cmd;
    uint16_t size;
} mspHeaderV2_t;

typedef struct sbuf_s {
    uint8_t *ptr;          // data pointer must be first (sbuff_t* is equivalent to uint8_t **)
    uint8_t *end;
} sbuf_t;

typedef struct mspPacket_s {
    sbuf_t buf;
    int16_t cmd;
    uint8_t flags;
    int16_t result;
} mspPacket_t;


static mspPort_t g_esp_msp_port;
static messageVersion_e g_esp_msg_center = MESSAGE_UNKNOW;

uint16_t g_esp_rc_channel[MAX_SUPPORTED_RC_CHANNEL_COUNT] ={
    1500,    //1
    1500,    //2
    885,     //3
    1500,    //4
    1200,    //5
    1200,    //6
    1200,    //7
    1200,    //8
    1200,    //9
    1200,    //10
    1200,    //11
    1200,    //12
    1200,    //13
    1200,    //14
    1200,    //15
    1200,    //16
    1200,    //17
    1200,    //18
};

uint8_t crc8_dvb_s2(uint8_t crc, unsigned char a)
{
    crc ^= a;
    for (int ii = 0; ii < 8; ++ii) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0xD5;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

uint8_t crc8_dvb_s2_update(uint8_t crc, const void *data, uint32_t length)
{
    const uint8_t *p = (const uint8_t *)data;
    const uint8_t *pend = p + length;

    for (; p != pend; p++) {
        crc = crc8_dvb_s2(crc, *p);
    }
    return crc;
}

static uint8_t mspSerialChecksumBuf(uint8_t checksum, const uint8_t *data, int len)
{
    while (len-- > 0) {
        checksum ^= *data++;
    }
    return checksum;
}

sbuf_t *sbufInit(sbuf_t *sbuf, uint8_t *ptr, uint8_t *end)
{
    sbuf->ptr = ptr;
    sbuf->end = end;
    return sbuf;
}

int sbufBytesRemaining(const sbuf_t *buf)
{
    return buf->end - buf->ptr;
}

uint8_t* sbufPtr(sbuf_t *buf)
{
    return buf->ptr;
}


void sbufWriteU8(sbuf_t *dst, uint8_t val)
{
    *dst->ptr++ = val;
}

void sbufWriteU16(sbuf_t *dst, uint16_t val)
{
    sbufWriteU8(dst, val >> 0);
    sbufWriteU8(dst, val >> 8);
}

void sbufWriteU32(sbuf_t *dst, uint32_t val)
{
    sbufWriteU8(dst, val >> 0);
    sbufWriteU8(dst, val >> 8);
    sbufWriteU8(dst, val >> 16);
    sbufWriteU8(dst, val >> 24);
}

uint8_t sbufReadU8(sbuf_t *src)
{
    return *src->ptr++;
}

uint16_t sbufReadU16(sbuf_t *src)
{
    uint16_t ret;
    ret = sbufReadU8(src);
    ret |= sbufReadU8(src) << 8;
    return ret;
}

uint32_t sbufReadU32(sbuf_t *src)
{
    uint32_t ret;
    ret = sbufReadU8(src);
    ret |= sbufReadU8(src) <<  8;
    ret |= sbufReadU8(src) << 16;
    ret |= sbufReadU8(src) << 24;
    return ret;
}


esp_err_t mspSetMessage(messageVersion_e type)
{
    if(MESSAGE_UNKNOW == type || g_esp_msg_center == type){
        g_esp_msg_center = type;
        return ESP_OK;
    }
    
    if (MESSAGE_UNKNOW == g_esp_msg_center){
        g_esp_msg_center = type;
    }else{
        do{
#if (0)
            ESP_LOGI(MODULE_MSP_PROTO, "mspSetMessage wait %d", g_esp_msg_center);
            vTaskDelay(TIME_500_MS / portTICK_PERIOD_MS);
#else
            vTaskDelay(TIME_5_MS / portTICK_PERIOD_MS);
#endif /* DEBUG_MSP_PROTO */
        }while(MESSAGE_UNKNOW != g_esp_msg_center);
        g_esp_msg_center = type;
    }
    return ESP_OK;
}

messageVersion_e mspGetMessage(void)
{
    return g_esp_msg_center;
}


esp_err_t mspSetChannel(uint8_t index, uint16_t value)
{
    if (index >= MAX_SUPPORTED_RC_CHANNEL_COUNT){
        return ESP_FAIL;
    }

    g_esp_rc_channel[index] = value;
    return ESP_OK;
}
esp_err_t mspSetChannels(uint8_t count, uint16_t *value)
{
    if (count >= MAX_SUPPORTED_RC_CHANNEL_COUNT 
        && NULL != value){
        return ESP_FAIL;
    }

    for(int i = 0; i < count; i++){
        g_esp_rc_channel[i] = *(value + i);
    }
    return ESP_OK;
}

esp_err_t mspSerialEncode(mspPacket_t *packet, mspVersion_e mspVersion)
{
    static const uint8_t mspMagic[MSP_VERSION_COUNT] = MSP_VERSION_MAGIC_INITIALIZER;
    const int dataLen = sbufBytesRemaining(&packet->buf);
    uint8_t hdrBuf[16] = { '$', mspMagic[mspVersion], packet->result == MSP_RESULT_ERROR ? '!' : '<'};
    uint8_t crcBuf[2];
    int hdrLen = 3;
    int crcLen = 0;

    #define V1_CHECKSUM_STARTPOS 3
    if (mspVersion == MSP_V1) {
        mspHeaderV1_t * hdrV1 = (mspHeaderV1_t *)&hdrBuf[hdrLen];
        hdrLen += sizeof(mspHeaderV1_t);
        hdrV1->cmd = packet->cmd;

        // Add JUMBO-frame header if necessary
        if (dataLen >= JUMBO_FRAME_SIZE_LIMIT) {
            mspHeaderJUMBO_t * hdrJUMBO = (mspHeaderJUMBO_t *)&hdrBuf[hdrLen];
            hdrLen += sizeof(mspHeaderJUMBO_t);

            hdrV1->size = JUMBO_FRAME_SIZE_LIMIT;
            hdrJUMBO->size = dataLen;
        }
        else {
            hdrV1->size = dataLen;
        }

        // Pre-calculate CRC
        crcBuf[crcLen] = mspSerialChecksumBuf(0, hdrBuf + V1_CHECKSUM_STARTPOS, hdrLen - V1_CHECKSUM_STARTPOS);
        crcBuf[crcLen] = mspSerialChecksumBuf(crcBuf[crcLen], sbufPtr(&packet->buf), dataLen);
        crcLen++;
    }
    else if (mspVersion == MSP_V2_OVER_V1) {
        mspHeaderV1_t * hdrV1 = (mspHeaderV1_t *)&hdrBuf[hdrLen];

        hdrLen += sizeof(mspHeaderV1_t);

        mspHeaderV2_t * hdrV2 = (mspHeaderV2_t *)&hdrBuf[hdrLen];
        hdrLen += sizeof(mspHeaderV2_t);

        const int v1PayloadSize = sizeof(mspHeaderV2_t) + dataLen + 1;  // MSPv2 header + data payload + MSPv2 checksum
        hdrV1->cmd = MSP_V2_FRAME_ID;

        // Add JUMBO-frame header if necessary
        if (v1PayloadSize >= JUMBO_FRAME_SIZE_LIMIT) {
            mspHeaderJUMBO_t * hdrJUMBO = (mspHeaderJUMBO_t *)&hdrBuf[hdrLen];
            hdrLen += sizeof(mspHeaderJUMBO_t);

            hdrV1->size = JUMBO_FRAME_SIZE_LIMIT;
            hdrJUMBO->size = v1PayloadSize;
        }
        else {
            hdrV1->size = v1PayloadSize;
        }

        // Fill V2 header
        hdrV2->flags = packet->flags;
        hdrV2->cmd = packet->cmd;
        hdrV2->size = dataLen;

        // V2 CRC: only V2 header + data payload
        crcBuf[crcLen] = crc8_dvb_s2_update(0, (uint8_t *)hdrV2, sizeof(mspHeaderV2_t));
        crcBuf[crcLen] = crc8_dvb_s2_update(crcBuf[crcLen], sbufPtr(&packet->buf), dataLen);
        crcLen++;

        // V1 CRC: All headers + data payload + V2 CRC byte
        crcBuf[crcLen] = mspSerialChecksumBuf(0, hdrBuf + V1_CHECKSUM_STARTPOS, hdrLen - V1_CHECKSUM_STARTPOS);
        crcBuf[crcLen] = mspSerialChecksumBuf(crcBuf[crcLen], sbufPtr(&packet->buf), dataLen);
        crcBuf[crcLen] = mspSerialChecksumBuf(crcBuf[crcLen], crcBuf, crcLen);
        crcLen++;
    }
    else if (mspVersion == MSP_V2_NATIVE) {
        mspHeaderV2_t * hdrV2 = (mspHeaderV2_t *)&hdrBuf[hdrLen];
        hdrLen += sizeof(mspHeaderV2_t);

        hdrV2->flags = packet->flags;
        hdrV2->cmd = packet->cmd;
        hdrV2->size = dataLen;

        crcBuf[crcLen] = crc8_dvb_s2_update(0, (uint8_t *)hdrV2, sizeof(mspHeaderV2_t));
        crcBuf[crcLen] = crc8_dvb_s2_update(crcBuf[crcLen], sbufPtr(&packet->buf), dataLen);
        crcLen++;
    }
    else {
        // Shouldn't get here
        return ESP_FAIL;
    }

    // Send the frame
    ttl_send(hdrBuf, hdrLen);
    ttl_send(sbufPtr(&packet->buf), dataLen);
    ttl_send(crcBuf, crcLen);

    return ESP_OK;
}

esp_err_t mspUpdateChannels(void)
{
    uint8_t msg_buffer[STR_BUFFER_LEN];
    sbuf_t msg;

    sbufInit(&msg, &msg_buffer[0], &msg_buffer[STR_BUFFER_LEN]);

    for(int i = 0; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++){
        sbufWriteU16(&msg, g_esp_rc_channel[i]);
    }

    mspPacket_t command = {
        .buf = { .ptr = &msg_buffer[0], .end = &msg_buffer[0] + sizeof(uint16_t)*MAX_SUPPORTED_RC_CHANNEL_COUNT, },
        .cmd = MSP_SET_RAW_RC,
        .flags = MSP_FLAG_DONT_REPLY,
        .result = 0,
    };

    return mspSerialEncode(&command, MSP_V2_NATIVE);
}




static bool mspSerialProcessReceivedData(mspPort_t *mspPort, uint8_t c)
{
    switch (mspPort->c_state) {
        default:
        case MSP_IDLE:      // Waiting for 'X' character
            if (c == '$') {
                mspPort->mspVersion = MSP_V1;
                mspPort->c_state = MSP_HEADER_START;
            }
            else {
                return false;
            }
            break;

        case MSP_HEADER_START:  // Waiting for 'M' (MSPv1 / MSPv2_over_v1) or 'X' (MSPv2 native)
            switch (c) {
                case 'M':
                    mspPort->c_state = MSP_HEADER_M;
                    break;
                case 'X':
                    mspPort->c_state = MSP_HEADER_X;
                    break;
                default:
                    mspPort->c_state = MSP_IDLE;
                    break;
            }
            break;

        case MSP_HEADER_M:      // Waiting for '<'
            if (c == '>') {
                mspPort->offset = 0;
                mspPort->checksum1 = 0;
                mspPort->checksum2 = 0;
                mspPort->c_state = MSP_HEADER_V1;
            }
            else {
                mspPort->c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_X:
            if (c == '>') {
                mspPort->offset = 0;
                mspPort->checksum2 = 0;
                mspPort->mspVersion = MSP_V2_NATIVE;
                mspPort->c_state = MSP_HEADER_V2_NATIVE;
            }
            else {
                mspPort->c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_V1:     // Now receive v1 header (size/cmd), this is already checksummable
            mspPort->inBuf[mspPort->offset++] = c;
            mspPort->checksum1 ^= c;
            if (mspPort->offset == sizeof(mspHeaderV1_t)) {
                mspHeaderV1_t * hdr = (mspHeaderV1_t *)&mspPort->inBuf[0];
                // Check incoming buffer size limit
                if (hdr->size > MSP_PORT_INBUF_SIZE) {
                    mspPort->c_state = MSP_IDLE;
                }
                else if (hdr->cmd == MSP_V2_FRAME_ID) {
                    // MSPv1 payload must be big enough to hold V2 header + extra checksum
                    if (hdr->size >= sizeof(mspHeaderV2_t) + 1) {
                        mspPort->mspVersion = MSP_V2_OVER_V1;
                        mspPort->c_state = MSP_HEADER_V2_OVER_V1;
                    }
                    else {
                        mspPort->c_state = MSP_IDLE;
                    }
                }
                else {
                    mspPort->dataSize = hdr->size;
                    mspPort->cmdMSP = hdr->cmd;
                    mspPort->cmdFlags = 0;
                    mspPort->offset = 0;                // re-use buffer
                    mspPort->c_state = mspPort->dataSize > 0 ? MSP_PAYLOAD_V1 : MSP_CHECKSUM_V1;    // If no payload - jump to checksum byte
                }
            }
            break;

        case MSP_PAYLOAD_V1:
            mspPort->inBuf[mspPort->offset++] = c;
            mspPort->checksum1 ^= c;
            if (mspPort->offset == mspPort->dataSize) {
                mspPort->c_state = MSP_CHECKSUM_V1;
            }
            break;

        case MSP_CHECKSUM_V1:
            if (mspPort->checksum1 == c) {
                mspPort->c_state = MSP_COMMAND_RECEIVED;
            } else {
                mspPort->c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_V2_OVER_V1:     // V2 header is part of V1 payload - we need to calculate both checksums now
            mspPort->inBuf[mspPort->offset++] = c;
            mspPort->checksum1 ^= c;
            mspPort->checksum2 = crc8_dvb_s2(mspPort->checksum2, c);
            if (mspPort->offset == (sizeof(mspHeaderV2_t) + sizeof(mspHeaderV1_t))) {
                mspHeaderV2_t * hdrv2 = (mspHeaderV2_t *)&mspPort->inBuf[sizeof(mspHeaderV1_t)];
                mspPort->dataSize = hdrv2->size;

                // Check for potential buffer overflow
                if (hdrv2->size > MSP_PORT_INBUF_SIZE) {
                    mspPort->c_state = MSP_IDLE;
                }
                else {
                    mspPort->cmdMSP = hdrv2->cmd;
                    mspPort->cmdFlags = hdrv2->flags;
                    mspPort->offset = 0;                // re-use buffer
                    mspPort->c_state = mspPort->dataSize > 0 ? MSP_PAYLOAD_V2_OVER_V1 : MSP_CHECKSUM_V2_OVER_V1;
                }
            }
            break;

        case MSP_PAYLOAD_V2_OVER_V1:
            mspPort->checksum2 = crc8_dvb_s2(mspPort->checksum2, c);
            mspPort->checksum1 ^= c;
            mspPort->inBuf[mspPort->offset++] = c;

            if (mspPort->offset == mspPort->dataSize) {
                mspPort->c_state = MSP_CHECKSUM_V2_OVER_V1;
            }
            break;

        case MSP_CHECKSUM_V2_OVER_V1:
            mspPort->checksum1 ^= c;
            if (mspPort->checksum2 == c) {
                mspPort->c_state = MSP_CHECKSUM_V1; // Checksum 2 correct - verify v1 checksum
            } else {
                mspPort->c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_V2_NATIVE:
            mspPort->headBuf[mspPort->offset] = c;
            mspPort->inBuf[mspPort->offset++] = c;
            mspPort->checksum2 = crc8_dvb_s2(mspPort->checksum2, c);
            if (mspPort->offset == sizeof(mspHeaderV2_t)) {
                mspHeaderV2_t * hdrv2 = (mspHeaderV2_t *)&mspPort->inBuf[0];

                // Check for potential buffer overflow
                if (hdrv2->size > MSP_PORT_INBUF_SIZE) {
                    mspPort->c_state = MSP_IDLE;
                }
                else {
                    mspPort->dataSize = hdrv2->size;
                    mspPort->cmdMSP = hdrv2->cmd;
                    mspPort->cmdFlags = hdrv2->flags;
                    mspPort->offset = 0;                // re-use buffer
                    mspPort->c_state = mspPort->dataSize > 0 ? MSP_PAYLOAD_V2_NATIVE : MSP_CHECKSUM_V2_NATIVE;
                }
            }
            break;

        case MSP_PAYLOAD_V2_NATIVE:
            mspPort->checksum2 = crc8_dvb_s2(mspPort->checksum2, c);
            mspPort->inBuf[mspPort->offset++] = c;

            if (mspPort->offset == mspPort->dataSize) {
                mspPort->c_state = MSP_CHECKSUM_V2_NATIVE;
            }
            break;

        case MSP_CHECKSUM_V2_NATIVE:
            if (mspPort->checksum2 == c) {
                mspPort->c_state = MSP_COMMAND_RECEIVED;
            } else {
                mspPort->c_state = MSP_IDLE;
            }
            break;
    }

    //ESP_LOGI(MODULE_UART, "ttl mspPort->c_state %d", mspPort->c_state);

    return true;
}

esp_err_t ttl_handle_bt_msp_protocol(uint8_t * buf, int len)
{
    if (NULL == buf){
        return ESP_FAIL;
    }

#if (DEBUG_MSP_PROTO)
    ESP_LOGI(MODULE_MSP_PROTO, "ttl_handle_bt_msp-exit %d bytes", len);
    esp_log_buffer_hex(MODULE_MSP_PROTO, buf, len);
#endif /* DEBUG_MSP_PROTO */

    esp_spp_write(g_esp_ssp_handle, len, buf);
    return ESP_OK;
}

esp_err_t ttl_handle_wifi_msp_protocol(uint8_t * buf, int len)
{
    if (NULL == buf){
        return ESP_FAIL;
    }

    if (*buf != '$' || len < 9){
        return ESP_FAIL;
    }

    if (len > STR_BUFFER_LEN){
        ESP_LOGW(MODULE_MSP_PROTO, "ttl_handle_wifi_msp-enter %d bytes", len);
        return ESP_FAIL;
    }

#if (0)
    ESP_LOGI(MODULE_MSP_PROTO, "ttl_handle_wifi_msp-enter %d bytes", len);
    esp_log_buffer_hex(MODULE_MSP_PROTO, buf, len);
#endif /* DEBUG_MSP_PROTO */

    g_esp_msp_port.c_state = MSP_IDLE;

    for(int i = 0; i < len; i++){
        mspSerialProcessReceivedData(&g_esp_msp_port, *(buf +i));
        if(MSP_COMMAND_RECEIVED == g_esp_msp_port.c_state){
#if (0)
            ESP_LOGI(MODULE_MSP_PROTO, "ttl_handle_msp-recv version %d wifi %d sta %d", 
                    g_esp_msp_port.mspVersion, snap_sw_state_active(SW_MODE_WIFI_AP), snap_sw_state_active(SW_MODE_WIFI_STA));
#endif /* DEBUG_MSP_PROTO */
            if(snap_sw_state_active(SW_MODE_WIFI_AP) || snap_sw_state_active(SW_MODE_WIFI_STA)){
                uint8_t rx_buffer[STR_BUFFER_LEN];
                switch(g_esp_msp_port.mspVersion){
                    case MSP_V2_NATIVE:
                        rx_buffer[0] = '$';
                        rx_buffer[1] = 'X';
                        rx_buffer[2] = '>';
                        int j = 0;
                        for(; j < sizeof(mspHeaderV2_t); j++){
                            rx_buffer[3 + j] = g_esp_msp_port.headBuf[j];
                        }
                        j = 0;
                        if (g_esp_msp_port.dataSize > 0){
                            for(; j < g_esp_msp_port.offset; j++){
                                rx_buffer[3 +sizeof(mspHeaderV2_t) + j] = g_esp_msp_port.inBuf[j];
                            }
                        }
                        rx_buffer[3 +sizeof(mspHeaderV2_t) + j] = g_esp_msp_port.checksum2;
#if (DEBUG_MSP_PROTO)
                        sbuf_t sbuf_hdrv2;
                        mspHeaderV2_t hdrv2;
                        
                        sbufInit(&sbuf_hdrv2, &g_esp_msp_port.headBuf[0], &g_esp_msp_port.headBuf[MSP_PORT_INBUF_SIZE]);
                        
                        hdrv2.flags = sbufReadU8(&sbuf_hdrv2);
                        hdrv2.cmd = sbufReadU16(&sbuf_hdrv2);
                        hdrv2.size = sbufReadU16(&sbuf_hdrv2);
                        
                        ESP_LOGI(MODULE_MSP_PROTO, "ttl_handle_wifi_msp-exit %d bytes cmd 0x%04x-%d flag %d size %d", 
                                            4+ sizeof(mspHeaderV2_t) + j, hdrv2.cmd, hdrv2.cmd, hdrv2.flags, hdrv2.size);
                        esp_log_buffer_hex(MODULE_MSP_PROTO, rx_buffer, 4+ sizeof(mspHeaderV2_t) + j);
#endif /* DEBUG_MSP_PROTO */
                        (void)udp_send_msg(rx_buffer, 4+ sizeof(mspHeaderV2_t) + j);

                        return ESP_OK;

                        break;
                        
                    case MSP_V1:
                    /* FALL THROUGH */
                    case MSP_V2_OVER_V1:
                    /* FALL THROUGH */
                    default:
                        break;
                }
            }
        }
    }

    return ESP_FAIL;
}

esp_err_t ttl_handle_wifi_nomsp_protocol(uint8_t * buf, int len)
{
    if(NULL == buf){
        return ESP_FAIL;
    }

#if (DEBUG_MSP_PROTO)
    ESP_LOGI(MODULE_MSP_PROTO, "ttl_handle_wifi_nomsp-exit %d bytes", len);
    esp_log_buffer_hex(MODULE_MSP_PROTO, buf, len);
#endif /* DEBUG_MSP_PROTO */
    (void)udp_send_msg(buf, len);

    return ESP_OK;
}

esp_err_t handle_msp_protocol(uint8_t * buf, int len)
{
    if (NULL == buf){
        return ESP_FAIL;
    }

    if (*buf != '$' || len < 9){
        return ESP_FAIL;
    }

    sbuf_t sbuf_hdrv2;
    mspHeaderV2_t hdrv2;
    
    sbufInit(&sbuf_hdrv2, buf + 3, buf + len);

    hdrv2.flags = sbufReadU8(&sbuf_hdrv2);
    hdrv2.cmd = sbufReadU16(&sbuf_hdrv2);
    hdrv2.size = sbufReadU16(&sbuf_hdrv2);

    if (0 == hdrv2.flags){
        mspSetMessage(MESSAGE_MSP);
    }

#if (DEBUG_MSP_PROTO)
    ESP_LOGI(MODULE_MSP_PROTO, "handle_msp_protocol %d bytes cmd 0x%04x-%d flag %d size %d", 
                                                len, hdrv2.cmd, hdrv2.cmd, hdrv2.flags, hdrv2.size);
    esp_log_buffer_hex(MODULE_MSP_PROTO, buf, len);
#endif /* DEBUG_MSP_PROTO */

    ESP_ERROR_CHECK(ttl_send(buf, len));

    return ESP_OK;
}

esp_err_t center_handle_msp_protocol(uint8_t * buf, int len)
{
#if (DEBUG_MSP_PROTO)
        ESP_LOGI(MODULE_MSP_PROTO, "center_handle %d bytes", len);
        esp_log_buffer_hex(MODULE_MSP_PROTO, buf, len);
#endif /* DEBUG_MSP_PROTO */

    return ESP_OK;
}


static void message_center_task(void *pvParameters)
{
    while (1) {
        if (SW_STATE_CLI != snap_sw_state_get() && !snap_sw_state_active(SW_MODE_BT_SPP) && snap_sw_command_get()){
            /* Used for Air Unit RC control in WiFi AP/STA MSP comunication */
            ESP_ERROR_CHECK(mspUpdateChannels());
        }

        /*
         * around 10Hz > 5Hz RC commands update rate
         * according to https://blog.csdn.net/lida2003/article/details/128328444
         */
        vTaskDelay(TIME_100_MS / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}


esp_err_t start_message_center(void)
{
    xTaskCreate(message_center_task, MODULE_MSP_PROTO, TASK_BUFFER_2K0, NULL, 5, NULL);

    return ESP_OK;
}


