#ifndef NOISE_H
#define NOISE_H

#include <GL/gl3w.h>

GLuint createNoiseTexture4f3D(int w, int h, int d, GLint internalFormat);

#endif // NOISE_H