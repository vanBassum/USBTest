/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @version        : v2.0_Cube
  * @brief          : USB Device Custom HID interface file.
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
#include "usbd_custom_hid_if.h"
#include "../../Core/src/lib/Queue.h"
#include "../../Core/src/lib/USB.h"

__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{

	    0x06, 0x00, 0xff,              // USAGE_PAGE (Vendor Defined Page 1)
	    0x09, 0x01,                    // USAGE (Vendor Usage 1)
	    0xa1, 0x01,                    // COLLECTION (Application)

	    0x19, 0x01,                    //   USAGE_MINIMUM (Undefined)
	    0x29, 0x40,                    //   USAGE_MAXIMUM (Undefined)
	    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	    0x75, 0x08,                    //   REPORT_SIZE (8)
	    0x95, CUSTOM_HID_EPIN_SIZE,    //   REPORT_COUNT (64)
	    0x81, 0x02,                    //   INPUT (Data,Ary,Abs)

	    0x19, 0x01,                    //   USAGE_MINIMUM (Undefined)
	    0x29, 0x40,                    //   USAGE_MAXIMUM (Undefined)
	    0x75, 0x08,                    //   REPORT_SIZE (8)
	    0x95, CUSTOM_HID_EPOUT_SIZE,   //   REPORT_COUNT (64)
	    0x91, 0x02,                    //   OUTPUT (Data,Ary,Abs)
	    0xc0                           // END_COLLECTION

};

Queue<USBPackage> USBRecievedQueue(10);

static int8_t CUSTOM_HID_Init_FS(void);
static int8_t CUSTOM_HID_DeInit_FS(void);
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state);


USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};

static int8_t CUSTOM_HID_Init_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

static int8_t CUSTOM_HID_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)
{

	USBD_CUSTOM_HID_HandleTypeDef    *hhid = (USBD_CUSTOM_HID_HandleTypeDef*)hUsbDeviceFS.pClassData;

	USBPackage upack;

	memcpy(upack.data, hhid->Report_buf, 64);


	if(USBRecievedQueue.Enqueue(&upack, 0))
	{
		//todo what if the queue is full?
	}

	return (USBD_OK);
}
