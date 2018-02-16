#pragma once

#include "Common.h"

class Timer
{
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_counter;

    double m_deltaTime;
    double m_globalTime;
    double m_accumulator;

 public:
    Timer( );
    ~Timer( ) = default;

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
        return static_cast< float >( m_deltaTime );
    }
    float GetGlobalTime( ) const
    {
        return static_cast< float >( m_globalTime );
    }
};
