#include "Window.h"
#include "GLContext.h"

#include <glbinding/glbinding.h>
#include <glbinding/gl33core/gl.h>
#include <imgui.h>
#include <examples/imgui_impl_sdl.h>
#include <examples/imgui_impl_opengl3.h>
#include <SDL.h>

#include <stdexcept>

using namespace ImGuiDesktop;
using namespace gl33core;

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

	SDL_GL_SetSwapInterval(1);

	m_ImGuiContext.reset(ImGui::CreateContext(&s_ImGuiFontAtlas));
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	if (!ImGui_ImplOpenGL3_Init("#version 330 core"))
		throw std::runtime_error("Failed to initialize ImGui OpenGL3 impl");
	if (!ImGui_ImplSDL2_InitForOpenGL(m_WindowImpl.get(), m_GLContext.get()))
		throw std::runtime_error("Failed to initialize ImGui GLFW impl");

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	style.WindowBorderSize = 0;
	style.WindowRounding = 0;
}

auto Window::EnterGLScope()
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

void Window::OnUpdateInternal()
{
	auto scope = EnterGLScope();

	if (m_IsUpdateQueued || SDL_WaitEventTimeout(nullptr, 1000))
	{
		m_IsUpdateQueued = false;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				SetShouldClose(true);
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

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_WindowImpl.get());
	ImGui::NewFrame();
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

		uint32_t windowW, windowH;
		GetWindowSize(windowW, windowH);
		ImGui::SetNextWindowSize(ImVec2(float(windowW), float(windowH)), ImGuiCond_Always);
		if (ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration))
		{
			OnDraw();
		}
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
