/*
 * CommandHandler.h
 *
 *  Created on: Nov 28, 2019
 *      Author: Bas
 */

#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_


#include <stdint.h>
#include <vector>
#include <map>

#include "Lib/Comminucation/Callback.h"
#include "Lib/Comminucation/Command.h"



class CommandHandler
{
private:
	std::map<uint8_t, Callback<bool, std::vector<uint8_t>*>> commandList;

public:

	template<typename T>
	bool SetCommand(uint8_t cmd, T *instance, bool (T::*Method)(std::vector<uint8_t>* data))
	{
		if(cmd & 0x80)
			return false;

		if(commandList.count(cmd))
			return false;

		commandList[0].bind(instance, Method);
		return true;
	}

	bool SetCommand(uint8_t cmd, bool (*func)(std::vector<uint8_t>* data))
	{
		if(cmd & 0x80)
			return false;

		if(commandList.count(cmd))
			return false;

		commandList[0].bind(func);
		return true;
	}


	CommandHandler()
	{
		//SetCommand(0x00, this, &CommandHandler::TestCommand);
	}

	ResponseType ExecCommand(Command *cmd, std::vector<uint8_t>* data)
	{

		if(commandList.count(cmd->GetCommand()))
		{
			if(commandList[cmd->GetCommand()](data))
				return ResponseType::Ack;
			else
				return ResponseType::Nack;
		}

		return ResponseType::Unknown;
	}
};





#endif 
