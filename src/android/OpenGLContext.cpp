#include "OpenGLContext.h"
#include "MelonLog.h"

const char* OPENGL_CONTEXT_TAG = "OpenGLContext";

void logGlError()
{
    GLenum error = glGetError();
    switch(error)
    {
        case GL_NO_ERROR:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "GL_NO_ERROR");
            break;
        case GL_INVALID_ENUM:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "GL_INVALID_ENUM");
            break;
        case GL_INVALID_VALUE:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "GL_INVALID_VALUE");
            break;
        case GL_INVALID_OPERATION:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "GL_INVALID_OPERATION");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "GL_INVALID_FRAMEBUFFER_OPERATION");
            break;
        case GL_OUT_OF_MEMORY:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "GL_OUT_OF_MEMORY");
            break;
        case GL_STACK_UNDERFLOW:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "GL_STACK_UNDERFLOW");
            break;
        case GL_STACK_OVERFLOW:
            LOG_ERROR(OPENGL_CONTEXT_TAG, "GL_STACK_OVERFLOW");
            break;
    }
}

bool OpenGLContext::InitContext()
{
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to get display");
        return false;
    }

    int majorVersion, minorVersion;
    if (!eglInitialize(display, &majorVersion, &minorVersion))
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to initialize display");
        return false;
    }
    else
    {
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "Initialised display with version %d.%d", majorVersion, minorVersion);
    }

    EGLint attributes[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE,
    };

    EGLint numConfigs;
    if (!eglChooseConfig(display, attributes, nullptr, 0, &numConfigs))
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to determine the number of configs");
        logGlError();
        return false;
    }

    if (numConfigs <= 0)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "No configs found");
        return false;
    }
    else
    {
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "Found %d configs", numConfigs);
    }

    EGLConfig configs[numConfigs];
    EGLint selectedConfigNumber = -1;

    //eglChooseConfig(display, attributes, &selectedConfig, 1, &selectedConfigNumber);
    eglChooseConfig(display, attributes, configs, numConfigs, &numConfigs);

    for (int i = 0; i < numConfigs; i++)
    {
        int depth, stencil, red, green, blue;

        eglGetConfigAttrib(display, configs[i], EGL_DEPTH_SIZE, &depth);
        eglGetConfigAttrib(display, configs[i], EGL_STENCIL_SIZE, &stencil);
        eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &red);
        eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &green);
        eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE, &blue);

        if (depth < 1 || stencil < 0)
            continue;

        if (red == 8 && green == 8 && blue == 8)
        {
            selectedConfigNumber = i;
            break;
        }
    }

    if (selectedConfigNumber < 0)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Couldn't find matching configuration");
        return false;
    }
    else
    {
        int depth, stencil, red, green, blue;

        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_DEPTH_SIZE, &depth);
        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_STENCIL_SIZE, &stencil);
        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_RED_SIZE, &red);
        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_GREEN_SIZE, &green);
        eglGetConfigAttrib(display, configs[selectedConfigNumber], EGL_BLUE_SIZE, &blue);

        LOG_DEBUG(OPENGL_CONTEXT_TAG, "Selected GL contextAttributes (#%d):", selectedConfigNumber);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tRED: %d", red);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tGREEN: %d", green);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tBLUE: %d", blue);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tDEPTH: %d", depth);
        LOG_DEBUG(OPENGL_CONTEXT_TAG, "\tSTENCIL: %d", stencil);
    }

    EGLint surfaceAttributes[] = {
        EGL_WIDTH, 256,
        EGL_HEIGHT, 192 * 2 + 2,
        EGL_NONE
    };

    surface = eglCreatePbufferSurface(display, configs[selectedConfigNumber], surfaceAttributes);
    if (surface == EGL_NO_SURFACE)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to create buffer surface");
        logGlError();
        return false;
    }

    int contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    glContext = eglCreateContext(display, configs[selectedConfigNumber], nullptr, contextAttributes);
    if (glContext == EGL_NO_CONTEXT)
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to create context");
        logGlError();
        return false;
    }

    return true;
}

bool OpenGLContext::Use()
{
    if (eglMakeCurrent(display, surface, surface, glContext))
    {
        return true;
    }
    else
    {
        LOG_ERROR(OPENGL_CONTEXT_TAG, "Failed to use OpenGL context");
        glGetError();
        return false;
    }
}

void OpenGLContext::Release()
{
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void OpenGLContext::DeInit()
{
    if (display == EGL_NO_DISPLAY)
        return;

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (glContext != EGL_NO_CONTEXT)
        eglDestroyContext(display, glContext);

    if (surface != EGL_NO_SURFACE)
        eglDestroySurface(display, surface);

    eglTerminate(display);

    display = EGL_NO_DISPLAY;
    surface = EGL_NO_SURFACE;
    glContext = EGL_NO_CONTEXT;
}
