#include "Timer.hpp"

bool Timer::m_timersPaused = false;

Timer::Timer( )
    : m_deltaTime( 0.0 )
    , m_globalTime( 0.0 )
{
    QueryPerformanceFrequency( &m_frequency );
    QueryPerformanceCounter( &m_counter );
}

void Timer::ToggleTimer( )
{
    LARGE_INTEGER currentCount;
    QueryPerformanceCounter( &currentCount );

    __int64 deltaCount = currentCount.QuadPart - m_counter.QuadPart;

    m_deltaTime = deltaCount / static_cast< double >( m_frequency.QuadPart );

    m_globalTime += m_deltaTime;
    m_counter = currentCount;
}
