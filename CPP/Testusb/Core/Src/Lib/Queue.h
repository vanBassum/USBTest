/*
 * MessageQueue.h
 *
 *  Created on: Nov 25, 2019
 *      Author: Bas
 */

#ifndef SRC_LIB_QUEUE_H_
#define SRC_LIB_QUEUE_H_

#include <freertos.h>
#include <queue.h>

template<typename T>
class Queue
{
	QueueHandle_t queueHandle;


public:


	Queue(int size)
	{
		queueHandle = xQueueCreate(10, sizeof(T));
	}

	bool Enqueue(T *item, int timeout)
	{
		BaseType_t xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;

		if(	xPortIsInsideInterrupt())
			return xQueueSendFromISR(queueHandle, item, &xHigherPriorityTaskWoken) == pdTRUE ;
		else
			return xQueueSend(queueHandle, item, timeout / portTICK_PERIOD_MS) == pdTRUE ;
	}

	bool Dequeue(T* item, int timeout)
	{
		if( xQueueReceive(queueHandle, item, timeout / portTICK_PERIOD_MS))
		{
			return true;
		}
		return false;
	}


};




#endif /* SRC_LIB_QUEUE_H_ */
