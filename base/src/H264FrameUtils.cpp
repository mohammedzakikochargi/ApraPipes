#include "stdafx.h"
#include <boost/asio/buffer.hpp>
#include <iostream>
#include "H264FrameUtils.h"
#include "Frame.h"
#include "FrameMetadata.h"
#include "FrameContainerQueue.h"
#include "H264Utils.h"
#include "Logger.h"

using namespace std;
const_buffer H264FrameUtils::parseNALU(mutable_buffer& input, short &typeFound)
{
	typeFound = 0;
	const char* p1 = static_cast<const char*>(input.data());
	size_t offset = 0;
	if (H264Utils::getNALUnit(p1, input.size(), offset)) //where does it start
	{
		input += offset;
		offset = 0;
		p1 = static_cast<const char*>(input.data());
		if (H264Utils::getNALUnit(p1, input.size(), offset)) //where does it end
		{
			//we see 0 0 0 1 as well as 0 0 1
			size_t nSize = offset - 3;
			if (p1[offset - 4] == 0x00) nSize--;
			//since we are here lets find the next type
			typeFound = H264Utils::getNALUType(p1 + offset - 4); //always looks at 5th byte
			input += nSize;
			return const_buffer(p1, nSize);
		}
	}
	return input;
}
frame_container H264FrameDemuxer::on_pop_success(frame_container item)
{
	auto h264Frame = item.begin()->second;
	if (myState == INITIAL &&
		(h264Frame->mFrameType == H264Utils::H264_NAL_TYPE_SEQ_PARAM
			|| h264Frame->mFrameType == H264Utils::H264_NAL_TYPE_ACCESS_UNIT))
	{
		short nextTypeFound = h264Frame->mFrameType;
		mutable_buffer& frame = *(h264Frame.get());
		const char* beg = static_cast<const char*>(frame.data());//for sps_pps
		while (nextTypeFound != H264Utils::H264_NAL_TYPE_IDR_SLICE && nextTypeFound != 0)
		{
			short type = nextTypeFound;
			const_buffer nalu = parseNALU(frame, nextTypeFound);
			if (type == H264Utils::H264_NAL_TYPE_ACCESS_UNIT)
			{
				beg = static_cast<const char*>(nalu.data()) + nalu.size();
				LOG_TRACE << "ignore H264_NAL_TYPE_ACCESS_UNIT";
			}
			else if (type == H264Utils::H264_NAL_TYPE_SEQ_PARAM)
			{
				sps = nalu;
				myState = SPS_RCVD;
				h264Frame->mFrameType = type;
			}
			else if (type == H264Utils::H264_NAL_TYPE_PIC_PARAM)
			{
				pps = nalu;
				myState = PPS_RCVD;
				h264Frame->mFrameType = type;

				//also initialize sps_pps
				const char* end = static_cast<const char*>(pps.data());//for sps_pps
				sps_pps = const_buffer(beg, (end - beg) + pps.size());
			}
		}
		if (nextTypeFound == H264Utils::H264_NAL_TYPE_IDR_SLICE)
		{
			myState = NORMAL;
			h264Frame->mFrameType = nextTypeFound;
		}
	}
	return item;
}