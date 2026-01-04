#include "ShaderUtils.h"
#include <fstream>
#include <sstream>
#include <cstring>

std::string loadShaderSourceWithUniformTag(const char* uniformsFile, const char* srcFile) {
    std::ifstream uniformsStream(uniformsFile);
    if (!uniformsStream.is_open()) {
        std::cerr << "Failed to open uniforms file: " << uniformsFile << std::endl;
        return "";
    }
    std::stringstream uniformsBuffer;
    uniformsBuffer << uniformsStream.rdbuf();
    std::string uniformsStr = uniformsBuffer.str();
    uniformsStream.close();
    
    std::ifstream srcStream(srcFile);
    if (!srcStream.is_open()) {
        std::cerr << "Failed to open source file: " << srcFile << std::endl;
        return "";
    }
    std::stringstream srcBuffer;
    srcBuffer << srcStream.rdbuf();
    std::string srcStr = srcBuffer.str();
    srcStream.close();
    
    const char* uniformTag = "#UNIFORMS";
    size_t tagPos = srcStr.find(uniformTag);
    
    if (tagPos != std::string::npos) {
        std::string result = srcStr.substr(0, tagPos);
        result += "\n";
        result += uniformsStr;
        result += "\n";
        result += srcStr.substr(tagPos + strlen(uniformTag));
        return result;
    }
    
    return srcStr;
}

