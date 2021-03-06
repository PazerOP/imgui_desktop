#pragma once

#include "GLContextVersion.h"

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>

struct SDL_Window;
struct ImGuiContext;
struct ImFontAtlas;

namespace ImGuiDesktop
{
	void SetLogFunction(std::function<void(const std::string_view&)> func);

	class GLContext;

	class Window
	{
	public:
		Window(uint32_t width, uint32_t height, const char* title = "ImGuiDesktopWindow");

		void GetWindowSize(uint32_t& w, uint32_t& h) const;

		bool ShouldClose() const { return m_ShouldClose; }
		void SetShouldClose(bool shouldClose = true) { m_ShouldClose = shouldClose; }

		bool HasFocus() const;

		void QueueUpdate();
		void Update();

		GLContextVersion GetGLContextVersion() const;

		float GetFPS() const { return m_FPS; }

	protected:
		virtual void OnUpdate() {}
		virtual void OnDraw() = 0;
		virtual void OnEndFrame() {}

		virtual bool HasMenuBar() const { return false; }
		virtual void OnDrawMenuBar() {}

		virtual bool IsSleepingEnabled() const { return true; }

		static ImFontAtlas& GetFontAtlas();

	private:
		void OnUpdateInternal();
		void OnDrawInternal();

		bool m_ShouldClose = false;
		bool m_IsUpdateQueued = false;
		float m_SleepDuration = 0.1f;
		float m_FPS = (1.0f / 60);
		std::chrono::high_resolution_clock::time_point m_LastUpdate{};

		auto EnterGLScope() const;

		struct CustomDeleters
		{
			void operator()(SDL_Window* window) const;
			void operator()(ImGuiContext* context) const;
		};

		std::unique_ptr<SDL_Window, CustomDeleters> m_WindowImpl;
		std::unique_ptr<ImGuiContext, CustomDeleters> m_ImGuiContext;
		std::shared_ptr<GLContext> m_GLContext;
	};
}
