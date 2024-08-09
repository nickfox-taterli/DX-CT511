#ifndef __DX_H
#define __DX_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "stm32h5xx.h"

#define MAX_BUFFER_SIZE 1500
#define MAX_AT_CMD_SIZE 256
#define AT_OK_STRING "OK\r\n"
#define AT_SUCCESS_STRING "SUCCESS\r\n"
#define AT_SUCCESS_NO_TAG "SUCCESS"
#define AT_ERROR_STRING "ERROR\r\n"

typedef enum
{
    DX_OK = 0,
    DX_ERROR = 1,
} DX_StatusTypeDef;

typedef enum
{
    DX_UNREGISTERED = 0,
    DX_REGISTERED = 1,
    DX_SEARCHING = 2,
    DX_REJECTED = 3,
    DX_UNKNOWN = 4,
    DX_ROAMING = 5
} DX_ConnectionStatusTypeDef;

typedef struct
{
    char manufacturer[8];
    char model[20];
    char revision[24];
    char imei[16];
    char iccid[24];
} DX_DeviceInfo;

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t timezone;
} DX_DateTime;

DX_StatusTypeDef DX_Init(void);
DX_StatusTypeDef DX_SoftReset(void);

DX_StatusTypeDef DX_GetDeviceInfo(DX_DeviceInfo *deviceInfo);
DX_ConnectionStatusTypeDef DX_GetConnectionStatus(void);

DX_StatusTypeDef DX_GetRSSIandBER(uint8_t *rssi, uint8_t *ber);
DX_StatusTypeDef DX_GetDateTime(DX_DateTime *dt);

DX_StatusTypeDef DX_SetAPN(char *apn, char *username, char *password);

DX_StatusTypeDef DX_NetOpen(void);
DX_StatusTypeDef DX_NetClose(void);

DX_StatusTypeDef DX_NetConnOpen(uint8_t i /* 0 -2 */, char *type /* only TCP implement */, char *ip, uint16_t port);
DX_StatusTypeDef DX_NetConnClose(uint8_t i /* 0 -2 */);
uint16_t DX_NetConnRead(uint8_t i /* 0 -2 */,char *buf);
DX_StatusTypeDef DX_NetConnWrite(uint8_t i /* 0 -2 */, uint16_t len, char *buf);

#ifdef __cplusplus
}
#endif

#endif
