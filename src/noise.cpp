#include <GL/gl3w.h>
#include "GLUtils.h"
#include <stdlib.h>

GLuint createNoiseTexture4f3D(int w, int h, int d, GLint internalFormat)
{
    uint8_t *data = new uint8_t [w*h*d*4];
    uint8_t *ptr = data;
    for(int z=0; z<d; z++) {
        for(int y=0; y<h; y++) {
            for(int x=0; x<w; x++) {
              *ptr++ = rand() & 0xff;
              *ptr++ = rand() & 0xff;
              *ptr++ = rand() & 0xff;
              *ptr++ = rand() & 0xff;
            }
        }
    }

    GLuint tex;
    glGenTextures(1, &tex);
    CHECK_GL_ERROR();
    glBindTexture(GL_TEXTURE_3D, tex);
    CHECK_GL_ERROR();

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    CHECK_GL_ERROR();

    glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, w, h, d, 0, GL_RGBA, GL_BYTE, data);
    CHECK_GL_ERROR();

    delete [] data;
    return tex;
}
