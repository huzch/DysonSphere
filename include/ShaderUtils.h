#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <GL/gl3w.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class ShaderProgram {
public:
    GLuint program;
    
    ShaderProgram() : program(0) {}
    
    ~ShaderProgram() {
        if (program) {
            glDeleteProgram(program);
        }
    }
    
    bool loadFromFiles(const char* vertexPath, const char* fragmentPath) {
        std::string vertexCode = readFile(vertexPath);
        std::string fragmentCode = readFile(fragmentPath);
        
        if (vertexCode.empty() || fragmentCode.empty()) {
            return false;
        }
        
        return createProgram(vertexCode.c_str(), fragmentCode.c_str());
    }
    
    bool loadFromStrings(const char* vertexSource, const char* fragmentSource) {
        return createProgram(vertexSource, fragmentSource);
    }
    
    void enable() {
        glUseProgram(program);
    }
    
    void disable() {
        glUseProgram(0);
    }
    
    GLint getUniformLocation(const char* name) {
        return glGetUniformLocation(program, name);
    }
    
private:
    std::string readFile(const char* filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filepath << std::endl;
            return "";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    GLuint compileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader compilation error: " << infoLog << std::endl;
            glDeleteShader(shader);
            return 0;
        }
        
        return shader;
    }
    
    bool createProgram(const char* vertexSource, const char* fragmentSource) {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        if (!vertexShader) return false;
        
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (!fragmentShader) {
            glDeleteShader(vertexShader);
            return false;
        }
        
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Program linking error: " << infoLog << std::endl;
            glDeleteProgram(program);
            program = 0;
        }
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        return success == GL_TRUE;
    }
};

std::string loadShaderSourceWithUniformTag(const char* uniformsFile, const char* srcFile);

#endif // SHADER_UTILS_H

