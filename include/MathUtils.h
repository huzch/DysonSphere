#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <cmath>
#include <cstring>

struct vec3f {
    float x, y, z;
    
    vec3f() : x(0), y(0), z(0) {}
    vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    vec3f operator+(const vec3f& v) const { return vec3f(x + v.x, y + v.y, z + v.z); }
    vec3f operator-(const vec3f& v) const { return vec3f(x - v.x, y - v.y, z - v.z); }
    vec3f operator*(float s) const { return vec3f(x * s, y * s, z * s); }
    vec3f& operator+=(const vec3f& v) { x += v.x; y += v.y; z += v.z; return *this; }
    vec3f& operator-=(const vec3f& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    
    float length() const { return sqrtf(x*x + y*y + z*z); }
    float lengthSq() const { return x*x + y*y + z*z; }
    vec3f normalize() const { float len = length(); return len > 0 ? vec3f(x/len, y/len, z/len) : vec3f(0,0,0); }
};

struct vec4f {
    float x, y, z, w;
    
    vec4f() : x(0), y(0), z(0), w(0) {}
    vec4f(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    vec4f(const vec3f& v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
    
    vec3f xyz() const { return vec3f(x, y, z); }
};

struct matrix4f {
    float m[16];
    
    matrix4f() {
        memset(m, 0, sizeof(m));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }
    
    matrix4f(float* data) {
        memcpy(m, data, sizeof(m));
    }
    
    float& operator[](int i) { return m[i]; }
    const float& operator[](int i) const { return m[i]; }
    
    matrix4f operator*(const matrix4f& other) const {
        matrix4f result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i*4 + j] = 0;
                for (int k = 0; k < 4; k++) {
                    result.m[i*4 + j] += m[i*4 + k] * other.m[k*4 + j];
                }
            }
        }
        return result;
    }
    
    vec4f operator*(const vec4f& v) const {
        return vec4f(
            m[0]*v.x + m[4]*v.y + m[8]*v.z + m[12]*v.w,
            m[1]*v.x + m[5]*v.y + m[9]*v.z + m[13]*v.w,
            m[2]*v.x + m[6]*v.y + m[10]*v.z + m[14]*v.w,
            m[3]*v.x + m[7]*v.y + m[11]*v.z + m[15]*v.w
        );
    }
};

inline void perspective(matrix4f& m, float fovy, float aspect, float zNear, float zFar) {
    float f = 1.0f / tanf(fovy / 2.0f);
    memset(m.m, 0, sizeof(m.m));
    m.m[0] = f / aspect;
    m.m[5] = f;
    m.m[10] = (zFar + zNear) / (zNear - zFar);
    m.m[11] = -1.0f;
    m.m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
}

inline void lookAt(matrix4f& m, const vec3f& eye, const vec3f& center, const vec3f& up) {
    vec3f f = (center - eye).normalize();
    vec3f s = vec3f(f.y * up.z - f.z * up.y, f.z * up.x - f.x * up.z, f.x * up.y - f.y * up.x).normalize();
    vec3f u = vec3f(s.y * f.z - s.z * f.y, s.z * f.x - s.x * f.z, s.x * f.y - s.y * f.x);
    
    m.m[0] = s.x; m.m[4] = s.y; m.m[8] = s.z; m.m[12] = 0;
    m.m[1] = u.x; m.m[5] = u.y; m.m[9] = u.z; m.m[13] = 0;
    m.m[2] = -f.x; m.m[6] = -f.y; m.m[10] = -f.z; m.m[14] = 0;
    m.m[3] = 0; m.m[7] = 0; m.m[11] = 0; m.m[15] = 1;
    
    matrix4f trans;
    trans.m[12] = -eye.x;
    trans.m[13] = -eye.y;
    trans.m[14] = -eye.z;
    m = m * trans;
}

inline void translate(matrix4f& m, const vec3f& v) {
    matrix4f trans;
    trans.m[12] = v.x;
    trans.m[13] = v.y;
    trans.m[14] = v.z;
    m = m * trans;
}

namespace nv {
    typedef vec3f vec3f;
    typedef vec4f vec4f;
    typedef matrix4f matrix4f;
}

#endif // MATH_UTILS_H

