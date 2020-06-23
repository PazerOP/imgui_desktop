#include "GLContext.h"

#include <SDL.h>

#include <cassert>

using namespace ImGuiDesktop;

namespace
{
	struct SDL_GLContextDeleter
	{
		void operator()(void* context) const
		{
			SDL_GL_DeleteContext(context);
		}
	};

	struct GLContextHolder
	{
		std::shared_ptr<GLContext> GetOrCreateGLContext(SDL_Window* window)
		{
			auto context = m_GLContext.lock();
			if (!context)
			{
				std::lock_guard lock(m_Mutex);

				context = m_GLContext.lock();
				if (!context)
				{
					SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
					SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
					SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
					SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
					context = std::make_shared<GLContext>(std::shared_ptr<void>(SDL_GL_CreateContext(window), SDL_GLContextDeleter{}));
					m_GLContext = context;
				}
			}

			return context;
		}

	private:
		std::mutex m_Mutex;
		std::weak_ptr<GLContext> m_GLContext;

	} static s_GLContextHolder;
}

GLContextScope::GLContextScope(SDL_Window* window, const std::shared_ptr<GLContext>& context) :
	m_Context(context),
	m_ContextActiveLock(m_Context->m_ActiveMutex)
{
	assert(m_Context->m_RecursionDepth >= 0);
	if (m_Context->m_RecursionDepth++ <= 0)
	{
		SDL_GL_MakeCurrent(window, context->m_InnerContext.get());
	}
}

GLContextScope::~GLContextScope()
{
	m_Context->m_RecursionDepth--;
	assert(m_Context->m_RecursionDepth >= 0);
}

std::shared_ptr<GLContext> ImGuiDesktop::GetOrCreateGLContext(SDL_Window* window)
{
	return s_GLContextHolder.GetOrCreateGLContext(window);
}

GLContext::GLContext(const std::shared_ptr<void>& context) :
	m_InnerContext(context)
{
}
