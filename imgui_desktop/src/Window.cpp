#include "Window.h"
#include "GLContext.h"
#include "ImGuiDesktopInternal.h"
#include "Application.h"
#include "ScopeGuards.h"

#ifdef IMGUI_USE_GLBINDING
#include <glbinding/glbinding.h>
#include <glbinding/gl/extension.h>
#include <glbinding/gl33core/gl.h>
#include <glbinding/gl43core/gl.h>
#include <glbinding/gl20ext/gl.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/types_to_string.h>
using namespace gl33core;
#elif IMGUI_USE_GLAD2
#include <glad/gl.h>
#else
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl/GL.h>
#endif

#include <imgui.h>
#include <mh/math/interpolation.hpp>
#include <mh/text/format.hpp>

#ifdef IMGUI_USE_SDL2
#include <imgui_impl_sdl.h>
#include <SDL.h>
#endif

#ifdef IMGUI_USE_OPENGL3
#include <imgui_impl_opengl3.h>
#endif
#ifdef IMGUI_USE_OPENGL2
#include <imgui_impl_opengl2.h>
#endif

#include <iomanip>
#include <sstream>
#include <stdexcept>

using namespace ImGuiDesktop;
using namespace std::string_literals;

static std::function<void(const std::string_view&, const mh::source_location&)> s_LogFunc;
void ImGuiDesktop::SetLogFunction(std::function<void(const std::string_view&, const mh::source_location&)> func)
{
	s_LogFunc = func;
}

void ImGuiDesktop::PrintLogMsg(const std::string_view& msg, const mh::source_location& location)
{
	if (s_LogFunc)
		s_LogFunc(msg, location);
}
void ImGuiDesktop::PrintLogMsg(const char* msg, const mh::source_location& location)
{
	PrintLogMsg(std::string_view(msg), location);
}

#ifdef IMGUI_USE_GLBINDING
#endif

static void ValidateDriver()
{
	const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
	const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	const char* driverVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	PrintLogMsg("GL_VENDOR =   "s << (vendor ? vendor : "<nullptr>"));
	PrintLogMsg("GL_RENDERER = "s << (renderer ? renderer : "<nullptr>"));
	PrintLogMsg("GL_VERSION =  "s << (driverVersion ? driverVersion : "<nullptr>"));

	if (!_stricmp("Intel", vendor))
	{
		static constexpr const char* BAD_DRIVER_VERSIONS[] =
		{
			"Build 27.20.100.8336",
			"Build 27.20.100.8280",
		};

		for (auto& badVersion : BAD_DRIVER_VERSIONS)
		{
			if (!strstr(driverVersion, badVersion))
				continue;

			auto errMsg = "The Intel driver version "s << std::quoted(driverVersion)
				<< " has severe bugs that make it incompatible with this program."
				"\n\nYou must upgrade to 27.20.100.8425 or downgrade to 27.20.100.8190 before using this tool.";

			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unsupported driver version",
				errMsg.c_str(), nullptr);

			std::exit(1);
		}
	}
}

auto Window::EnterGLScope() const
{
	return GLContextScope(m_WindowImpl.get(), m_GLContext);
}

Window::Window(Application& app, uint32_t width, uint32_t height, const char* title) :
	m_Application(&app)
{
	SDL_Init(SDL_INIT_VIDEO);

	SetupBasicWindowAttributes();

	m_WindowImpl.reset(SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI));
	if (!m_WindowImpl)
		throw std::runtime_error("Failed to create SDL window");

	m_GLContext = static_cast<IApplicationWindowInterface&>(app).GetOrCreateGLContext(m_WindowImpl.get());

	auto glScope = EnterGLScope();

	if (SDL_GL_SetSwapInterval(1))
		SDL_PRINT_AND_CLEAR_ERROR();

#ifdef IMGUI_USE_GLBINDING
	glbinding::initialize([](const char* fn) { return reinterpret_cast<glbinding::ProcAddress>(SDL_GL_GetProcAddress(fn)); });
#endif

	ValidateDriver();

	const bool isFirstContext = !ImGui::GetCurrentContext();
	m_ImGuiContext.reset(ImGui::CreateContext(&app.GetFontAtlas()));

	if (isFirstContext)
	{
		assert(ImGui::GetCurrentContext() == m_ImGuiContext.get());
		ImGui::SetCurrentContext(nullptr); // So ScopeGuards::Context sets it back to nullptr after we leave this scope
	}

	ScopeGuards::Context imGuiContextScope(m_ImGuiContext.get());

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::GetIO().IniFilename = nullptr; // Don't save stuff... for now

