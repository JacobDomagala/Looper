#pragma once

#include "Common.hpp"

class Timer
{
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_counter;

    double m_deltaTime;
    double m_globalTime;
    double m_accumulator;

    static bool m_timersPaused;

 public:
    Timer( );
    ~Timer( ) = default;

    static void PauseAllTimers( )
    {
        m_timersPaused = true;
    }

    static void ResumeAllTimers( )
    {
        m_timersPaused = false;
    }

    static bool AreTimersRunning( )
    {
        return !m_timersPaused;
    }

    void ToggleTimer( );

    void Accumulate( )
    {
        m_accumulator += m_deltaTime;
    }
    void ToggleAndAccumulate( )
    {
        ToggleTimer( );
        m_accumulator += m_deltaTime;
    }
    void ResetAccumulator( )
    {
        m_accumulator = 0;
    }
    float GetAccumulator( ) const
    {
        return static_cast< float >( m_accumulator );
    }
    float GetDeltaTime( ) const
    {
        return static_cast< float >( m_deltaTime * !m_timersPaused );
    }
    float GetGlobalTime( ) const
    {
        return static_cast< float >( m_globalTime );
    }
};
