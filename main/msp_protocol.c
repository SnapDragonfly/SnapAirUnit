
#include "esp_system.h"
#include "esp_log.h"

#include "module.h"
#include "mode.h"
#include "define.h"
#include "ttl.h"
#include "udp_server.h"
#include "msp_protocol.h"

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

#define MSP_V2_FRAME_ID         255

typedef enum {
    MSP_V1          = 0,
    MSP_V2_OVER_V1  = 1,
    MSP_V2_NATIVE   = 2,
    MSP_VERSION_COUNT
} mspVersion_e;

#define MSP_PORT_INBUF_SIZE 192

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


static mspPort_t esp_msp_port;

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

static bool mspSerialProcessReceivedData(mspPort_t *mspPort, uint8_t c)
{
    switch (mspPort->c_state) {
        default:
        case MSP_IDLE:      // Waiting for '$' character
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

esp_err_t ttl_handle_msp_protocol(uint8_t * buf, int len)
{
    if (NULL == buf){
        return ESP_FAIL;
    }

#if (DEBUG_MSP_PROTO)
    ESP_LOGI(MODULE_MSP_PROTO, "ttl_handle %d bytes", len);
    esp_log_buffer_hex(MODULE_MSP_PROTO, buf, len);
#endif /* DEBUG_MSP_PROTO */

    esp_msp_port.c_state = MSP_IDLE;

    for(int i = 0; i < len; i++){
        mspSerialProcessReceivedData(&esp_msp_port, *(buf +i));
        if(MSP_COMMAND_RECEIVED == esp_msp_port.c_state){
            if(snap_sw_state_active(SW_MODE_WIFI_AP) || snap_sw_state_active(SW_MODE_WIFI_STA)){
                uint8_t rx_buffer[128];
                switch(esp_msp_port.mspVersion){
                    case MSP_V2_NATIVE:
                        rx_buffer[0] = '$';
                        rx_buffer[1] = 'X';
                        rx_buffer[2] = '>';
                        int j = 0;
                        for(; j < sizeof(mspHeaderV2_t); j++){
                            rx_buffer[3 + j] = esp_msp_port.headBuf[j];
                        }
                        j = 0;
                        if (esp_msp_port.dataSize > 0){
                            for(; j < esp_msp_port.offset; j++){
                                rx_buffer[3 +sizeof(mspHeaderV2_t) + j] = esp_msp_port.inBuf[j];
                            }
                        }
                        rx_buffer[3 +sizeof(mspHeaderV2_t) + j] = esp_msp_port.checksum2;
                        udp_send_msg(rx_buffer, 4+ sizeof(mspHeaderV2_t) + j);
                        
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



esp_err_t udp_handle_msp_protocol(uint8_t * buf, int len)
{
    if (NULL == buf){
        return ESP_FAIL;
    }

    if (*buf != '$'){
        return ESP_FAIL;
    }

#if (DEBUG_MSP_PROTO)
    ESP_LOGI(MODULE_MSP_PROTO, "udp_handle %d bytes", len);
    esp_log_buffer_hex(MODULE_MSP_PROTO, buf, len);
#endif /* DEBUG_MSP_PROTO */

    ESP_ERROR_CHECK(ttl_send(buf, len));

    return ESP_OK;
}



