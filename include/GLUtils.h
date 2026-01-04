#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <GL/gl3w.h>
#include <iostream>

#define CHECK_GL_ERROR() \
    do { \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            std::cerr << "OpenGL error at " << __FILE__ << ":" << __LINE__ << ": " << err << std::endl; \
        } \
    } while(0)

#endif // GL_UTILS_H

