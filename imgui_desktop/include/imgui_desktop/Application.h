#pragma once

#include <memory>
#include <vector>

struct ImFontAtlas;

namespace ImGuiDesktop
{
	class Window;

	class IApplicationWindowInterface
	{
	public:
		virtual ~IApplicationWindowInterface() = default;

	private:
		friend class Window;
		virtual void AddWindow(Window* window) = 0;
		virtual void RemoveWindow(Window* window) = 0;
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

	private:
		void AddWindow(Window* window) override final;
		void RemoveWindow(Window* window) override final;

		bool m_ShouldQuit = false;
		std::vector<Window*> m_Windows;
		std::vector<std::unique_ptr<Window>> m_ManagedWindows;

		std::unique_ptr<ImFontAtlas> m_SharedFontAtlas;
	};
}
