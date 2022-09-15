#pragma once

#include "Module.h"

class MultimediaQueue;
class MultimediaQueueProps : public ModuleProps
{
public:
    MultimediaQueueProps()
    {
        // in msec or number of frames
        lowerWaterMark = 10000;
        upperWaterMark = 15000;
        isMapDelayInTime = true;
    }
    MultimediaQueueProps(uint32_t queueLength = 10000, uint16_t tolerance = 5000, bool _isDelayTime = true)
    {
        lowerWaterMark = queueLength;
        upperWaterMark = queueLength + tolerance;
        isMapDelayInTime = _isDelayTime;
    }

    uint32_t lowerWaterMark; // Length of multimedia queue in terms of time or number of frames
    uint32_t upperWaterMark; //Length of the multimedia queue when the next module queue is full
    bool isMapDelayInTime;
};

class State;

class MultimediaQueue : public Module {
public:
    MultimediaQueue(MultimediaQueueProps _props);

    virtual ~MultimediaQueue() {
    }

    // variable names - start with small letters
    bool init();
    bool term();
    void setState(uint64_t ts, uint64_t te);
    bool handleCommand(Command::CommandType type, frame_sp& frame);
    bool allowFrames(uint64_t& ts, uint64_t& te);
    // default behaviour is overridden
    bool setNext(boost::shared_ptr<Module> next, bool open = true, bool sieve = false);
    void setProps(MultimediaQueueProps _props);
    MultimediaQueueProps getProps();
    bool handlePropsChange(frame_sp& frame);
    boost::shared_ptr<State> mState;
    MultimediaQueueProps mProps;

protected:
    bool process(frame_container& frames);
    bool validateInputPins();
    bool validateOutputPins();
    bool validateInputOutputPins();

private:
    void getQueueBoundaryTS(uint64_t& tOld, uint64_t& tNew);

    bool pushNext = true;
    bool reset = false;
    uint64_t startTimeSaved = 0;
    uint64_t endTimeSaved = 0;
    uint64_t queryStartTime = 0;
    uint64_t queryEndTime = 0;
};

class QueueClass;

class State {
public:
    boost::shared_ptr<QueueClass> queueObject;
    State() {}
    //State(MultimediaQueueProps& _props) {}
    virtual ~State() {}
    typedef std::map<uint64_t, frame_container> mQueueMap;
    // check this once
    virtual bool handleExport(uint64_t& queryStart, uint64_t& queryEnd, bool& timeReset, mQueueMap& mQueue) { return true; };

    enum StateType
    {
        IDLE = 0,
        WAITING,
        EXPORT
    };

    State(StateType type_)
    {
        Type = type_;
    }
    StateType Type = StateType::IDLE;
};