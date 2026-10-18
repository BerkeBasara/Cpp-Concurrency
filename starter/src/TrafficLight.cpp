#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <utility>
#include "TrafficLight.h"

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> lock(_mutex);

    _condition.wait(lock, [this]() {
        return !_queue.empty();
    });

    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);

        _queue.push_back(std::move(msg));
    }

    _condition.notify_one();
}

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight()
{
    _stopRequested = true;
}

void TrafficLight::waitForGreen()
{
    while (true)
    {
        TrafficLightPhase phase = _queue.receive();

        if (phase == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    std::lock_guard<std::mutex> lock(_phaseMutex);

    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(
        std::thread(&TrafficLight::cycleThroughPhases, this)
    );
}

void TrafficLight::cycleThroughPhases()
{
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<int> distribution(4000, 6000);

    auto lastUpdate = std::chrono::steady_clock::now();
    int cycleDuration = distribution(engine);

    while (!_stopRequested)
    {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(1)
        );

        auto currentTime = std::chrono::steady_clock::now();

        long timeSinceLastUpdate =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - lastUpdate
            ).count();

        if (timeSinceLastUpdate >= cycleDuration)
        {
            TrafficLightPhase newPhase;

            {
                std::lock_guard<std::mutex> lock(_phaseMutex);

                if (_currentPhase == TrafficLightPhase::red)
                {
                    _currentPhase = TrafficLightPhase::green;
                }
                else
                {
                    _currentPhase = TrafficLightPhase::red;
                }

                newPhase = _currentPhase;
            }

            _queue.send(std::move(newPhase));

            lastUpdate = std::chrono::steady_clock::now();
            cycleDuration = distribution(engine);
        }
    }
}