#include "Application.h"
#include "Window.h"

#include <mh/error/ensure.hpp>

#ifdef IMGUI_USE_SDL2
#include <backends/imgui_impl_sdl.h>
#include <SDL.h>
#endif

using namespace ImGuiDesktop;

static constexpr const char SDL_WINDOW_PTR[] = __FILE__ " - {7825F11D-0FA9-4607-8B6F-977798B1C24C}";

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
}

Application::Application() :
	m_SharedFontAtlas(std::make_unique<ImFontAtlas>())
{
	m_SharedFontAtlas->AddFontDefault();
}

Application::~Application() = default;

void Application::Update()
{
	bool skipWait = false; //m_IsUpdateQueued || !IsSleepingEnabled();// || HasFocus();

	for (Window* wnd : m_Windows)
	{
		IWindowApplicationInterface* interface = wnd;
		if (interface->IsUpdateQueued() || !interface->IsSleepingEnabled())
		{
			skipWait = true;
			interface->ClearUpdateQueued();
		}
	}

	constexpr int SLEEP_DURATION = 100; // FIXME
	if (skipWait || SDL_WaitEventTimeout(nullptr, SLEEP_DURATION))
	{
		//m_IsUpdateQueued = false;

		bool shouldQueueWakeup = false;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			shouldQueueWakeup = true;

			if (!ImGui_ImplSDL2_ProcessEvent(&event))
			{
				switch (event.type)
				{
				case SDL_QUIT:
					m_ShouldQuit = true;
					for (Window* wnd : m_Windows)
						wnd->SetShouldClose(true);

					break;

				case SDL_USEREVENT:
				{
					switch ((CustomWindowEventCodes)event.user.code)
					{
					case CustomWindowEventCodes::Wakeup:
						shouldQueueWakeup = false;
						break;
					}

					break;
				}
				case SDL_WINDOWEVENT:
				{
					switch (event.window.event)
					{
					case SDL_WINDOWEVENT_CLOSE:
					{
						if (SDL_Window* window = SDL_GetWindowFromID(event.window.windowID))
						{
							if (auto managedWindow = mh_ensure(reinterpret_cast<Window*>(SDL_GetWindowData(window, SDL_WINDOW_PTR))))
							{
								static_cast<IWindowApplicationInterface*>(managedWindow)->OnCloseButtonClicked();
							}
						}

						break;
					}
					}
					break;
				}
				}
			}
		}

		// Imgui has a lot of "measure, then update next frame" sort of stuff.
		// Make sure we have an "extra" update before every sleep to account for that.
		if (shouldQueueWakeup)
			QueueUpdate(nullptr);
	}

	// Cannot be a range based for loop, stuff might get removed/added during the updates
	for (size_t i = 0; i < m_Windows.size(); i++)
		static_cast<IWindowApplicationInterface*>(m_Windows[i])->Update();

	std::erase_if(m_ManagedWindows, [](const auto& wnd) { return wnd->ShouldClose(); });
}

void Application::QueueUpdate(Window* window)
{
	SDL_Event event{};
	event.type = GetCustomWindowEventType();
	event.user.code = (int)CustomWindowEventCodes::Wakeup;
	event.user.windowID = window ? SDL_GetWindowID(window->GetSDLWindow()) : 0;
	SDL_PushEvent(&event);
}

bool Application::ShouldQuit() const
{
	for (Window* wnd : m_Windows)
	{
		if (!wnd->ShouldClose() && wnd->IsPrimaryAppWindow())
			return false;
	}

	return true;
	//return m_ShouldQuit;
}

void Application::AddManagedWindow(std::unique_ptr<Window> window)
{
	m_ManagedWindows.push_back(std::move(window));
}

void Application::AddWindow(Window* window)
{
	SDL_SetWindowData(window->GetSDLWindow(), SDL_WINDOW_PTR, window);
	m_Windows.push_back(window);
}

void Application::RemoveWindow(Window* window)
{
	mh_ensure(std::erase(m_Windows, window));
}
