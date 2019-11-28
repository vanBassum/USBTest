/*
 * TaskCPP.h
 *
 *  Created on: Nov 8, 2019
 *      Author: Bas
 */
//Took some advise from:
//https://www.freertos.org/FreeRTOS_Support_Forum_Archive/July_2010/freertos_Is_it_possible_create_freertos_task_in_c_3778071.html

#ifndef _TaskCPP_H_
#define _TaskCPP_H_


#include <FreeRTOS.h>
#include <task.h>


class Task
{


private:
	char const *_name;
	portBASE_TYPE _priority;
	portSHORT _stackDepth;
	xTaskHandle taskHandle;

	static void taskfun(void* parm)
	{
		Task* t = static_cast<Task*>(parm);
		t->Work();
		t->taskHandle = NULL;
		vTaskDelete(NULL);
	}

public:



	/// <summary>
	/// Constructor of the task.
	/// </summary>
	/// <param name="name">A string representing the task.</param>
	/// <param name="priority">The priority of the task.</param>
	/// <param name="stackDepth">The size of the stack. (For ESP-IDF this is in bytes. Check the manual of FREERTOS for clarity.)</param>
	Task(const char *name, portBASE_TYPE priority, portSHORT stackDepth=configMINIMAL_STACK_SIZE)
	{
		if(stackDepth < configMINIMAL_STACK_SIZE)
			stackDepth = configMINIMAL_STACK_SIZE;

		_name = name;
		_priority = priority;
		_stackDepth = stackDepth;
		taskHandle = NULL;
		//Don't create the task yet. To prevent the task beeing started before the constructor is finished.
		//xTaskCreate(&taskfun, name, stackDepth, this, priority, &handle);
	}

	/// <summary>
	/// Deconstructor.
	/// </summary>
	virtual ~Task()
	{
		if(taskHandle != NULL)
			vTaskDelete(taskHandle);
		return;
	}

	/// <summary>
	/// Used to start the task.
	/// This should only be called after the initializer is done.
	/// Otherwise the task could run before the initializer is finished resulting in possible hardfaults.
	/// </summary>
	void Run()
	{
		BaseType_t xReturned = xTaskCreate(&taskfun, _name, _stackDepth, this, _priority, &taskHandle);
	}


	void Delay(int timeMs)
	{
		vTaskDelay(timeMs / portTICK_PERIOD_MS);
	}

	/// <summary>
	/// Function that is run by the task.
	/// When focus is lost, the task will be stopped and deleted.
	/// </summary>
	virtual void Work() = 0;

};

#endif
