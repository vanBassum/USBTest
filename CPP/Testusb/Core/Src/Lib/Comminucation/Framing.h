/*
 * Bytestuffing.h
 *
 *  Created on: Nov 27, 2019
 *      Author: Bas
 */

#ifndef FRAMING_H_
#define FRAMING_H_

#include <vector>
#include <stdint.h>
#include "callback.h"
#include "FreeRTOS/taskcpp.h"
#include "FreeRTOS/Ringbuffercpp.h"
#include "event.h"

#define _SOF '&'
#define _EOF '\n'
#define _SEP '|'
#define _ESC '\\'
#define _NOP '*'




typedef std::vector<std::vector<uint8_t>> Frame;

class Framing : Task
{
private:
	bool startFound = false;
	bool esc = false;
	Frame frameBuffer;

public:
	ByteBuffer *rawDataIn = NULL;
	ByteBuffer *rawDataOut = NULL;
	Event<Frame*> OnFrameComplete;

	Framing() : Task("Framing", 7, 2048)
	{
		Run();
	}

	~Framing()
	{
		delete rawDataIn;
		delete rawDataOut;
	}

	void Work()
	{
		rawDataIn = new ByteBuffer(256);
		rawDataOut = new ByteBuffer(256);

		while(1)
		{
			size_t len;

			uint8_t *data;
			if((data = rawDataIn->Dequeue(&len, 16, 1000)) != NULL)
			{


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
						switch((char)data[i])
						{
						case _ESC:
							esc = true;
							break;
						case _SOF:
							ESP_LOGE("Framing", "SOF");
							startFound = true;
							frameBuffer.clear();
							break;
						case _SEP:
							ESP_LOGE("Framing", "SEP");
							frameBuffer.push_back(std::vector<uint8_t>());
							break;
						case _EOF:
							ESP_LOGE("Framing", "EOF");
							startFound = false;
							OnFrameComplete(&frameBuffer);
							break;
						case _NOP:
							break;
						default:
							record = true;
							break;
						}
					}
					if(record && startFound)
					{
						ESP_LOGE("Framing", "'%c'", (char) data[i]);
						(frameBuffer.end()-1)->push_back(data[i]);
						//TODO: Check if there is at least one item in the framebuffer!
					}
				}
				ESP_LOGE("Framing", "Disposeitem'");
				rawDataIn->DisposeItem(data);
			}
		}
	}


	bool add(uint8_t d)
	{
		//ESP_LOGI("framing", "enc: '%c'", (char)d);

		if(!rawDataOut->EnqueueByte(d, 1, 0)) return false;
		return true;
	}

	bool StuffFrame(Frame *frame)
	{
		if(!add(_SOF)) return false;

		for(auto it = frame->begin(); it != frame->end(); it++)
		{
			int len = it->size();
			uint8_t *data = &(*it)[0];
			for(int i=0; i<len; i++)
			{
				if(data[i] == _SOF || data[i] == _EOF || data[i] == _ESC|| data[i] == _NOP || data[i] == _SEP)
					if(!add(_ESC)) return false;

				if(!add(data[i])) return false;

			}

			if((it+1) != frame->end())
				if(!add(_SEP)) return false;
		}

		if(!add(_EOF)) return false;
		return true;
	}


/*
	bool StuffFrame(Frame *frame)
	{
		if(!rawDataOut->Enqueue(_SOF, 1, 1000)) return false;

		for(auto it = frame->begin(); it != frame->end(); it++)
		{
			int len = it->size();
			uint8_t *data = &(*it)[0];
			for(int i=0; i<len; i++)
			{
				if(data[i] == _SOF || data[i] == _EOF || data[i] == _ESC|| data[i] == _NOP || data[i] == _SEP)
					if(!rawDataOut->Enqueue(_ESC, 1, 1000)) return false;

				if(!rawDataOut->Enqueue(data[i], 1, 1000)) return false;

			}

			if(it != frame->end())
				if(!rawDataOut->Enqueue(_SEP, 1, 1000)) return false;
		}

		if(!rawDataOut->Enqueue(_EOF, 1, 1000)) return false;
		return true;
	}*/



};


#endif
