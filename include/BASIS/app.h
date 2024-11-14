#pragma once
#include <BASIS/BASIS.h>

#include <cstdint>
#include <string_view>

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/quaternion.hpp>

#include <GLFW/glfw3.h>

namespace BASIS
{

struct Camera
{
	float sens{0.0025f};
	float speed{9.f};
	glm::vec3 pos{};
	
	glm::mat4 view() const;
	glm::vec3 forward() const;

	float pitch{};
	float yaw{};   
};

enum AppFlags : std::uint8_t
{
	SRGB = 1 << 0,
	DEBUG = 1 << 1,
	DOUBLEBUFFER = 1 << 2,
	UNDECORATED = 1 << 3,
	TRANSPARENT = 1 << 4,
	UNRESIZABLE = 1 << 5,
	NO_VSYNC = 1 << 6
};
struct AppCreateInfo
{
	std::string_view name{};
	std::uint32_t width{};
	std::uint32_t height{};
	std::uint8_t flags{};
};
struct App
{
	App(const App&) = delete;
	App(App&&) = delete;
	App& operator=(const App&) = delete;
	App& operator=(App&&) = delete;
	
	explicit App(const AppCreateInfo& info);
	static std::string loadFile(std::string_view p);
	
	void run();
	
	virtual ~App();
	protected:
	virtual void render([[maybe_unused]]double delta) {}
	virtual void gui([[maybe_unused]]double delta) {}
	virtual void updateCamera([[maybe_unused]]double delta);
	GLFWwindow* m_win{};
	bool active{true};
	std::uint32_t m_width{};
	std::uint32_t m_height{};
	Camera  m_camera{};
	glm::dvec2 m_cursorOffs{};
	glm::dvec2 m_lastCursorPos{};
};

};
