#include "stdafx.h"
#include <boost/asio/buffer.hpp>
#include <iostream>
#include "H264FrameUtils.h"
#include "Frame.h"
#include "FrameMetadata.h"
#include "FrameContainerQueue.h"
#include "Mp4WriterSink.h"
#include "H264Utils.h"
#include "Logger.h"

using namespace std;
bool H264FrameUtils::getNALUnit(const char* buffer, size_t length, size_t& offset)
{
	if (length < 3) return false;
	size_t cnt = 3;

	while (cnt < length)
	{
		if (buffer[cnt - 1] == 0x1 && buffer[cnt - 2] == 0x0 && buffer[cnt - 3] == 0x0)
		{
			offset = cnt;
			return true;
		}
		cnt++;
	}

	return false;
}
const_buffer H264FrameUtils::parseNALUU(mutable_buffer& input, short& typeFound,char*& spsBuffer,char*& ppsBuffer,size_t& spsSize,size_t& ppsSize, char* &Iframe)
{
	typeFound = 0;
	char* p1 = static_cast<char*>(input.data());
	Iframe = p1;
	size_t offset = 0;
	typeFound = H264Utils::getNALUType(p1);
	if (typeFound == H264Utils::H264_NAL_TYPE::H264_NAL_TYPE_IDR_SLICE)
	{
	return input;
	}
	if(typeFound == H264Utils::H264_NAL_TYPE::H264_NAL_TYPE_SEQ_PARAM)
	{
		if (H264FrameUtils::getNALUnit(p1, input.size(), offset)) //where does it start
		{
			input += offset;
			offset = 0;
			p1 = static_cast<char*>(input.data());
			if (H264FrameUtils::getNALUnit(p1, input.size(), offset)) //where does it end
			{
				char* spsBits = p1;
				//we see 0 0 0 1 as well as 0 0 1
				size_t nSize = offset - 3;
				if (p1[offset - 4] == 0x00)
					nSize--;
				spsSize = nSize;
				spsBuffer = (char*)malloc(spsSize);
				memcpy(spsBuffer, spsBits, spsSize);
				
				auto frame = makeFrame(spsSize);
				input += offset;
				p1 = static_cast<char*>(input.data());
				if (H264FrameUtils::getNALUnit(p1, input.size(), offset))
				{
					char* ppsBits = p1;
					size_t nSize = offset - 3;
					if (p1[offset - 4] == 0x00)
						nSize--;
					ppsSize = nSize;
					ppsBuffer = (char*)malloc(spsSize);
					memcpy(ppsBuffer, ppsBits, ppsSize);
					//since we are here lets find the next type
					typeFound = H264Utils::getNALUType(p1 + offset - 4); //always looks at 5th byte
					input += offset - 4;
					return const_buffer(p1, nSize);
				}

			}
		}
		
	}
	typeFound = H264Utils::getNALUType(p1 + offset - 4);
	return input;
}

