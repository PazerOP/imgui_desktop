#pragma once

#include "GLContextVersion.h"

#include <memory>
#include <mutex>
#include <string_view>

struct SDL_Window;

namespace ImGuiDesktop
{
	class GLContext
	{
	public:
		GLContext(const std::shared_ptr<void>& context, GLContextVersion version);

		GLContextVersion GetVersion() const { return m_GLVersion; }

	private:
		friend class GLContextScope;

		std::shared_ptr<void> m_InnerContext;
		std::recursive_mutex m_ActiveMutex;
		int m_RecursionDepth = 0;

		GLContextVersion m_GLVersion{};
	};

	std::shared_ptr<GLContext> GetOrCreateGLContext(SDL_Window* window);
	void SetupBasicWindowAttributes();

	class GLContextScope
	{
	public:
		GLContextScope(SDL_Window* window, const std::shared_ptr<GLContext>& context);
		~GLContextScope();

		GLContextVersion GetVersion() const { return m_Context->GetVersion(); }
		bool HasExtension(const std::string_view& extensionName) const;

		void* GetProcAddress(const char* symbolName) const;
		template<typename T> T GetProcAddress(const char* symbolName) const { return static_cast<T>(GetProcAddress(symbolName)); }

	private:
		std::shared_ptr<GLContext> m_Context;
		std::lock_guard<std::recursive_mutex> m_ContextActiveLock;
	};
}
