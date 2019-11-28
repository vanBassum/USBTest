/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "Data/BVprotocol.h"
#include "Data/Command.h"
#include "Lib/usb.h"
#include "stdint.h"
#include <map>


extern "C"
{
	#include "st7735/st7735.h"
}


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void StartDefaultTask(void *argument);
void spiinit();

//CMDHandler *cmdHandler;

typedef struct
{
	uint8_t data[64];
}CMDMessage;



SPI_HandleTypeDef ST7735_SPI_PORT;

USBTask<CMDMessage> * usb;

BVProtocol prot;

void * operator new( size_t size )
{
    return pvPortMalloc(size);
}

void * operator new[]( size_t size )
{
    return pvPortMalloc( size );
}

void operator delete( void * ptr )
{
    vPortFree( ptr );
}

void operator delete[]( void * ptr )
{
    vPortFree( ptr );
}


void DataRecieved(CMDMessage msg)
{
	UBaseType_t size =  uxTaskGetStackHighWaterMark(NULL);
	std::vector<uint8_t> data;

	for(int i=0; i<64; i++)
		data.push_back(msg.data[i]);

	prot.RawDataIn(&data);
}


void CommandReceived(Command cmd)
{
	std::vector<uint8_t> data;
	prot.SendResponse(ResponseType::Unknown, &data);
}

void DataSend(std::vector<uint8_t>* data)
{

	for(uint16_t i=0; i<data->size();)
	{
		CMDMessage msg;
		uint8_t p=0;

		for(p=0; p<64; p++)
		{
			if(i+p < data->size())
				msg.data[p] = (*data)[i+p];
			else
				msg.data[p] = _NOP;
		}

		usb->Send(msg);

		i+=p;
	}

}


int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  osKernelInitialize();

  spiinit();


  ST7735_Init();
  ST7735_FillScreen(ST7735_BLACK);

  usb = new USBTask<CMDMessage>();
  usb->OnDataRecieved.Bind(&DataRecieved);
  prot.OnCommandRecieved.bind(&CommandReceived);
  prot.OnRawDataOut.bind(&DataSend);


  osKernelStart();

  while (1)
  {

  }
}

void spiinit()
{
	ST7735_SPI_PORT.Instance = SPI2;
	ST7735_SPI_PORT.Init.Mode = SPI_MODE_MASTER;
	ST7735_SPI_PORT.Init.Direction = SPI_DIRECTION_1LINE;
	ST7735_SPI_PORT.Init.DataSize = SPI_DATASIZE_8BIT;
	ST7735_SPI_PORT.Init.CLKPolarity = SPI_POLARITY_LOW;
	ST7735_SPI_PORT.Init.CLKPhase = SPI_PHASE_1EDGE;
	ST7735_SPI_PORT.Init.NSS = SPI_NSS_SOFT;
	ST7735_SPI_PORT.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	ST7735_SPI_PORT.Init.FirstBit = SPI_FIRSTBIT_MSB;
	ST7735_SPI_PORT.Init.TIMode = SPI_TIMODE_DISABLE;
	ST7735_SPI_PORT.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	ST7735_SPI_PORT.Init.CRCPolynomial = 10;

	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{

	}

}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}



static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
