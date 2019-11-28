/*
 * BVUSB.h
 *
 *  Created on: Nov 28, 2019
 *      Author: Bas
 */

#ifndef SRC_LIB_BVUSB_H_
#define SRC_LIB_BVUSB_H_

#include "cmsis_os.h"
#include "usb_device.h"
#include "usb.h"
#include "stdint.h"
#include <map>
#include "Comminucation/BVProtocol.h"
#include "Comminucation/Command.h"



class BVUSB
{

	typedef struct
	{
		uint8_t data[64];
	}CMDMessage;


	USBTask<CMDMessage> * usb;
	BVProtocol prot;


	void DataRecieved(CMDMessage msg)
	{
		std::vector<uint8_t> data;

		for(int i=0; i<64; i++)
			data.push_back(msg.data[i]);

		prot.RawDataIn(&data);
	}


	void CommandReceived(Command cmd)
	{
		OnCommandRecieved(cmd);
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

public:

	BVUSB()
	{
		usb = new USBTask<CMDMessage>();
		usb->OnDataRecieved.Bind(this, &BVUSB::DataRecieved);
		prot.OnCommandRecieved.bind(this, &BVUSB::CommandReceived);
		prot.OnRawDataOut.bind(this, &BVUSB::DataSend);
	}


	bool SendRequest(uint8_t cmdNo, std::vector<uint8_t>* data, void(*fp)(Command))
	{
		return prot.SendRequest(cmdNo, data, fp);
	}

	void SendResponse(ResponseType response, std::vector<uint8_t>* data)
	{
		prot.SendResponse(response, data);
	}

	Callback<void, Command> OnCommandRecieved;
};



#endif /* SRC_LIB_BVUSB_H_ */
