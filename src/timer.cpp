#include <BASIS/timer.h>

#include <utility>

#include <glad/gl.h>
CPUTimer::CPUTimer() noexcept
{
	m_start = highClock::now();
}
std::uint64_t CPUTimer::getTime() const noexcept
{
	return std::chrono::duration<double, std::nano>(highClock::now() - m_start).count();
}
void CPUTimer::reset() noexcept
{
	m_start = highClock::now();
}
GPUTimer::GPUTimer() noexcept
{
	glGenQueries(1,&m_query);
	reset();
}
GPUTimer::~GPUTimer() noexcept
{
	glDeleteQueries(1,&m_query);
}
void GPUTimer::reset() noexcept
{
	glQueryCounter(m_query,GL_TIMESTAMP);
	// rethink this part maybe
	int complete{};
	while (!complete) glGetQueryObjectiv(m_query, GL_QUERY_RESULT_AVAILABLE, &complete);
	glGetQueryObjectui64v(m_query, GL_QUERY_RESULT, &m_start);
}
std::uint64_t GPUTimer::getTime() const noexcept
{
	int complete{};
    glQueryCounter(m_query, GL_TIMESTAMP);
    while (!complete) glGetQueryObjectiv(m_query, GL_QUERY_RESULT_AVAILABLE, &complete);
    std::uint64_t endTime;
    glGetQueryObjectui64v(m_query, GL_QUERY_RESULT, &endTime);
    return endTime - m_start;
}
