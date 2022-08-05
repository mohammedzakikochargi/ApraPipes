#pragma once
#include <boost/asio/buffer.hpp>
#include "FrameContainerQueue.h"
#include "Module.h"
using boost::asio::const_buffer;
using boost::asio::mutable_buffer;
class Frame;
class H264FrameUtils : public FrameContainerQueueAdapter
{
	enum STATE {
		INITIAL,
		SPS_RCVD,
		PPS_RCVD,
		WAITING_FOR_IFRAME, // drops
		NORMAL
	};
	STATE myState;
	const_buffer sps, pps, sps_pps;
public:
	H264FrameUtils(){}
	H264FrameUtils(std::function<frame_sp(size_t size)> _makeFrame) 
	{
		makeFrame = _makeFrame;
	}
	const_buffer parseNALUU(mutable_buffer& input, short& typeFound ,char*& spsBuffer,char*& ppsBuffer , size_t& spsSize, size_t& ppsSize, char* &Iframe);
	static bool getNALUnit(const char* buffer, size_t length, size_t& offset);
	std::function<frame_sp(size_t size)> makeFrame;
};