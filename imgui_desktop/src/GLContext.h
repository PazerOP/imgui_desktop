#pragma once

#include <memory>
#include <mutex>

struct SDL_Window;

namespace ImGuiDesktop
{
	class GLContext
	{
	public:
		GLContext(const std::shared_ptr<void>& context);

	private:
		friend class GLContextScope;

		std::shared_ptr<void> m_InnerContext;
		std::recursive_mutex m_ActiveMutex;
		int m_RecursionDepth = 0;
	};

	std::shared_ptr<GLContext> GetOrCreateGLContext(SDL_Window* window);

	class GLContextScope
	{
	public:
		GLContextScope(SDL_Window* window, const std::shared_ptr<GLContext>& context);
		~GLContextScope();

	private:
		std::shared_ptr<GLContext> m_Context;
		std::lock_guard<std::recursive_mutex> m_ContextActiveLock;
	};
}
