#include <boost/foreach.hpp>
#include "FramesMuxer.h"
#include "Frame.h"
#include "MultimediaQueue.h"
#include "Logger.h"
#include "AIPExceptions.h"
class MultimediaQueueStrategy
{
public:
	MultimediaQueueStrategy(MultimediaQueueProps& _props) {}
	virtual ~MultimediaQueueStrategy()
	{
	}
	virtual std::string addInputPin(std::string& pinId)
	{
		return getMultimediaQueuePinId(pinId);
	}
	virtual bool queue(frame_container& frames)
	{
		return true;
	}
	virtual bool get(frame_container& frames)
	{
		return false;
	}
protected:
	std::string getMultimediaQueuePinId(const std::string& pinId)
	{
		return pinId + "_mul_";
	}
};
//Strategy begins here
class TimeStampStrategy : public MultimediaQueueStrategy
{
public:
	TimeStampStrategy(MultimediaQueueProps& _props) :MultimediaQueueStrategy(_props), maxQueueLength(_props.maxQueueLength) {}
	~TimeStampStrategy()
	{
		clear();
	}
	std::string addInputPin(std::string& pinId)
	{
		boost::multi_index
		mQueue[pinId] = boost::container::deque<frame_sp>();
		return MultimediaQueueStrategy::addInputPin(pinId);
	}
	bool queue(frame_container& frames)
	{
		// add all the frames to the que
		// store the most recent fIndex
		double largestTimeStamp = 0;
		for (auto it = frames.cbegin(); it != frames.cend(); it++)
		{
			mQueue[it->first].push_back(it->second);
			if (largestTimeStamp < it->second->timestamp)
			{
				largestTimeStamp = it->second->timestamp;
			}
		}
		// loop over the que and remove old frames using maxQueueLength
		for (auto it = mQueue.begin(); it != mQueue.end(); it++)
		{
			auto& frames_arr = it->second;
			while (frames_arr.size())
			{
				auto& frame = frames_arr.front();
				// if (frame->timeStamp < firstHighestTimeStamp || largestTimeStamp - frame->timeStamp > maxTsDelay)
				if (largestTimeStamp - frame->timestamp > maxQueueLength)
				{
					// LOG_ERROR << "Dropping Frames";
					frames_arr.pop_front();
				}
				else
				{
					break;
				}
			}
		}
		return true;
	}
	bool get(frame_container& frames)
	{
		bool allFound = true;
		size_t fIndex = 0;
		bool firstIter = true;
		for (auto it = mQueue.begin(); it != mQueue.end(); it++)
		{
			auto& frames_arr = it->second;
			if (frames_arr.size() == 0)
			{
				allFound = false;
				break;
			}
			auto& frame = frames_arr.front();
			if (firstIter)
			{
				firstIter = false;
				fIndex = frame->fIndex;
			}
		}
		if (!allFound)
		{
			// LOG_ERROR << "Not Found All the 3 Frames Sorry Will Return from here";
			return false;
		}
		int count1 = 0;
		for (auto it = mQueue.begin(); it != mQueue.end(); it++)
		{
			count1++;
			// LOG_ERROR << "Dropping Frame";
			// LOG_ERROR << "	"
			frames[MultimediaQueueStrategy::getMultimediaQueuePinId(it->first)] = it->second.front();
			it->second.pop_front();
		}
		// LOG_ERROR << "Total Number Of Frames Found is <<<<<<" << count1 <<">>>>>>>>>>>>>";
		return true;
	}
	void clear()
	{
		for (auto it = mQueue.begin(); it != mQueue.end(); it++)
		{
			it->second.clear();
		}
		mQueue.clear();
	}
private:
	typedef std::map<std::string, boost_deque<frame_sp>> MultimediaQueue; // pinId and frame queue
	MultimediaQueue mQueue;
	double maxQueueLength;
	int maxDelay;
};
// Methods
MultimediaQueue::MultimediaQueue(MultimediaQueueProps _props) :Module(TRANSFORM, "MultimediaQueue", _props)
{
	mDetail.reset(new MultimediaQueueStrategy(_props));
}
bool MultimediaQueue::validateInputPins()
{
	return true;
}
bool MultimediaQueue::validateOutputPins()
{
	return true;
}
bool MultimediaQueue::validateInputOutputPins()
{
	return Module::validateInputOutputPins();
}
bool MultimediaQueue::init()
{
	if (!Module::init())
	{
		return false;
	}
	return true;
}
bool MultimediaQueue::term()
{
	mDetail.reset();
	return Module::term();
}
void MultimediaQueue::addInputPin(framemetadata_sp& metadata, string& pinId)
{
	Module::addInputPin(metadata, pinId);
	auto outputPinId = mDetail->addInputPin(pinId);
	addOutputPin(metadata, outputPinId);
}
bool MultimediaQueue::process(frame_container& frames)
{
	mDetail->queue(frames);
	frame_container outFrames;
	while (mDetail->get(outFrames))
	{
		send(outFrames);
		outFrames.clear();
	}
	// LOG_ERROR << "Sending frames from multimedia queue";
	return true;
}