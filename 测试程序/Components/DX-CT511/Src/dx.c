#include "dx.h"
#include "dx_io.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static uint8_t AtCmd[MAX_AT_CMD_SIZE];
uint8_t RxBuffer[MAX_BUFFER_SIZE];

static DX_StatusTypeDef runAtCmd(uint8_t *cmd, uint32_t Length, const uint8_t *Token);

DX_StatusTypeDef DX_Init(void)
{
  DX_StatusTypeDef Ret;
  char *Token;

  DX_IO_Init();

  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "ATE0%c%c", '\r', '\n');

  Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
  if (Ret != DX_OK)
  {
    return DX_ERROR;
  }

  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+CIPMODE=0%c%c", '\r', '\n');

  Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
  if (Ret != DX_OK)
  {
    return DX_ERROR;
  }

  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+NETOPEN?%c%c", '\r', '\n');

  Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
  if (Ret != DX_OK)
  {
    return DX_ERROR;
  }
  else
  {
    Token = strstr((char *)RxBuffer, "+NETOPEN:");
    Token += 9;

    if (*Token != '1')
    {
      memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
      sprintf((char *)AtCmd, "AT+NETOPEN%c%c", '\r', '\n');

      Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_SUCCESS_STRING);
      if (Ret != DX_OK)
      {
        return DX_ERROR;
      }
    }
  }

  return DX_OK;
}

DX_StatusTypeDef DX_SoftReset(void)
{
  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+RESET%c%c", '\r', '\n');

  return runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
}

DX_StatusTypeDef DX_GetDeviceInfo(DX_DeviceInfo *deviceInfo)
{
  DX_StatusTypeDef Ret;
  int i = 0;
  char *Token;

  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "ATI%c%c", '\r', '\n');

  Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
  if (Ret != DX_OK)
  {
    return DX_ERROR;
  }

  Token = strstr((char *)RxBuffer, "Manufacturer:");

  // Manufacturer
  Token += 14;
  while (*Token != '\"' && i < sizeof(deviceInfo->manufacturer) - 1)
  {
    deviceInfo->manufacturer[i++] = *Token++;
  }
  deviceInfo->manufacturer[i] = '\0';

  // Model
  Token += 10;
  i = 0;
  while (*Token != '\"' && i < sizeof(deviceInfo->model) - 1)
  {
    deviceInfo->model[i++] = *Token++;
  }
  deviceInfo->model[i] = '\0';

  // Revision
  Token += 12;
  i = 0;
  while (*Token != '\r' && i < sizeof(deviceInfo->revision) - 1)
  {
    deviceInfo->revision[i++] = *Token++;
  }
  deviceInfo->revision[i] = '\0';

  // IMEI
  Token += 8;
  i = 0;
  while (*Token != '\r' && i < sizeof(deviceInfo->imei) - 1)
  {
    deviceInfo->imei[i++] = *Token++;
  }
  deviceInfo->imei[i] = '\0';

  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+ICCID%c%c", '\r', '\n');

  Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
  if (Ret != DX_OK)
  {
    return DX_ERROR;
  }

  Token = strstr((char *)RxBuffer, "ICCID:");

  // ICCID
  Token += 6;
  i = 0;
  while (*Token != '\r' && i < sizeof(deviceInfo->iccid) - 1)
  {
    deviceInfo->iccid[i++] = *Token++;
  }
  deviceInfo->iccid[i] = '\0';

  return DX_OK;
}

DX_ConnectionStatusTypeDef DX_GetConnectionStatus(void)
{
  DX_StatusTypeDef Ret;
  char *Token;

  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+CEREG?%c%c", '\r', '\n');

  Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
  if (Ret != DX_OK)
  {
    return DX_UNKNOWN;
  }

  Token = strstr((char *)RxBuffer, "+CEREG:");
  Token += 10;
  if (*Token - '0' >= 0 && *Token - '0' <= 5)
  {
    return (DX_ConnectionStatusTypeDef)(*Token - '0');
  }
  else
  {
    return DX_UNKNOWN;
  }
}

DX_StatusTypeDef DX_GetRSSIandBER(uint8_t *rssi, uint8_t *ber)
{
  DX_StatusTypeDef Ret;
  char *Token;

  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+CSQ%c%c", '\r', '\n');

  Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
  if (Ret != DX_OK)
  {
    return Ret;
  }

  Token = strstr((char *)RxBuffer, "+CSQ:");
  Token += 6;
  if (Token[0] - '0' >= 0 && Token[0] - '0' <= 9)
  {

    *rssi = Token[0] - '0';
    if (Token[1] - '0' >= 0 && Token[1] - '0' <= 9)
    {
      *rssi = *rssi * 10 + (Token[1] - '0');
      Token++;
    }

    Token += 2;
    if (Token[0] - '0' >= 0 && Token[0] - '0' <= 9)
    {

      *ber = Token[0] - '0';
      if (Token[1] - '0' >= 0 && Token[1] - '0' <= 9)
      {
        *ber = *ber * 10 + (Token[1] - '0');
      }
    }
  }

  return DX_OK;
}

