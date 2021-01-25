#pragma once

#include "GLContextVersion.h"

#include <mh/source_location.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>

struct SDL_Window;
struct ImGuiContext;
struct ImFontAtlas;

namespace ImGuiDesktop
{
	void SetLogFunction(std::function<void(const std::string_view&, const mh::source_location&)> func);

	class Application;
	class GLContext;

	class IWindowApplicationInterface
	{
	public:
		virtual ~IWindowApplicationInterface() = default;

	private:
		friend class Application;
		virtual bool IsUpdateQueued() const = 0;
		virtual void ClearUpdateQueued() = 0;
		virtual bool IsSleepingEnabled() const = 0;
		virtual void Update() = 0;
		virtual void OnCloseButtonClicked() = 0;
	};

	class Window : public IWindowApplicationInterface
	{
	public:
		Window(Application& app, uint32_t width, uint32_t height, const char* title = "ImGuiDesktopWindow");
		virtual ~Window();

		SDL_Window* GetSDLWindow() const { return m_WindowImpl.get(); }

		void GetWindowSize(uint32_t& w, uint32_t& h) const;

		bool ShouldClose() const { return m_ShouldClose; }
		void SetShouldClose(bool shouldClose = true) { m_ShouldClose = shouldClose; }

		bool HasFocus() const;

		void QueueUpdate();

		void ShowWindow();
		void HideWindow();
		void RaiseWindow();
		bool IsVisible() const;

		GLContextVersion GetGLContextVersion() const;

		float GetFPS() const { return m_FPS; }

		Application& GetApplication() const { return *m_Application; }

		bool IsPrimaryAppWindow() const { return m_IsPrimaryAppWindow; }

	protected:
		virtual void OnImGuiInit() {}
		virtual void OnUpdate() {}
		virtual void OnPreDraw() {}
		virtual void OnDraw() = 0;
		virtual void OnEndFrame() {}
		void OnCloseButtonClicked() override;

		virtual bool HasMenuBar() const { return false; }
		virtual void OnDrawMenuBar() {}

		void SetIsPrimaryAppWindow(bool isPrimaryAppWindow);

		bool IsSleepingEnabled() const override { return true; }

	private:
		void OnUpdateInternal();
		void OnDrawInternal();
		void Update() override final;

		bool IsUpdateQueued() const override final { return m_IsUpdateQueued; }
		void ClearUpdateQueued() override final { m_IsUpdateQueued = false; }

		bool m_IsPrimaryAppWindow = false;
		bool m_IsImGuiInit = false;
		bool m_ShouldClose = false;
		bool m_IsUpdateQueued = false;
		float m_SleepDuration = 0.1f;
		float m_FPS = (1.0f / 60);
		std::chrono::high_resolution_clock::time_point m_LastUpdate{};

		auto EnterGLScope() const;

		struct CustomDeleters
		{
			void operator()(SDL_Window* window) const;
			void operator()(ImGuiContext* ctx) const;
		};

		Application* m_Application{};
		std::unique_ptr<SDL_Window, CustomDeleters> m_WindowImpl;
		std::unique_ptr<ImGuiContext, CustomDeleters> m_ImGuiContext;
		std::shared_ptr<GLContext> m_GLContext;
	};
}
