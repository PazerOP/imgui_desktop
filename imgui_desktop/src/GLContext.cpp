#include "GLContext.h"
#include "ImGuiDesktopInternal.h"

#include <mh/algorithm/algorithm.hpp>
#include <mh/error/ensure.hpp>
#include <mh/text/format.hpp>
#include <SDL.h>

#ifdef IMGUI_USE_GLBINDING
	#error fixme
#else
	#ifdef WIN32
		#include <Windows.h>
	#endif

	#include <gl/GL.h>

	#ifdef _MSC_VER
		#define GL_APIENTRY APIENTRY
	#else
		#define GL_APIENTRY
	#endif
#endif

#include <cassert>
#include <sstream>

using GLchar = char;
static constexpr GLenum GL_DEBUG_SEVERITY_NOTIFICATION = 0x826B;
static constexpr GLenum GL_DEBUG_SEVERITY_LOW = 0x9148;
static constexpr GLenum GL_DEBUG_SEVERITY_LOW_ARB = 0x9148;
static constexpr GLenum GL_DEBUG_SEVERITY_MEDIUM = 0x9147;
static constexpr GLenum GL_DEBUG_SEVERITY_HIGH = 0x9146;
static constexpr GLenum GL_DEBUG_OUTPUT = 0x92E0;
static constexpr GLenum GL_DEBUG_OUTPUT_SYNCHRONOUS = 0x8242;
static constexpr GLenum GL_NUM_EXTENSIONS = 0x821D;

using GLDEBUGPROC = void (GL_APIENTRY*)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam);

using glGetStringi_t = const GLubyte* (GL_APIENTRY*)(GLenum name, GLuint index);

using glDebugMessageCallback_t = void(GL_APIENTRY*)(GLDEBUGPROC callback, const void* userParam);
using glDebugMessageControl_t = void (GL_APIENTRY*)(GLenum source, GLenum type, GLenum severity, GLsizei count,
	const GLuint* ids, GLboolean enabled);

using glDebugMessageCallbackARB_t = glDebugMessageCallback_t;
using glDebugMessageControlARB_t = glDebugMessageControl_t;

using namespace ImGuiDesktop;
using namespace std::string_literals;

#define SDL_TRY_SET_ATTR(func) \
	if (auto result = func; result != 0) \
	{ \
		SDL_PRINT_AND_CLEAR_ERROR(); \
		assert(!"Failed to run " #func); \
	}

#define LOOKUP_GL_SYMBOL(ctx, name) \
	const auto name = mh_ensure((ctx).GetProcAddress<name ## _t>(#name))

void ImGuiDesktop::SetupBasicWindowAttributes()
{
	SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8));
	SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8));
	SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8));
	SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0));
	SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0));
	SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)); // Needed for vsync

#ifdef _DEBUG
	SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG));
#endif
}

static void GL_APIENTRY DebugCallbackFn(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	// These get sent even when using the ARB extension on nvidia drivers
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
		return;

	std::stringstream ss;

	ss << "OpenGL Error:"
		<< "\n\tSource   : " << source
		<< "\n\tType     : " << type
		<< "\n\tID       : " << id
		<< "\n\tSeverity : " << severity
		<< "\n\tMessage  : " << message;

	PrintLogMsg(ss.str());
}

static void InstallDebugCallback(const GLContextScope& context)
{
	if (context.HasExtension("GL_KHR_debug"))
	{
		LOOKUP_GL_SYMBOL(context, glDebugMessageCallback);
		LOOKUP_GL_SYMBOL(context, glDebugMessageControl);
		glDebugMessageCallback(&DebugCallbackFn, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		PrintLogMsg("Installed GL_KHR_debug debug message callback.");
	}
	else if (context.HasExtension("GL_ARB_debug_output"))
	{
		LOOKUP_GL_SYMBOL(context, glDebugMessageCallbackARB);
		LOOKUP_GL_SYMBOL(context, glDebugMessageControlARB);
		glDebugMessageCallbackARB(&DebugCallbackFn, nullptr);
		glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW_ARB, 0, nullptr, GL_FALSE);
		PrintLogMsg("Installed GL_ARB_debug_output debug message callback.");
	}
	else
	{
		PrintLogMsg(mh::format("No OpenGL debug message callback supported (context version {})", context.GetVersion()));
	}
}

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
					constexpr GLContextVersion VERSION_4(4, 3);
					constexpr GLContextVersion VERSION_3(3, 2);
					constexpr GLContextVersion VERSION_2(2, 0);

#if IMGUI_USE_OPENGL3
					// Try OpenGL 4
					{
						PrintLogMsg(mh::format("Initializing OpenGL {}...", VERSION_4));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, VERSION_4.m_Major));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, VERSION_4.m_Minor));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
						context = std::make_shared<GLContext>(
							std::shared_ptr<void>(SDL_GL_CreateContext(window), SDL_GLContextDeleter{}),
							VERSION_4);

						if (!SDL_PRINT_AND_CLEAR_ERROR())
							context.reset();
					}

					if (!context)
					{
						// Try OpenGL 3
						PrintLogMsg(mh::format("Initializing OpenGL {}...", VERSION_3));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, VERSION_3.m_Major));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, VERSION_3.m_Minor));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
						context = std::make_shared<GLContext>(
							std::shared_ptr<void>(SDL_GL_CreateContext(window), SDL_GLContextDeleter{}),
							VERSION_3);

						if (!SDL_PRINT_AND_CLEAR_ERROR())
							context.reset();
					}
#endif

					if (!context)
					{
						// Try OpenGL 2
						PrintLogMsg(mh::format("Initializing OpenGL {}...", VERSION_2));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, VERSION_2.m_Major));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, VERSION_2.m_Minor));

						// These must be set back to zero for legacy context creation
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0));
						SDL_TRY_SET_ATTR(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0));

						context = std::make_shared<GLContext>(
							std::shared_ptr<void>(SDL_GL_CreateContext(window), SDL_GLContextDeleter{}),
							VERSION_2);

						if (!SDL_PRINT_AND_CLEAR_ERROR())
							context.reset();
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

					{
						GLContextScope scope(window, context);
						InstallDebugCallback(scope);
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
		if (auto err = SDL_GL_MakeCurrent(window, context->m_InnerContext.get()); err != 0)
		{
			if (!SDL_PRINT_AND_CLEAR_ERROR())
				assert(false);
		}
	}
}

GLContextScope::~GLContextScope()
{
	if (--m_Context->m_RecursionDepth == 0)
	{
		if (auto err = SDL_GL_MakeCurrent(nullptr, nullptr); err != 0)
		{
			if (!SDL_PRINT_AND_CLEAR_ERROR())
				assert(false);
		}
	}

	assert(m_Context->m_RecursionDepth >= 0);
}

void* GLContextScope::GetProcAddress(const char* symbolName) const
{
	return SDL_GL_GetProcAddress(symbolName);
}

bool GLContextScope::HasExtension(const std::string_view& extensionName) const
{
	LOOKUP_GL_SYMBOL(*this, glGetStringi);

	GLint extensionCount;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
	for (GLint i = 0; i < extensionCount; i++)
	{
		const auto ext = std::string_view(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
		if (extensionName == ext)
			return true;
	}

	return false;
}

std::shared_ptr<GLContext> ImGuiDesktop::GetOrCreateGLContext(SDL_Window* window)
{
	return s_GLContextHolder.GetOrCreateGLContext(window);
}

GLContext::GLContext(const std::shared_ptr<void>& context, GLContextVersion version) :
	m_InnerContext(context), m_GLVersion(version)
{
}
