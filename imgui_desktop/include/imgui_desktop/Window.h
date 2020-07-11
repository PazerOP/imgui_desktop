#pragma once

#include "GLContextVersion.h"

#include <cstdint>
#include <memory>

struct SDL_Window;
struct ImGuiContext;
struct ImFontAtlas;

namespace ImGuiDesktop
{
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

	protected:
		virtual void OnUpdate() {}
		virtual void OnDraw() = 0;

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
