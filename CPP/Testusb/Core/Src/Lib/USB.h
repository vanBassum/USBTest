/*
 * USB.h
 *
 *  Created on: Nov 25, 2019
 *      Author: Bas
 */

#ifndef SRC_LIB_USB_H_
#define SRC_LIB_USB_H_

#include "TaskCPP.h"
#include "usb_device.h"
#include "usbd_customhid.h"
#include "event.h"
#include "Queue.h"
#include <vector>

typedef struct
{
	uint8_t data[64];
}USBPackage;




extern USBD_HandleTypeDef hUsbDeviceFS;
extern Queue<USBPackage> USBRecievedQueue;


template<typename T>
class USBTask : public Task
{

public:

	USBTask() : Task("USBTask", 7, 1536)
	{
		Run();
	}

	void Send(T msg)
	{
		uint8_t rawMsg[64];

		memcpy(&rawMsg, &msg, sizeof(rawMsg));

		USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&rawMsg, sizeof(msg));
	}

	void Work()
	{
		MX_USB_DEVICE_Init();

		while(1)
		{

			USBPackage package;

			if(USBRecievedQueue.Dequeue(&package, 1000))
			{
				T msg;
				memcpy((void *)&msg, (void *)&package, sizeof(T));
				OnDataRecieved(msg);
			}

		}
	}

	Event<T> OnDataRecieved;

};



#endif /* SRC_LIB_USB_H_ */
