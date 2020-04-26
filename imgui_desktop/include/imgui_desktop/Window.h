#pragma once

#include <cstdint>
#include <memory>

struct SDL_Window;
struct ImGuiContext;

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

		void QueueUpdate();
		void Update();

	protected:
		virtual void OnUpdate() {}
		virtual void OnDraw() = 0;

	private:
		void OnUpdateInternal();
		void OnDrawInternal();

		bool m_ShouldClose = false;
		bool m_IsUpdateQueued = false;

		auto EnterGLScope();

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
