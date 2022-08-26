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

    auto condYellow = [&]() { return yellow; };
    auto condRed = [&]() { return red; };
    auto condGreen = [&]() { return green; };

    trafficLights.RegisterTransition(TrafficLightsState::Green, TrafficLightsState::Yellow, condYellow);
    trafficLights.RegisterTransition(TrafficLightsState::Yellow, TrafficLightsState::Red, condRed);
    trafficLights.RegisterTransition(TrafficLightsState::Red, TrafficLightsState::Green, condGreen);

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
