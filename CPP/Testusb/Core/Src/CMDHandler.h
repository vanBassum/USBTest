/*
 * CMDHandler.h
 *
 *  Created on: Nov 25, 2019
 *      Author: Bas
 */

#ifndef SRC_CMDHANDLER_H_
#define SRC_CMDHANDLER_H_


#include "Lib/usb.h"
#include "stdint.h"
#include <map>








class CMDHandler
{
	typedef  void(CMDHandler::*method)(CMDMessage*);
	USBTask<CMDMessage> * usb;
	std::map<uint8_t, method> functions;


public:

	void CommandRecieved(CMDMessage msg)
	{
		if(functions.count(msg.cmd) > 0)
		{
			method m = functions[msg.cmd];
			(this->*m)(&msg);
		}

	}

	void ReportStackSize(CMDMessage *data)
	{
		CMDMessage msg;
		msg.cmd = data->cmd;

		size_t heap = xPortGetFreeHeapSize();

		memcpy(&msg.data, &heap, sizeof(heap));

		usb->Send(msg);
	}


	CMDHandler()
	{
		functions[0] = &CMDHandler::ReportStackSize;

		usb = new USBTask<CMDMessage>();
		usb->OnDataRecieved.Bind(this, &CMDHandler::CommandRecieved);


	}

};





#endif /* SRC_CMDHANDLER_H_ */
