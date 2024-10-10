#pragma once

#include <BASIS/interfaces.h>

#include <chrono>

struct CPUTimer : public ITimer
{
	using highClock = std::chrono::high_resolution_clock;
	CPUTimer() noexcept;
	~CPUTimer() = default;

	CPUTimer(const CPUTimer&) = delete;
	CPUTimer& operator=(const CPUTimer&) = delete;

	CPUTimer(CPUTimer&&) = default;
	CPUTimer& operator=(CPUTimer&&) = default;

	void reset() noexcept override;
	std::uint64_t getTime() const noexcept override;

private:
	std::chrono::time_point<highClock> m_start{};
};
struct GPUTimer : public ITimer
{
	GPUTimer() noexcept;
	~GPUTimer() noexcept;

	GPUTimer(const GPUTimer&) = delete;
	GPUTimer& operator=(const GPUTimer&) = delete;

	GPUTimer(GPUTimer&& other) = default;
	GPUTimer& operator=(GPUTimer&& other) = default;

	void reset() noexcept override;
	std::uint64_t getTime() const noexcept override;
private:
	std::uint64_t m_start{};
	std::uint32_t m_query{};
};