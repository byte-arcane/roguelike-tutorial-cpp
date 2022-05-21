#include "utility.h"

#include <cstdint>
#include <iostream>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <cmath>

#include <fmt/format.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include "framework.h"
#include "input.h"

namespace fs = std::filesystem;
using namespace glm;

namespace rlf
{
    std::string ReadTextFile(const std::string& path)
    {
        std::stringstream buffer;
        std::ifstream ifsdb(path);
        if (ifsdb.is_open())
            buffer << ifsdb.rdbuf();
        else
            std::cerr << "Error opening file for reading: " << path << std::endl;
        return buffer.str();
    }

    void WriteTextFile(const std::string& path, const std::string& text)
    {
        std::ofstream myfile(path);
        if (myfile.is_open())
            myfile << text;
        else
            std::cerr << "Error opening file for writing: " << path << std::endl;
    }

    std::string MediaSearch(const std::string& filename)
    {
        static const std::string media_path_prefixes[] = {
            "",                                                       // for absolute paths
            "media/",                                                 // for relative paths to the media folder in the working directory (e.g. exe file)
            fs::path(__FILE__).parent_path().string() + "/../media/"  // for relative paths to the media folder in the repository
        };

        for (const auto& media_path_prefix : media_path_prefixes)
        {
            std::string fullpath = media_path_prefix + filename;
            std::cout << "Combining " << media_path_prefix << " and " << filename << " yields " << fullpath << ".\n";
            fs::path path = fullpath;
            if (fs::exists(path))
            {
                fmt::print("mediaSearch: found media file {0}\n", path.string());
                return path.string();
            }
        }
        // could not find the path, return empty string
        fmt::print("mediaSearch: ERROR could not find media file {0}\n", filename);
        return {};
    }

    GLuint BuildShader(const char* vsource, const char* fsource)
    {
        GLuint shaderProgram = 0;
        // vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vsource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fsource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // link shaders
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return shaderProgram;
    }

    GLuint BuildVBO(const float* vertexData, int size)
    {
        GLuint vbo;
        glCreateBuffers(1, &vbo);
        glNamedBufferStorage(vbo, size, vertexData, GL_DYNAMIC_STORAGE_BIT);
        return vbo;
    }

    GLuint BuildVAO(GLuint vbo, int vertexSize)
    {
        GLuint vao;
        
        glCreateVertexArrays(1, &vao);
        glVertexArrayVertexBuffer(vao, 0, vbo, 0, vertexSize);
        glEnableVertexArrayAttrib(vao, 0);
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(vao, 0, 0);

        return vao;
    }

    GLuint LoadTexture(const std::string& filename, bool generateMipmaps, ivec2& textureSize)
    {
        textureSize = ivec2(0, 0);
        GLuint tex = 0;
        int width, height, n;
        unsigned char *pixel_data = stbi_load(filename.c_str(), &width, &height, &n, 0);
        if (pixel_data != nullptr)
        {
            textureSize = ivec2(width, height);
            glCreateTextures(GL_TEXTURE_2D, 1, &tex);

            GLenum format = 0;
            GLenum internalFormat = 0;
            switch (n)
            {
            case 1:
                format = GL_RED;
                internalFormat = GL_R8;
                break;
            case 3:
                format = GL_RGB;
                internalFormat = GL_RGB8;
                break;
            case 4:
                format = GL_RGBA;
                internalFormat = GL_RGBA8;
                break;
            default:
                break;
            }

            int numLevels = generateMipmaps ? int(ceil(log2(double(max(width,height))))) : 1;

            glTextureStorage2D(tex, numLevels, internalFormat, width, height);
            glTextureSubImage2D(tex, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, pixel_data);
            stbi_image_free(pixel_data);
            if (generateMipmaps)
                glGenerateTextureMipmap(tex);
        }
        return tex;
    }

    GLuint CreateTexture(const glm::ivec2& textureSize, GLenum format, GLenum internalFormat)
    {
        const int numLevels = 1;
        GLuint tex = 0;
        glCreateTextures(GL_TEXTURE_2D, 1, &tex);
        glTextureStorage2D(tex, numLevels, internalFormat, textureSize.x, textureSize.y);
        return tex;
    }

    void DeleteTexture(uint32_t& texture)
    {
        if (texture != 0)
        {
            texture = 0;
            glDeleteTextures(1, &texture);
        }
    }

    GLuint CreateBuffer(size_t numBytes, const void* data, GLenum usage)
    {
        GLuint ssbo;
        glCreateBuffers(1, &ssbo);
        glNamedBufferData(ssbo, numBytes, data, usage);
        return ssbo;
    }

    void DeleteBuffer(uint32_t& buffer)
    {
        if (buffer != 0)
        {
            buffer = 0;
            glDeleteBuffers(1, &buffer);
        }
    }
   
    void UpdateSSBO(GLuint ssbo, size_t offset, size_t size, const void* data)
    {
        // https://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/
        GLvoid* p = glMapNamedBufferRange(ssbo, offset, size, GL_MAP_WRITE_BIT);
        memcpy(p, data, size);
        glUnmapNamedBuffer(ssbo);
    }

    uint32_t PackColor(const vec4& color)
    {
        auto c = uvec4(clamp(color*255.0f, vec4(0), vec4(255)));
        return c.x | (c.y << 8) | (c.z << 16) | (c.w << 24);
    }

    vec4 UnpackColor(const uint32_t color)
    {
        vec4 c = { color & 255, (color >> 8) & 255, (color >> 16) & 255, (color >> 24) & 255 };
        return c / 255.0f;
    }
}