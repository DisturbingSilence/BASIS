#include <glad/gl.h>

#include <BASIS/app.h>

#include <fstream>
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace BASIS
{
std::string App::loadFile(std::string_view path)
{
	if(std::filesystem::exists(path))
	{
		std::ifstream file{path.data()};
		return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
	}
	throw FileException("File not found:",path);
}
App::App(const AppCreateInfo& info)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SRGB_CAPABLE, info.flags & AppFlags::SRGB);
	glfwWindowHint(GLFW_DECORATED, !(info.flags & AppFlags::UNDECORATED));
	glfwWindowHint(GLFW_RESIZABLE, !(info.flags & AppFlags::UNRESIZABLE));
	glfwWindowHint(GLFW_DOUBLEBUFFER, info.flags & AppFlags::DOUBLEBUFFER);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT , info.flags & AppFlags::DEBUG);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, info.flags & AppFlags::TRANSPARENT);
	
	m_win = glfwCreateWindow(info.width,info.height,info.name.data(),0,0);
	
	if(!m_win)
	{
		const char* errorMsg{};
		glfwGetError(&errorMsg);
		glfwTerminate();
		throw ApplicationException("Window creation failure[",errorMsg,']');
	}
	glfwMakeContextCurrent(m_win);
	if (!gladLoadGL(glfwGetProcAddress))
	{
		throw ApplicationException("Glad initialization failure");
	}
	assert(info.width != 0 && info.height != 0 && "invalid width or height");
	m_width  = info.width;
	m_height = info.height;
	
	// default callbacks, can be adjusted by inheriting from App class later
	glfwSetWindowUserPointer(m_win,reinterpret_cast<void*>(this));
	
	glfwSetKeyCallback(m_win, [](GLFWwindow* window, int key,int, int action,int) -> void
	{
		App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
		if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS)
		{
			app->active = !app->active;
			glfwSetInputMode(window, GLFW_CURSOR, app->active ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		}
		if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) app->m_camera.speed = 18.f;
		if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) app->m_camera.speed = 9.f;
	});
	
	glfwSetCursorPosCallback(m_win, [](GLFWwindow* window, double X, double Y) -> void
	{
		App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

		app->m_cursorOffs += glm::dvec2{X - app->m_lastCursorPos.x, app->m_lastCursorPos.y - Y};
		app->m_lastCursorPos = {X, Y};
	});
	glfwSetFramebufferSizeCallback(m_win, [](GLFWwindow* window,int w,int h) -> void
	{
		App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
		app->m_width = w;
		app->m_height = h;
		glViewport(0, 0, w, h);
	});
	
	if(info.flags & AppFlags::DEBUG)
	{
	Renderer::enableCapability(Cap::DEBUG_OUTPUT);
	Renderer::enableCapability(Cap::DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	using ui = unsigned int;
	glDebugMessageCallback([](ui src, ui type, ui id, ui severity, int, const char* msg, const void*) -> void
	{
		if (id == 131169 || 
        id == 131185 || // NV: Buffer will use video memory
        id == 131218 || 
        id == 131204 || // Texture cannot be used for texture mapping
        id == 131222 ||
        id == 131154 || // NV: pixel transfer is synchronized with 3D rendering
        id == 0         // gl{Push, Pop}DebugGroup
      )
      return;
		auto srcOut = [=]() 
		{
			switch (src)
			{
				case GL_DEBUG_SOURCE_API: 			  return "API";
				case GL_DEBUG_SOURCE_OTHER: 		  return "OTHER";
				case GL_DEBUG_SOURCE_THIRD_PARTY: 	  return "THIRD PARTY";
				case GL_DEBUG_SOURCE_APPLICATION: 	  return "APPLICATION";
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WINDOW SYSTEM";
				case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
				
				default : return "UNKNOWN";
			}
		}();

		auto typeOut = [=]() 
		{
			switch (type)
			{
				case GL_DEBUG_TYPE_ERROR: 				return "ERROR";
				case GL_DEBUG_TYPE_OTHER: 				return "OTHER";
				case GL_DEBUG_TYPE_MARKER: 			 	return "MARKER";
				case GL_DEBUG_TYPE_PORTABILITY: 		return "PORTABILITY";
				case GL_DEBUG_TYPE_PERFORMANCE: 		return "PERFORMANCE";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEFINED_BEHAVIOR";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
				
				default : return "UNKNOWN";
			}
		}();

		auto severityOut = [=]() 
		{
			switch (severity) 
			{
				case GL_DEBUG_SEVERITY_LOW: 		 return "LOW";
				case GL_DEBUG_SEVERITY_HIGH: 		 return "HIGH";
				case GL_DEBUG_SEVERITY_MEDIUM: 		 return "MEDIUM";
				case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
				
				default : return "UNKNOWN";
			}
		}();
		printf("[%d | %s | %s | %s] %s\n",id,severityOut,typeOut,srcOut,msg);
	}, nullptr);
	}
	if(info.flags & AppFlags::NO_VSYNC)
	{
		glfwSwapInterval(0);
	}
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
    
	ImGui_ImplGlfw_InitForOpenGL(m_win, true);
	ImGui_ImplOpenGL3_Init();
}

void App::run()
{
	double delta{},lFrame{};
	while (!glfwWindowShouldClose(m_win))
	{	
		const auto curTime = glfwGetTime();
		delta  = curTime - lFrame;
		lFrame = curTime;
		
		updateCamera(delta);
		
		render(delta);
		ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		gui(delta);
		
		ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(m_win);
		glfwPollEvents();
	}
}
App::~App()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(m_win);
	glfwTerminate();
}

void App::updateCamera(double delta)
{
	glfwSetInputMode(m_win, GLFW_CURSOR, active ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	if (!active)
    {
		glfwSetCursorPos(m_win, 0, 0);
		m_lastCursorPos = {};
		
		const float deltaSpeed = m_camera.speed * delta;
		const glm::vec3 frd = m_camera.forward();
		const glm::vec3 right = glm::normalize(glm::cross(frd, {0, 1, 0}));
		
		if (glfwGetKey(m_win, GLFW_KEY_W) == GLFW_PRESS) m_camera.pos += frd * deltaSpeed;
		if (glfwGetKey(m_win, GLFW_KEY_S) == GLFW_PRESS) m_camera.pos -= frd * deltaSpeed;
		if (glfwGetKey(m_win, GLFW_KEY_D) == GLFW_PRESS) m_camera.pos += right * deltaSpeed;
		if (glfwGetKey(m_win, GLFW_KEY_A) == GLFW_PRESS) m_camera.pos -= right * deltaSpeed;

		if (glfwGetKey(m_win, GLFW_KEY_SPACE) 	      == GLFW_PRESS) m_camera.pos.y += deltaSpeed;
		if (glfwGetKey(m_win, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) m_camera.pos.y -= deltaSpeed;
		
		m_camera.m_yaw   += static_cast<float>(m_cursorOffs.x * m_camera.sens);
		m_camera.m_pitch += static_cast<float>(m_cursorOffs.y * m_camera.sens);
		m_camera.m_pitch  = glm::clamp(m_camera.m_pitch, -glm::half_pi<float>() + 1e-4f, glm::half_pi<float>() - 1e-4f);
    }
    m_cursorOffs = {};
}

glm::vec3 Camera::forward() const
{
	return glm::vec3{cos(m_pitch) * cos(m_yaw), sin(m_pitch), cos(m_pitch) * sin(m_yaw)};
}
glm::mat4 Camera::view() const
{
	return glm::lookAt(pos, pos + forward(), glm::vec3(0, 1, 0));
}

}
