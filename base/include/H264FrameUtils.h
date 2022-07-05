#pragma once
#include <boost/asio/buffer.hpp>
#include "FrameContainerQueue.h"
using boost::asio::const_buffer;
using boost::asio::mutable_buffer;
class Frame;
class H264FrameUtils : public FrameContainerQueueAdapter {
	enum STATE {
		INITIAL,
		SPS_RCVD,
		PPS_RCVD,
		WAITING_FOR_IFRAME, // drops
		NORMAL
	};
	const_buffer parseNALU(mutable_buffer& input, short& typeFound);
	STATE myState;
	const_buffer sps, pps, sps_pps;

protected:
	frame_container on_pop_success(frame_container item);


public:
	H264FrameUtils() : myState(INITIAL) {}
	short getState() { return myState; }
};