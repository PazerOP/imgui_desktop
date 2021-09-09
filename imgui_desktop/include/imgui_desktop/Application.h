#pragma once

#include <memory>
#include <vector>

struct ImFontAtlas;
struct SDL_Window;

namespace ImGuiDesktop
{
	class GLContext;
	class Window;

	class IApplicationWindowInterface
	{
	public:
		virtual ~IApplicationWindowInterface() = default;

	private:
		friend class Window;
		virtual void AddWindow(Window* window) = 0;
		virtual void RemoveWindow(Window* window) = 0;

		virtual std::shared_ptr<GLContext> GetOrCreateGLContext(SDL_Window* window) = 0;
	};

	class Application : public IApplicationWindowInterface
	{
	public:
		Application();
		~Application();

		void Update();
		void QueueUpdate(Window* window);

		bool ShouldQuit() const;

		ImFontAtlas& GetFontAtlas() const { return *m_SharedFontAtlas.get(); }

		void AddManagedWindow(std::unique_ptr<Window> window);

	protected:
		virtual void OnAddingManagedWindow(Window& window) {}
		virtual void OnRemovingManagedWindow(Window& window) {}

		virtual void OnOpenGLInit() {}
		virtual void OnEndFrame() {}

	private:
		void AddWindow(Window* window) override final;
		void RemoveWindow(Window* window) override final;

		std::shared_ptr<GLContext> GetOrCreateGLContext(SDL_Window* window) override final;

		std::shared_ptr<GLContext> m_GLContext; // TODO: Do we actually ever want to release this (without exiting)

		bool m_ShouldQuit = false;
		std::vector<Window*> m_Windows;
		std::vector<std::unique_ptr<Window>> m_ManagedWindows;

		std::unique_ptr<ImFontAtlas> m_SharedFontAtlas;
	};
}
