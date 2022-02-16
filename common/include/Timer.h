#pragma once

#include <cstdint>

class Timer
{
public:
    volatile uint16_t counter;
    volatile uint16_t control;

    static constexpr uint32_t IRQ_ENABLE = 1<<6;
    static constexpr uint32_t START = 1<<7;

    enum Prescaler
    {
        e1 = 0,
        e64 = 1,
        e256 = 2,
        e1024 = 3
    };

    // Start the timer and clear the counter
    template<unsigned prescaler>
    void reset()
    {
        static_assert(prescaler < 4);
        // Forze Enable bit to 0 so that the reload value will be copied
        // immediately to the counter once we reset it.
        control = prescaler;
        counter = 0;
        // Copy reload to restart and re-enable the timer
        control = START | prescaler;
    }

    void pause()
    {
        control = control & (~START);
    }
    
    void resume()
    {
        control = control | START;
    }
};

Timer& Timer0()
{
    return *reinterpret_cast<Timer*>(0x04000100);
}

Timer& Timer1()
{
    return *reinterpret_cast<Timer*>(0x04000104);
}

Timer& Timer2()
{
    return *reinterpret_cast<Timer*>(0x04000108);
}

Timer& Timer3()
{
    return *reinterpret_cast<Timer*>(0x0400010C);
}