#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace cgf
{
	std::string readTextFile(const std::string& path);
	void writeTextFile(const std::string& path, const std::string& text);

	// Searches a media filename (shader, texture, model, etc)
	std::string mediaSearch(const std::string& path);

	bool imguiTextboxAndButton(std::string& content, const std::string& label);

	glm::vec3 simpleArcballCamera(glm::mat4& viewMatrix, glm::mat4& projMatrix);

	// This takes as parameters the shader TEXT (not the filename)
	GLuint buildShader(const char* vsource, const char* fsource);
	
	// Helper to reload a shader based on a keypress, e.g. GLFW_KEY_L
	// This takes as parameters the filenames, to search in media folders
	void reloadShaderOnKey(GLuint& program, const std::string& vs, const std::string& fs, int key);

	// Build a VBO that contains floats in any arrangement (array of floats, array of vec2, array of vec3, etc)
	GLuint buildVBO(const float * vertexData, int size);
	GLuint buildVAO(GLuint vbo, int vertexSize);
	GLuint buildIBO(const int* indexData, int size);

	// Load a simple 8-bit per channel texture (grayscale, RGB or RGBA)
	GLuint loadTexture(const std::string& filename, bool generateMipmaps, glm::ivec2& textureSize);

	// Create a texture
	GLuint createTexture(const glm::ivec2& textureSize, GLenum format, GLenum internalFormat);

	void updateTexture(uint32_t texture, const void* data);

	void deleteTexture(uint32_t& texture);

	// Create a buffer
	GLuint createBuffer(size_t numBytes, const void* data, GLenum usage);

	void deleteBuffer(uint32_t& buffer);

	// update an SSBO buffer
	void updateSSBO(GLuint ssbo, size_t offset, size_t size, const void* data);

	// Pack an RGBA color as a uint (1 byte per component)
	uint32_t packColor(const glm::vec4& color);

	// Unpack an RGBA color from a uint (1 byte per component)
	glm::vec4 unpackColor(const uint32_t color);
}