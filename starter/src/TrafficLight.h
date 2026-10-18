#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <atomic>
#include "TrafficObject.h"

enum class TrafficLightPhase
{
    red,
    green
};

template <class T>
class MessageQueue
{
public:
    T receive();
    void send(T &&msg);

private:
    std::mutex _mutex;
    std::condition_variable _condition;
    std::deque<T> _queue;
};

class TrafficLight : public TrafficObject
{
public:
    TrafficLight();
    ~TrafficLight();

    void waitForGreen();
    TrafficLightPhase getCurrentPhase();
    void simulate();

private:
    void cycleThroughPhases();

    TrafficLightPhase _currentPhase;
    MessageQueue<TrafficLightPhase> _queue;
    std::mutex _phaseMutex;
    std::atomic<bool> _stopRequested{false};
};

#endif