#include "stm32h5xx.h"

#include "stm32h5xx_ll_icache.h"
#include "stm32h5xx_ll_pwr.h"
#include "stm32h5xx_ll_crs.h"
#include "stm32h5xx_ll_rcc.h"
#include "stm32h5xx_ll_bus.h"
#include "stm32h5xx_ll_system.h"
#include "stm32h5xx_ll_exti.h"
#include "stm32h5xx_ll_cortex.h"
#include "stm32h5xx_ll_utils.h"
#include "stm32h5xx_ll_dma.h"
#include "stm32h5xx_ll_usart.h"
#include "stm32h5xx_ll_gpio.h"

#include "dx.h"

void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
  while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_5)
  {
  }

  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE0);
  while (LL_PWR_IsActiveFlag_VOS() == 0)
  {
  }
  LL_RCC_CSI_Enable();

  /* Wait till CSI is ready */
  while (LL_RCC_CSI_IsReady() != 1)
  {
  }

  LL_RCC_CSI_SetCalibTrimming(16);
  LL_RCC_PLL1_SetSource(LL_RCC_PLL1SOURCE_CSI);
  LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_4_8);
  LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL1_SetM(1);
  LL_RCC_PLL1_SetN(125);
  LL_RCC_PLL1_SetP(2);
  LL_RCC_PLL1_SetQ(2);
  LL_RCC_PLL1_SetR(2);
  LL_RCC_PLL1P_Enable();
  LL_RCC_PLL1_Enable();

  /* Wait till PLL is ready */
  while (LL_RCC_PLL1_IsReady() != 1)
  {
  }

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);

  /* Wait till System clock is ready */
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1)
  {
  }

  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);

  LL_Init1msTick(250000000);

  LL_SetSystemCoreClock(250000000);

  /* GPIO Ports Clock Enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOH);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
}

DX_DeviceInfo device;

char buf[1000] = "";
char request[] = "GET / HTTP/1.1\r\nHost: baidu.com\r\nConnection: close\r\n\r\n";

int main(void)
{
  NVIC_SetPriorityGrouping(3);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

  /* Configure the system clock */
  SystemClock_Config();

  LL_ICACHE_Enable();

  DX_Init();
  DX_GetDeviceInfo(&device);
  // DX_GetConnectionStatus();
  // DX_GetRSSIandBER(&rssi,&ber);
  // DX_GetDateTime(&dt);
  // DX_SetAPN("cmnbiot","","");

  // IF DX_ERROR , DELAY ANY RETRY;
  while (DX_NetConnOpen(0, "TCP", "183.2.172.185", 80) != DX_OK)
  {
    LL_mDelay(5000);
    DX_NetConnClose(0);
    LL_mDelay(1000);
  }

  // DX_NetConnClose(0);

  while (1)
  {
    LL_mDelay(3000);
    DX_NetConnWrite(0, sizeof(request), request);
    LL_mDelay(3000);
    DX_NetConnRead(0, buf);
  }
}
