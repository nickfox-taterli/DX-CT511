#include "stm32h5xx.h"
#include "stm32h5xx_ll_usart.h"
#include "stm32h5xx_ll_gpio.h"
#include "stm32h5xx_ll_rcc.h"
#include "stm32h5xx_ll_bus.h"

#include "dx_io.h"

#define RING_BUFFER_SIZE 4096

typedef struct
{
  uint8_t *pData;
  int32_t index;
  int32_t pos;
} Buffer_t;

typedef struct
{
  uint8_t data[RING_BUFFER_SIZE];
  uint16_t tail;
  uint16_t head;
} RingBuffer_t;

static Buffer_t DX_TxBuffer;
static RingBuffer_t DX_RxBuffer;

void DX_IO_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA1   ------> USART1_RX
  PA2   ------> USART1_TX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1 | LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);

  /* 对于TX是剩余N个,对于RX是还有N个满. */
  LL_USART_SetTXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_8_8);
  LL_USART_SetRXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_7_8);
  LL_USART_EnableFIFO(USART1);

  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);

  /* 只有需要发送时才打开! */
  // LL_USART_EnableIT_TXFE(USART1);
  LL_USART_EnableIT_RXFT(USART1);
  LL_USART_EnableIT_IDLE(USART1);

  /* USART1 interrupt Init */
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(USART1_IRQn);
}

int8_t DX_IO_Send(uint8_t *pData, uint32_t Length)
{
  if (DX_TxBuffer.pData != 0)
  {
    /* 有数据正在传输 */
    return -1;
  }

  DX_TxBuffer.pData = pData;
  DX_TxBuffer.index = 0;
  DX_TxBuffer.pos = Length - 8;

  while (DX_TxBuffer.index < Length)
  {
    LL_USART_TransmitData8(USART1, DX_TxBuffer.pData[DX_TxBuffer.index++]);
    if (DX_TxBuffer.index == 8)
    {
      LL_USART_EnableIT_TXFE(USART1);
      DX_TxBuffer.pos = Length - 8;
      break;
    }
  }

  if (DX_TxBuffer.pos <= 0)
  {
    DX_TxBuffer.pData = 0;
  }

  return 0;
}

int8_t DX_IO_Receive(uint8_t *Buffer, uint32_t Length)
{
  uint32_t ReadData = 0;
  uint32_t tick = 0;

  while (Length--)
  {
    do
    {
      tick++;
      if (DX_RxBuffer.head != DX_RxBuffer.tail)
      {
        *Buffer++ = DX_RxBuffer.data[DX_RxBuffer.head++];
        ReadData++;

        if (DX_RxBuffer.head >= RING_BUFFER_SIZE)
        {
          DX_RxBuffer.head = 0;
        }
        break;
      }
    } while (tick < DEFAULT_TIME_OUT);
  }

  return ReadData;
}

void USART1_IRQHandler(void)
{
  if (LL_USART_IsEnabledIT_RXFT(USART1) && LL_USART_IsActiveFlag_RXFT(USART1))
  {
    while (LL_USART_IsActiveFlag_RXNE(USART1))
    {
      DX_RxBuffer.data[DX_RxBuffer.tail] = LL_USART_ReceiveData8(USART1);

      if (++DX_RxBuffer.tail >= RING_BUFFER_SIZE)
      {
        DX_RxBuffer.tail = 0;
      }
    };
  }
  else if (LL_USART_IsEnabledIT_IDLE(USART1) && LL_USART_IsActiveFlag_IDLE(USART1))
  {
    LL_USART_ClearFlag_IDLE(USART1);

    while (LL_USART_IsActiveFlag_RXNE(USART1))
    {
      DX_RxBuffer.data[DX_RxBuffer.tail] = LL_USART_ReceiveData8(USART1);

      if (++DX_RxBuffer.tail >= RING_BUFFER_SIZE)
      {
        DX_RxBuffer.tail = 0;
      }
    };
  }
  else if (LL_USART_IsEnabledIT_TXFE(USART1) && LL_USART_IsActiveFlag_TXFE(USART1))
  {
    LL_USART_ClearFlag_TXFE(USART1);

    while (DX_TxBuffer.pos-- > 0)
    {
      LL_USART_TransmitData8(USART1, DX_TxBuffer.pData[DX_TxBuffer.index++]);
      if (!LL_USART_IsActiveFlag_TXE_TXFNF(USART1))
      {
        break;
      }
    }

    if (DX_TxBuffer.pos == -1)
    {
      DX_TxBuffer.pData = 0;
      LL_USART_DisableIT_TXFE(USART1);
    }
  }
}
