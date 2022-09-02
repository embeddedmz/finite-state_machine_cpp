#include <chrono>
#include <iostream>
#include <thread>

#include "FiniteStateMachine.h"

using namespace std::chrono_literals;

int main()
{
    enum TrafficLightsState
    {
        Green,
        Yellow,
        Red
    };

    FiniteStateMachine<TrafficLightsState> trafficLights;

    trafficLights.RegisterState(TrafficLightsState::Green,
        []() { std::cout << "Green ON" << std::endl; std::this_thread::sleep_for(3000ms); },
        []() { std::cout << "Green OFF" << std::endl; });

    trafficLights.RegisterState(TrafficLightsState::Yellow,
        []() { std::cout << "Yellow ON" << std::endl; std::this_thread::sleep_for(1000ms); },
        []() { std::cout << "Yellow OFF" << std::endl; });

    trafficLights.RegisterState(TrafficLightsState::Red,
        []() { std::cout << "Red ON" << std::endl; std::this_thread::sleep_for(2000ms); },
        []() { std::cout << "Red OFF" << std::endl; });

    bool green = false;
    bool yellow = false;
    bool red = false;

    trafficLights.RegisterTransition(TrafficLightsState::Green, TrafficLightsState::Yellow, [&]() { return yellow; });
    trafficLights.RegisterTransition(TrafficLightsState::Yellow, TrafficLightsState::Red, [&]() { return red; });
    trafficLights.RegisterTransition(TrafficLightsState::Red, TrafficLightsState::Green, [&]() { return green; });

    trafficLights.Start(TrafficLightsState::Green);

    yellow = true;
    trafficLights.Update();
    yellow = false;

    red = true;
    trafficLights.Update();
    red = false;

    green = true;
    trafficLights.Update();

    return 0;
}
