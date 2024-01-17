#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock( _mutex);
    cond.wait(uLock, [this] { return !_queue.empty(); });
    T k = std::move(_queue.back());
    _queue.clear();
    return (k);
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> Lock(_mutex);
    _queue.emplace_back(std::move(msg));
    cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    _type = ObjectType::objectTrafficLight;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
       TrafficLightPhase j =  _mq.receive();
       if (j == TrafficLightPhase::green)
       {
        return;
       }
    }

}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}
 
float TrafficLight::random(float low, float high)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(low, high);
    return (dist(gen));
}
 
// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    loop_start_time = std::chrono::system_clock::now();
    float cycle_duration = random(4000.0, 6000.0); // in miliseconds
    
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        loop_end_time = std::chrono::system_clock::now();              

        if (std::chrono::duration_cast<std::chrono::milliseconds>(loop_end_time - loop_start_time ).count() > cycle_duration )
        {
            //reset start time and make new cycle time
            loop_start_time = std::chrono::system_clock::now();
            cycle_duration = random(4000.0, 6000.0);  // in miliseconds
            _currentPhase = (_currentPhase == green) ? red : green;
            TrafficLightPhase p = _currentPhase;
            _mq.send(std::move(p));
        }
    }   
}