#ifdef IMGUI_USE_OPENGL3
	if (GetGLContextVersion().m_Major >= 3)
	{
		if (!ImGui_ImplOpenGL3_Init())
			throw std::runtime_error("Failed to initialize ImGui OpenGL3 impl");
	}
	else
#endif
	{
		if (!ImGui_ImplOpenGL2_Init())
			throw std::runtime_error("Failed to initialize ImGui OpenGL2 impl");
	}

	if (!ImGui_ImplSDL2_InitForOpenGL(m_WindowImpl.get(), m_GLContext.get()))
		throw std::runtime_error("Failed to initialize ImGui GLFW impl");

	static_cast<IApplicationWindowInterface&>(GetApplication()).AddWindow(this);
}

Window::~Window()
{
	static_cast<IApplicationWindowInterface&>(GetApplication()).RemoveWindow(this);
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
	GetApplication().QueueUpdate(this);
}

void Window::ShowWindow()
{
	SDL_ShowWindow(m_WindowImpl.get());
}

void Window::HideWindow()
{
	SDL_HideWindow(m_WindowImpl.get());
}

void Window::RaiseWindow()
{
	SDL_RaiseWindow(m_WindowImpl.get());
}

bool Window::IsVisible() const
{
	return SDL_GetWindowFlags(m_WindowImpl.get()) & SDL_WINDOW_SHOWN;
}

bool Window::HasFocus() const
{
	return SDL_GetWindowFlags(m_WindowImpl.get()) & (SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS);
}

void Window::OnUpdateInternal()
{
	auto scope = EnterGLScope();
	OnUpdate();
}

void Window::OnDrawInternal()
{
	// Update FPS
	{
		using hrc = std::chrono::high_resolution_clock;
		const auto now = hrc::now();
		if (m_LastUpdate != hrc::time_point{})
		{
			const auto delta = now - m_LastUpdate;
			const auto deltaSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();

			if (deltaSeconds <= 0 || deltaSeconds >= 1)
				m_FPS = 1;
			else
				m_FPS = mh::lerp(deltaSeconds, m_FPS, (1 / deltaSeconds));
		}

		m_LastUpdate = now;
	}

	auto scope = EnterGLScope();

#ifdef IMGUI_USE_GLBINDING
	glbinding::useCurrentContext();
#endif

	glClearStencil(0);
	glClearDepth(1.0f);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	assert(!ImGui::GetCurrentContext());
	ScopeGuards::Context imGuiContextScope(m_ImGuiContext.get());

	if (!m_IsInit)
	{
		OnOpenGLInit();
		OnImGuiInit();
		m_IsInit = true;
	}

	OnPreDraw();

#ifdef IMGUI_USE_OPENGL3
	if (GetGLContextVersion().m_Major >= 3)
		ImGui_ImplOpenGL3_NewFrame();
	else
#endif
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

		ImGuiStyle& style = ImGui::GetStyle();
		const auto backupWindowBg = style.Colors[ImGuiCol_WindowBg];
		const auto backupWindowBorderSize = style.WindowBorderSize;
		const auto backupWindowRounding = style.WindowRounding;

		ImGui::PushStyleColor(ImGuiCol_WindowBg, { backupWindowBg.x, backupWindowBg.y, backupWindowBg.z, 1 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

		if (ImGui::Begin("MainWindow", nullptr, windowFlags))
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, backupWindowBg);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, backupWindowBorderSize);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, backupWindowRounding);

			if (hasMenuBar)
			{
				if (ImGui::BeginMenuBar())
				{
					OnDrawMenuBar();
					ImGui::EndMenuBar();
				}
			}

			OnDraw();

			ImGui::PopStyleColor(1);
			ImGui::PopStyleVar(2);
		}
		ImGui::End();

		ImGui::PopStyleColor(1);
		ImGui::PopStyleVar(2);
	}

	ImGui::Render();

#ifdef IMGUI_USE_OPENGL3
	if (GetGLContextVersion().m_Major >= 3)
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	else
#endif
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(m_WindowImpl.get());
	OnEndFrame();
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

void Window::OnCloseButtonClicked()
{
	SetShouldClose(true);
}

void Window::SetIsPrimaryAppWindow(bool isPrimaryAppWindow)
{
	m_IsPrimaryAppWindow = isPrimaryAppWindow;
}
