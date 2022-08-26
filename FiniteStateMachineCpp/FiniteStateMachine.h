#pragma once

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <unordered_map>

template <typename StateEnumType>
class FiniteStateMachine
{
    //static_assert(std::is_enum_v<StateEnumType>); // requires C++17

public:
    // nested classes are public so we can unit test them
    struct State;
    struct Transition
    {
        State* TargetState;
        std::function<bool()> Condition;

        Transition(State* targetState, std::function<bool()> condition)
        {
            TargetState = targetState;
            Condition = condition;
        }
    };

    struct State
    {
        StateEnumType Value;
        std::function<void()> OnEnter;
        std::function<void()> OnUpdate;
        std::function<void()> OnLeave;
        std::vector<Transition> Transitions;

        State(StateEnumType stateValue,
            std::function<void()> onEnter = std::function<void()>(),
            std::function<void()> onUpdate = std::function<void()>(),
            std::function<void()> onLeave = std::function<void()>())
        {
            Value = stateValue;
            OnEnter = onEnter;
            OnUpdate = onUpdate;
            OnLeave = onLeave;
        }
    };

private:
    State* _currentState = nullptr;
    std::unordered_map<StateEnumType, State*> _registeredStates;

public:
    ~FiniteStateMachine()
    {
        for (const auto& pair : _registeredStates)
        {
            delete pair.second;
        }
    }

    bool isStarted() const { return _currentState != nullptr; }
    StateEnumType currentState() const { return _currentState != nullptr ? _currentState->Value : StateEnumType(); }

    // we need to expose it for unit tests
    const std::unordered_map<StateEnumType, State*>& registeredStates() const { return _registeredStates; }

    void Start(StateEnumType initState)
    {
        if (_currentState == nullptr)
        {
            if (_registeredStates.count(initState) > 0)
            {
                _currentState = _registeredStates[initState];
                if (_currentState->OnEnter)
                {
                    _currentState->OnEnter();
                }
            }
            else
            {
                throw std::logic_error("Initial state is not registered !");
            }
        }
        else
        {
            throw std::logic_error("FSM has already been started !");
        }
    }

    void RegisterState(StateEnumType state,
        std::function<void()> onEnter = std::function<void()>(),
        std::function<void()> onUpdate = std::function<void()>(),
        std::function<void()> onLeave = std::function<void()>())
    {
        if (_registeredStates.count(state) > 0)
        {
            throw std::invalid_argument("The state is already registered !");
        }

        _registeredStates[state] = new State(state, onEnter, onUpdate, onLeave);
    }

    void RegisterTransition(StateEnumType fromState, StateEnumType toState, std::function<bool()> condition)
    {
        if (!condition)
        {
            throw std::invalid_argument("Condition doesn't contain a target !");
        }

        if (!_registeredStates.count(fromState))
        {
            throw std::logic_error("Source state is not registered !");
        }

        if (!_registeredStates.count(toState))
        {
            throw std::logic_error("Target state is not registered !");
        }

        _registeredStates[fromState]->Transitions.push_back(Transition(_registeredStates[toState], condition));
    }

    bool Update()
    {
        if (!isStarted())
        {
            throw std::logic_error("FSM is not started !");
        }

        if (_currentState->OnUpdate)
        {
            _currentState->OnUpdate();
        }

        auto validTrans = std::find_if(_currentState->Transitions.begin(), _currentState->Transitions.end(),
            [](const Transition& t) { return t.Condition(); });
        if (validTrans != _currentState->Transitions.end())
        {
            if (_currentState->OnLeave)
            {
                _currentState->OnLeave();
            }
            
            _currentState = validTrans->TargetState;
            
            if (_currentState->OnEnter)
            {
                _currentState->OnEnter();
            }

            return true;
        }

        return false;
    }
};
