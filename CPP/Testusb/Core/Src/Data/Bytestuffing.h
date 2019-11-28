/*
 * Bytestuffing.h
 *
 *  Created on: Nov 27, 2019
 *      Author: Bas
 */

#ifndef BYTESTUFFING_H_
#define BYTESTUFFING_H_

#include <vector>
#include <stdint.h>
#include "callback.h"

#define _SOF '&'
#define _EOF '%'
#define _ESC '\\'
#define _NOP '*'

class ByteStuffing
{


	bool startFound = false;
	bool esc = false;

	std::vector<uint8_t> dataBuffer;

public:
	Callback<void, std::vector<uint8_t>*> FrameComplete;

	bool UnStuff(std::vector<uint8_t> *dataIn)
	{
		int len = dataIn->size();
		for(int i=0; i<len; i++)
		{
			bool record = false;
			if(esc)
			{
				record = true;
				esc = false;
			}
			else
			{
				switch((*dataIn)[i])
				{
				case _ESC:
					esc = true;
					break;
				case _SOF:
					startFound = true;
					dataBuffer.clear();
					break;
				case _EOF:
					startFound = false;
					FrameComplete(&dataBuffer);
					break;
				case _NOP:
					break;
				default:
					record = true;
					break;
				}
			}

			if(record && startFound)
				dataBuffer.push_back((*dataIn)[i]);

			/*
			try
			{

			}
			catch(std::bad_alloc const&)
			{
				return false;
			}*/
		}
		return true;
	}

	void Stuff(std::vector<uint8_t> *dataIn, std::vector<uint8_t> *dataOut)
	{
		int len = dataIn->size();
		dataOut->push_back(_SOF); //Start of frame
		for(int i=0; i<len; i++)
		{
			if((*dataIn)[i] == _SOF || (*dataIn)[i] == _EOF || (*dataIn)[i] == _ESC|| (*dataIn)[i] == _NOP)
				dataOut->push_back(_ESC);

			dataOut->push_back((*dataIn)[i]);
		}
		dataOut->push_back(_EOF); //End of frame
	}
};


#endif
