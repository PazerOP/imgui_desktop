#include "GLContext.h"

#include <SDL.h>

#include <cassert>
#include <sstream>

using namespace ImGuiDesktop;

#define SDL_TRY_SET_ATTR(func) \
	if (auto result = func; result != 0) \
		assert(!"Failed to run " #func);

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
					SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8));
					SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8));
					SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8));
					SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0));
					SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0));
					SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0));

					constexpr GLContextVersion VERSION_4(4, 3);
					constexpr GLContextVersion VERSION_3(3, 2);
					constexpr GLContextVersion VERSION_2(2, 0);

					SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG));

					// Try OpenGL 4
					{
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, VERSION_4.m_Major));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, VERSION_4.m_Minor));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
						context = std::make_shared<GLContext>(
							std::shared_ptr<void>(SDL_GL_CreateContext(window), SDL_GLContextDeleter{}),
							VERSION_4);
					}

					if (!context)
					{
						// Try OpenGL 3
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, VERSION_3.m_Major));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, VERSION_3.m_Minor));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
						context = std::make_shared<GLContext>(
							std::shared_ptr<void>(SDL_GL_CreateContext(window), SDL_GLContextDeleter{}),
							VERSION_3);
					}

					if (!context)
					{
						// Try OpenGL 2
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, VERSION_2.m_Major));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, VERSION_2.m_Minor));
						context = std::make_shared<GLContext>(
							std::shared_ptr<void>(SDL_GL_CreateContext(window), SDL_GLContextDeleter{}),
							VERSION_2);
					}

					if (!context)
					{
						// Neither worked, show an error and quit
						std::stringstream ss;
						ss << "Failed to initialize OpenGL " << VERSION_4
							<< ", OpenGL " << VERSION_3
							<< ", or OpenGL " << VERSION_2
							<< ". Unfortunately, this means your computer is too old to run this software.";

						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenGL Initialization Failed",
							ss.str().c_str(), window);

						std::exit(1);
					}

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

GLContext::GLContext(const std::shared_ptr<void>& context, GLContextVersion version) :
	m_InnerContext(context), m_GLVersion(version)
{
}
