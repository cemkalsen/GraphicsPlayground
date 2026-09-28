#ifndef GL_DEBUG_H
#define GL_DEBUG_H

#include <glad/glad.h>

GLenum glCheckError_(const char* file, int line);

#define glCheckError() glCheckError_(__FILE__, __LINE__)

void EnableGLDebugOutput();

#endif