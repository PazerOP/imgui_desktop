#include "Window.h"
#include "GLContext.h"

#include <glbinding/glbinding.h>
#include <glbinding/gl/extension.h>
#include <glbinding/gl33core/gl.h>
#include <glbinding/gl43core/gl.h>
#include <glbinding/gl20ext/gl.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/types_to_string.h>
#include <imgui.h>
#include <examples/imgui_impl_sdl.h>
#include <examples/imgui_impl_opengl2.h>
#include <examples/imgui_impl_opengl3.h>
#include <mh/text/string_insertion.hpp>
#include <SDL.h>

#include <sstream>
#include <stdexcept>

using namespace ImGuiDesktop;
using namespace gl33core;
using namespace std::string_literals;

namespace
{
	static uint32_t GetCustomWindowEventType()
	{
		static uint32_t s_EventID = SDL_RegisterEvents(1);
		return s_EventID;
	}

	enum class CustomWindowEventCodes
	{
		Wakeup,
	};

	static ImFontAtlas& s_ImGuiFontAtlas = []() -> ImFontAtlas&
	{
		static ImFontAtlas atlas;

		atlas.AddFontDefault();

		return atlas;
	}();
}

static std::function<void(const std::string_view&)> s_LogFunc;
void ImGuiDesktop::SetLogFunction(std::function<void(const std::string_view&)> func)
{
	s_LogFunc = func;
}

static void PrintLogMsg(const std::string_view& msg)
{
	if (s_LogFunc)
		s_LogFunc(msg);
}
static void PrintLogMsg(const char* msg)
{
	PrintLogMsg(std::string_view(msg));
}

static void DebugCallbackFn(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	// These get sent even when using the ARB extension on nvidia drivers
	if (severity == gl20ext::GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	std::stringstream ss;

	ss << "OpenGL Error:"
		<< "\n\tSource   : " << source
		<< "\n\tType     : " << type
		<< "\n\tID       : " << id
		<< "\n\tSeverity : " << severity
		<< "\n\tMessage  : " << message;

	PrintLogMsg(ss.str());
}

Window::Window(uint32_t width, uint32_t height, const char* title)
{
	SDL_Init(SDL_INIT_VIDEO);

	m_WindowImpl.reset(SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI));
	if (!m_WindowImpl)
		throw std::runtime_error("Failed to create SDL window");

	m_GLContext = GetOrCreateGLContext(m_WindowImpl.get());

	GLContextScope glScope(m_WindowImpl.get(), m_GLContext);
	glbinding::initialize([](const char* fn) { return reinterpret_cast<glbinding::ProcAddress>(SDL_GL_GetProcAddress(fn)); });

	const auto extensions = glbinding::aux::ContextInfo::extensions();

	if (extensions.contains(gl::GLextension::GL_KHR_debug))
	{
		gl20ext::glDebugMessageCallback(DebugCallbackFn, nullptr);
		gl20ext::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, gl20ext::GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
		gl20ext::glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, gl20ext::GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		gl::glEnable(gl43core::GL_DEBUG_OUTPUT);
		PrintLogMsg("Installed GL_KHR_debug debug message callback.");
	}
	else if (extensions.contains(gl::GLextension::GL_ARB_debug_output))
	{
		gl20ext::glDebugMessageCallbackARB(&DebugCallbackFn, nullptr);
		gl20ext::glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, gl20ext::GL_DEBUG_SEVERITY_LOW_ARB, 0, nullptr, GL_FALSE);
		PrintLogMsg("Installed GL_ARB_debug_output debug message callback.");
	}
	else
	{
		PrintLogMsg("No OpenGL debug message callback supported (context version "s << GetGLContextVersion() << ')');
	}

	SDL_GL_SetSwapInterval(1);

	m_ImGuiContext.reset(ImGui::CreateContext(&s_ImGuiFontAtlas));
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::GetIO().IniFilename = nullptr; // Don't save stuff... for now

	if (GetGLContextVersion().m_Major >= 3)
	{
		if (!ImGui_ImplOpenGL3_Init())
			throw std::runtime_error("Failed to initialize ImGui OpenGL3 impl");
	}
	else
	{
		if (!ImGui_ImplOpenGL2_Init())
			throw std::runtime_error("Failed to initialize ImGui OpenGL2 impl");
	}

	if (!ImGui_ImplSDL2_InitForOpenGL(m_WindowImpl.get(), m_GLContext.get()))
		throw std::runtime_error("Failed to initialize ImGui GLFW impl");

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	style.WindowBorderSize = 0;
	style.WindowRounding = 0;
}

