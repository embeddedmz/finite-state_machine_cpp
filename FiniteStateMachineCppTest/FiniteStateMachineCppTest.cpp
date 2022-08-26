#include "CppUnitTest.h"
#include "FiniteStateMachine.h" // SUT

#include <algorithm>
#include <iostream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace FiniteStateMachineCppTest
{
    template <typename ExceptionType>
    void ThrowsException(std::function<void()> code)
    {
        try
        {
            code();
            Assert::Fail();
        }
        catch (const ExceptionType& exception)
        {
        }
        catch (...)
        {
            Assert::Fail();
        }
    }

	TEST_CLASS(FiniteStateMachineCppTestWithFixture)
	{
		enum TrafficLightsState
		{
			Green,
			Yellow,
			Red
		};

		FiniteStateMachine<TrafficLightsState> _sut;

	public:
		// ctor/dtor are executed before/after each unit test (setUp)
		FiniteStateMachineCppTestWithFixture()
		{
		}

		~FiniteStateMachineCppTestWithFixture()
		{
		}

		// Executed once before all units tests
		TEST_CLASS_INITIALIZE(ClassInitialize)
		{
			srand(unsigned int(time(NULL)));
		}

		// Executed once after all units tests
		TEST_CLASS_CLEANUP(ClassCleanup)
		{
		}

		TEST_METHOD(CheckRegisterStateFailure)
		{
			_sut.RegisterState(TrafficLightsState::Green);
            ThrowsException<std::invalid_argument>([this]() { _sut.RegisterState(TrafficLightsState::Green); });
            Assert::IsTrue(_sut.registeredStates().count(TrafficLightsState::Green) > 0);
            Assert::AreEqual(_sut.registeredStates().size(), (size_t)1);
		}

		TEST_METHOD(CheckRegisterStateSuccess)
		{
            std::function<void()> onEnterGreen = []() { std::cout << "onEnter green\n"; };
            std::function<void()> onUpdateGreen = []() { std::cout << "onUpdate green\n"; };
            std::function<void()> onLeaveGreen = []() { std::cout << "onLeave green\n"; };

            std::function<void()> onEnterRed = []() { std::cout << "onEnter red\n"; };
            std::function<void()> onUpdateRed = []() { std::cout << "onUpdate red\n"; };
            std::function<void()> onLeaveRed = []() { std::cout << "onLeave red\n"; };

            _sut.RegisterState(TrafficLightsState::Green,
                onEnterGreen, onUpdateGreen, onLeaveGreen);

            _sut.RegisterState(TrafficLightsState::Red,
                onEnterRed, onUpdateRed, onLeaveRed);

            Assert::AreEqual(_sut.registeredStates().size(), (size_t)2);
            Assert::IsTrue(_sut.registeredStates().count(TrafficLightsState::Green) > 0);
            Assert::IsTrue(_sut.registeredStates().count(TrafficLightsState::Red) > 0);

            const auto& statesMap = _sut.registeredStates();
            auto greenState = statesMap.at(TrafficLightsState::Green);
            Assert::IsTrue(TrafficLightsState::Green == greenState->Value);
            
            //std::function objects unlike .NET Action objects cannot be compared to each other
            // In the unit test UpdateFsm we check if they are correctly assinged
            // or use the same technique used in the unit test CheckStartFSMSuccess

            auto redState = statesMap.at(TrafficLightsState::Red);
            Assert::IsTrue(TrafficLightsState::Red == redState->Value);
		}

		TEST_METHOD(CheckTransitionFailure)
		{
            ThrowsException<std::logic_error>([this]() {
                _sut.RegisterTransition(
                    TrafficLightsState::Red,
                    TrafficLightsState::Green,
                    []() { return false; });
                });

            _sut.RegisterState(TrafficLightsState::Green);

            ThrowsException<std::logic_error>([this]() {
                _sut.RegisterTransition(
                    TrafficLightsState::Red,
                    TrafficLightsState::Green,
                    []() { return false; });
                });

            _sut.RegisterState(TrafficLightsState::Red);

            ThrowsException<std::invalid_argument>([this]() {
                _sut.RegisterTransition(
                    TrafficLightsState::Red,
                    TrafficLightsState::Green,
                    std::function<bool()>());
                });

            _sut.RegisterTransition(TrafficLightsState::Red,
                    TrafficLightsState::Green, []() { return false; });
		}

		TEST_METHOD(CheckTransitionSuccess)
		{
			_sut.RegisterState(TrafficLightsState::Green);
            _sut.RegisterState(TrafficLightsState::Red);

            std::function<bool()> condGreenToRed = []() { return true; };
            _sut.RegisterTransition(TrafficLightsState::Green,
                TrafficLightsState::Red, condGreenToRed);

            std::function<bool()> condRedToGreen = []() { return false; };
            _sut.RegisterTransition(TrafficLightsState::Red,
                TrafficLightsState::Green, condRedToGreen);

            auto greenState = _sut.registeredStates().at(TrafficLightsState::Green);
            auto redState = _sut.registeredStates().at(TrafficLightsState::Red);

            Assert::AreEqual((size_t)1, greenState->Transitions.size());
            Assert::AreEqual((size_t)1, redState->Transitions.size());

            auto toRedTransIt = std::find_if(greenState->Transitions.begin(), greenState->Transitions.end(),
                [](const auto& t) { return t.TargetState->Value == TrafficLightsState::Red; });
            Assert::IsTrue(toRedTransIt != greenState->Transitions.end());
            // can't compare toRedTransIt->Condition to condGreenToRed
            Assert::IsTrue(TrafficLightsState::Red == toRedTransIt->TargetState->Value);

            auto toGreenTransIt = std::find_if(redState->Transitions.begin(), redState->Transitions.end(),
                [](const auto& t) { return t.TargetState->Value == TrafficLightsState::Green; });
            Assert::IsTrue(toGreenTransIt != redState->Transitions.end());
            // can't compare toGreenTransIt->Condition to condRedToGreen
            Assert::IsTrue(TrafficLightsState::Green == toGreenTransIt->TargetState->Value);
		}

        TEST_METHOD(CheckStartFSMFailure)
        {
            ThrowsException<std::logic_error>([this]() { _sut.Start(TrafficLightsState::Green); });

            _sut.RegisterState(TrafficLightsState::Green);
            _sut.Start(TrafficLightsState::Green);
            ThrowsException<std::logic_error>([this]() { _sut.Start(TrafficLightsState::Green); });
        }

        TEST_METHOD(CheckStartFSMSuccess)
        {
            Assert::IsFalse(_sut.isStarted());

            bool onEnterInvoked = false;
            _sut.RegisterState(TrafficLightsState::Green, [&onEnterInvoked]() { onEnterInvoked = true; });
            _sut.Start(TrafficLightsState::Green);

            Assert::IsTrue(_sut.isStarted());
            Assert::IsTrue(TrafficLightsState::Green == _sut.currentState());
            Assert::IsTrue(onEnterInvoked);
        }

        TEST_METHOD(CheckUpdateFSMFailure)
        {
            ThrowsException<std::logic_error>([this]() { _sut.Update(); });
        }

        TEST_METHOD(UpdateFsm)
        {
            bool onEnterGreen = false, onUpdateGreen = false, onLeaveGreen = false;
            bool onEnterYellow = false, onUpdateYellow = false, onLeaveYellow = false;
            bool onEnterRed = false, onUpdateRed = false, onLeaveRed = false;

            bool goToGreen = false, goToYellow = false, goToRed = false;

            _sut.RegisterState(TrafficLightsState::Green,
                [&]() { onEnterGreen = true; onUpdateGreen = false; onLeaveGreen = false; },
                [&]() { onEnterGreen = false; onUpdateGreen = true; onLeaveGreen = false; },
                [&]() { onEnterGreen = false; onUpdateGreen = false; onLeaveGreen = true; });

            _sut.RegisterState(TrafficLightsState::Yellow,
                [&]() { onEnterYellow = true; onUpdateYellow = false; onLeaveYellow = false; },
                [&]() { onEnterYellow = false; onUpdateYellow = true; onLeaveYellow = false; },
                [&]() { onEnterYellow = false; onUpdateYellow = false; onLeaveYellow = true; });

            _sut.RegisterState(TrafficLightsState::Red,
                [&]() { onEnterRed = true; onUpdateRed = false; onLeaveRed = false; },
                [&]() { onEnterRed = false; onUpdateRed = true; onLeaveRed = false; },
                [&]() { onEnterRed = false; onUpdateRed = false; onLeaveRed = true; });

            _sut.RegisterTransition(TrafficLightsState::Green, TrafficLightsState::Yellow, [&goToYellow]() { return goToYellow; });
            _sut.RegisterTransition(TrafficLightsState::Yellow, TrafficLightsState::Red, [&goToRed]() { return goToRed; });
            _sut.RegisterTransition(TrafficLightsState::Red, TrafficLightsState::Green, [&goToGreen]() { return goToGreen; });

            _sut.Start(TrafficLightsState::Green);
            Assert::IsTrue(TrafficLightsState::Green == _sut.currentState());
            Assert::IsTrue(onEnterGreen);
            Assert::IsFalse(onUpdateGreen);
            Assert::IsFalse(onLeaveGreen);

            Assert::IsFalse(_sut.Update());
            Assert::IsFalse(onEnterGreen);
            Assert::IsTrue(onUpdateGreen);
            Assert::IsFalse(onLeaveGreen);

            goToYellow = true;
            Assert::IsTrue(_sut.Update());
            goToYellow = false;
            Assert::IsTrue(TrafficLightsState::Yellow == _sut.currentState());
            Assert::IsFalse(onEnterGreen);
            Assert::IsFalse(onUpdateGreen);
            Assert::IsTrue(onLeaveGreen);
            Assert::IsTrue(onEnterYellow);
            Assert::IsFalse(onUpdateYellow);
            Assert::IsFalse(onLeaveYellow);

            Assert::IsFalse(_sut.Update());
            Assert::IsFalse(_sut.Update());
            Assert::IsFalse(_sut.Update());

            Assert::IsTrue(TrafficLightsState::Yellow == _sut.currentState());
            Assert::IsFalse(onEnterGreen);
            Assert::IsFalse(onUpdateGreen);
            Assert::IsTrue(onLeaveGreen);
            Assert::IsFalse(onEnterYellow);
            Assert::IsTrue(onUpdateYellow);
            Assert::IsFalse(onLeaveYellow);

            goToRed = true;
            Assert::IsTrue(_sut.Update());
            goToRed = false;
            Assert::IsTrue(TrafficLightsState::Red == _sut.currentState());
            Assert::IsFalse(onEnterGreen);
            Assert::IsFalse(onUpdateGreen);
            Assert::IsTrue(onLeaveGreen);
            Assert::IsFalse(onEnterYellow);
            Assert::IsFalse(onUpdateYellow);
            Assert::IsTrue(onLeaveYellow);
            Assert::IsTrue(onEnterRed);
            Assert::IsFalse(onUpdateRed);
            Assert::IsFalse(onLeaveRed);

            Assert::IsFalse(_sut.Update());
            Assert::IsFalse(onEnterRed);
            Assert::IsTrue(onUpdateRed);
            Assert::IsFalse(onLeaveRed);

            goToGreen = true;
            Assert::IsTrue(_sut.Update());
            goToGreen = false;
            Assert::IsTrue(onEnterGreen);
            Assert::IsFalse(onUpdateGreen);
            Assert::IsFalse(onLeaveGreen);

            Assert::IsFalse(_sut.Update());
            Assert::IsFalse(onEnterGreen);
            Assert::IsTrue(onUpdateGreen);
            Assert::IsFalse(onLeaveGreen);
        }
	};
}
