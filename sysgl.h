#if !defined __APPLE__ || AWBACKEND != cocoa
#if defined _WIN32
#include <windows.h>
#elif defined __ANDROID__
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif
#elif defined __IPHONE_OS_VERSION_MIN_REQUIRED
#include <OpenGLES/ES2/gl.h>
#else
#include <OpenGL/gl.h>
#endif
