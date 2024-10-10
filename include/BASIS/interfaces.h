// abstract classes and interfaces to inherit from
#pragma once

#include <cstdint>

// base class for opengl wrappers to inherit from
struct IGLObject
{
	IGLObject() = default;
	IGLObject(const IGLObject&) = delete;
	IGLObject& operator=(const IGLObject&) = delete;

	std::uint32_t id() const noexcept { return m_id; }
protected:
	std::uint32_t m_id{};
};
struct ITimer
{
	// resets timer
	virtual void reset() noexcept = 0;

	virtual std::uint64_t getTime() const noexcept = 0;

};

