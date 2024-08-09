#ifndef __DX_IO_H
#define __DX_IO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32h5xx.h"

#define DEFAULT_TIME_OUT (50000000) /* loop count */

    void DX_IO_Init(void);
    int8_t DX_IO_Send(uint8_t *Buffer, uint32_t Length);
    int8_t DX_IO_Receive(uint8_t *Buffer, uint32_t Length);

#ifdef __cplusplus
}
#endif

#endif
