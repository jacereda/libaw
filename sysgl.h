#if !defined __APPLE__ || AWBACKEND != cocoa
#if defined _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#elif defined __IPHONE_OS_VERSION_MIN_REQUIRED
#include <OpenGLES/ES2/gl.h>
#else
#include <OpenGL/gl.h>
#endif