DX_StatusTypeDef DX_GetDateTime(DX_DateTime *dt)
{

  DX_StatusTypeDef Ret;
  char *Token;

  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+CCLK?%c%c", '\r', '\n');

  Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
  if (Ret != DX_OK)
  {
    return Ret;
  }

  Token = strstr((char *)RxBuffer, "+CCLK:");
  Token += 8;

  dt->year = (Token[0] - '0') * 10 + (Token[1] - '0');
  dt->month = (Token[3] - '0') * 10 + (Token[4] - '0');
  dt->day = (Token[6] - '0') * 10 + (Token[7] - '0');
  dt->hour = (Token[9] - '0') * 10 + (Token[10] - '0');
  dt->minute = (Token[12] - '0') * 10 + (Token[13] - '0');
  dt->second = (Token[15] - '0') * 10 + (Token[16] - '0');
  dt->timezone = ((Token[18] - '0') * 10 + (Token[19] - '0'));

  return DX_OK;
}

DX_StatusTypeDef DX_SetAPN(char *apn, char *username, char *password)
{
  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\"%c%c", apn, username, password, '\r', '\n');

  /* Send the command */
  return runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
}

DX_StatusTypeDef DX_NetOpen(void)
{
  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+NETOPEN%c%c", '\r', '\n');

  /* Send the command */
  return runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
}

DX_StatusTypeDef DX_NetClose(void)
{
  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+NETCLOSE%c%c", '\r', '\n');

  /* Send the command */
  return runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_OK_STRING);
}

DX_StatusTypeDef DX_NetConnOpen(uint8_t i, char *type, char *ip, uint16_t port)
{
  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+CIPOPEN=%d,\"%s\",\"%s\",%d%c%c", i, type, ip, port, '\r', '\n');

  /* Send the command */
  return runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_SUCCESS_NO_TAG);
}

DX_StatusTypeDef DX_NetConnClose(uint8_t i)
{
  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+CIPCLOSE=%d%c%c", i, '\r', '\n');

  /* Send the command */
  return runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)AT_SUCCESS_NO_TAG);
}

uint16_t DX_NetConnRead(uint8_t i,char *buf)
{
  DX_StatusTypeDef Ret;
  char *Token;

  uint32_t idx = 0;
  uint8_t RxChar;
  
  uint16_t cnt = 0;

  Ret = runAtCmd(AtCmd, 0, (uint8_t *)AT_SUCCESS_NO_TAG);
  if (Ret == DX_OK)
  {
    memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
    sprintf((char *)AtCmd, ",%d,0,", i);

    while (DX_IO_Receive(&RxChar, 1) != 0)
    {
      RxBuffer[idx++] = RxChar;
      if (idx == MAX_BUFFER_SIZE)
      {
        break;
      }
    }

    Token = strstr((char *)RxBuffer, (char const *)AtCmd);
    if (Token != NULL)
    {
      Token += 5;
      
      if (Token[4] == ',')
      {
        cnt = (Token[3] - '0') * 1000 + (Token[2] - '0') * 100 + (Token[1] - '0') * 10 + (Token[2] - '0');
        memcpy(buf,Token+5,cnt);
      }else if (Token[3] == ','){
        cnt = (Token[2] - '0') * 100 + (Token[1] - '0') * 10 + (Token[2] - '0');   
         memcpy(buf,Token+4,cnt);
      }else if (Token[2] == ','){
        cnt = (Token[1] - '0') * 10 + (Token[2] - '0');   
         memcpy(buf,Token+3,cnt);
      }else if (Token[1] == ','){
        cnt = (Token[2] - '0');   
         memcpy(buf,Token+2,cnt);
      }
        return cnt;
      
      
    }
  }

  return 0;
}

DX_StatusTypeDef DX_NetConnWrite(uint8_t i, uint16_t len, char *buf)
{
  DX_StatusTypeDef Ret;

  memset(AtCmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)AtCmd, "AT+CIPSEND=%i,%d%c%c", i, len, '\r', '\n');

  /* Send the command */
  Ret = runAtCmd(AtCmd, strlen((char *)AtCmd), (uint8_t *)">");
  if (Ret != DX_OK)
  {
    return Ret;
  }

  DX_IO_Send((uint8_t *)buf, len);

  return DX_OK;
}

/**
 * @brief  Run the AT command
 * @param  cmd the buffer to fill will the received data.
 * @param  Length the maximum data size to receive.
 * @param  Token the expected output if command runs successfully
 * @retval Returns DX_OK on success and DX_ERROR otherwise.
 */
static DX_StatusTypeDef runAtCmd(uint8_t *cmd, uint32_t Length, const uint8_t *Token)
{
  uint32_t idx = 0;
  uint8_t RxChar;

  /* Reset the Rx buffer to make sure no previous data exist */
  memset(RxBuffer, '\0', MAX_BUFFER_SIZE);

  /* Send the command */
  if (Length > 0)
  {
    if (DX_IO_Send(cmd, Length) < 0)
    {
      return DX_ERROR;
    }
  }

  /* Wait for reception */
  while (1)
  {
    /* Wait to receive data */
    if (DX_IO_Receive(&RxChar, 1) != 0)
    {
      RxBuffer[idx++] = RxChar;
    }
    else
    {
      break;
    }

    /* Check that max buffer size has not been reached */
    if (idx == MAX_BUFFER_SIZE)
    {
      break;
    }

    /* Extract the Token */
    if (strstr((char *)RxBuffer, (char *)Token) != NULL)
    {
      return DX_OK;
    }

    /* Check if the message contains error code */
    if (strstr((char *)RxBuffer, AT_ERROR_STRING) != NULL)
    {
      return DX_ERROR;
    }
  }

  return DX_ERROR;
}