ImFontAtlas& Window::GetFontAtlas()
{
	return s_ImGuiFontAtlas;
}

auto Window::EnterGLScope() const
{
	return GLContextScope(m_WindowImpl.get(), m_GLContext);
}

void Window::GetWindowSize(uint32_t& w, uint32_t& h) const
{
	int wi, hi;
	SDL_GetWindowSize(m_WindowImpl.get(), &wi, &hi);

	assert(wi >= 0);
	assert(hi >= 0);

	w = static_cast<uint32_t>(wi);
	h = static_cast<uint32_t>(hi);
}

void Window::Update()
{
	OnUpdateInternal();
	OnDrawInternal();
}

void Window::QueueUpdate()
{
	m_IsUpdateQueued = true;

	SDL_Event event{};
	event.type = GetCustomWindowEventType();
	event.user.code = (int)CustomWindowEventCodes::Wakeup;
	event.user.windowID = SDL_GetWindowID(m_WindowImpl.get());
	SDL_PushEvent(&event);
}

bool Window::HasFocus() const
{
	return SDL_GetWindowFlags(m_WindowImpl.get()) & (SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS);
}

void Window::OnUpdateInternal()
{
	auto scope = EnterGLScope();

	const auto windowFlags = SDL_GetWindowFlags(m_WindowImpl.get());
	bool skipWait = m_IsUpdateQueued || !IsSleepingEnabled();// || HasFocus();

	if (skipWait || SDL_WaitEventTimeout(nullptr, int(m_SleepDuration * 1000)))
	{
		m_IsUpdateQueued = false;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			bool shouldQueueWakeup = true;

			if (!ImGui_ImplSDL2_ProcessEvent(&event))
			{
				switch (event.type)
				{
				case SDL_QUIT:
					SetShouldClose(true);
					break;

				case SDL_USEREVENT:
				{
					if (event.user.code == (int)CustomWindowEventCodes::Wakeup)
						shouldQueueWakeup = false;
				}
				}
			}

			// Imgui has a lot of "measure, then update next frame" sort of stuff.
			// Make sure we have an "extra" update before every sleep to account for that.
			if (shouldQueueWakeup)
				QueueUpdate();
		}
	}

	OnUpdate();
}

void Window::OnDrawInternal()
{
	auto scope = EnterGLScope();

	glbinding::useCurrentContext();
	ImGui::SetCurrentContext(m_ImGuiContext.get());

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (GetGLContextVersion().m_Major >= 3)
		ImGui_ImplOpenGL3_NewFrame();
	else
		ImGui_ImplOpenGL2_NewFrame();

	ImGui_ImplSDL2_NewFrame(m_WindowImpl.get());
	ImGui::NewFrame();
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

		uint32_t windowW, windowH;
		GetWindowSize(windowW, windowH);
		ImGui::SetNextWindowSize(ImVec2(float(windowW), float(windowH)), ImGuiCond_Always);

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration;
		const bool hasMenuBar = HasMenuBar();
		if (hasMenuBar)
			windowFlags |= ImGuiWindowFlags_MenuBar;

		if (ImGui::Begin("MainWindow", nullptr, windowFlags))
		{
			if (hasMenuBar)
			{
				if (ImGui::BeginMenuBar())
				{
					OnDrawMenuBar();
					ImGui::EndMenuBar();
				}
			}

			OnDraw();
		}
		ImGui::End();
	}

	ImGui::Render();

	if (GetGLContextVersion().m_Major >= 3)
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	else
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(m_WindowImpl.get());
}

void Window::CustomDeleters::operator()(SDL_Window* window) const
{
	SDL_DestroyWindow(window);
}

void Window::CustomDeleters::operator()(ImGuiContext* context) const
{
	ImGui::DestroyContext(context);
}

GLContextVersion Window::GetGLContextVersion() const
{
	return m_GLContext->GetVersion();
}
