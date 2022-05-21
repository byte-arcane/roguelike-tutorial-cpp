#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace rlf
{
	std::string ReadTextFile(const std::string& path);
	void WriteTextFile(const std::string& path, const std::string& text);

	// Searches a media filename (shader, texture, model, etc)
	std::string MediaSearch(const std::string& path);

	// This takes as parameters the shader TEXT (not the filename)
	GLuint BuildShader(const char* vsource, const char* fsource);

	// Build a vertex buffer object that contains floats in any arrangement (array of floats, array of vec2, array of vec3, etc)
	GLuint BuildVBO(const float * vertexData, int size);

	// Build a vertex array object
	GLuint BuildVAO(GLuint vbo, int vertexSize);

	// Load a simple 8-bit per channel texture (grayscale, RGB or RGBA)
	GLuint LoadTexture(const std::string& filename, bool generateMipmaps, glm::ivec2& textureSize);

	// Create a texture
	GLuint CreateTexture(const glm::ivec2& textureSize, GLenum format, GLenum internalFormat);

	// Delete a texture and set the ID to 0
	void DeleteTexture(uint32_t& texture);

	// Create a buffer
	GLuint CreateBuffer(size_t numBytes, const void* data, GLenum usage);

	// Delete a buffer and set the ID to 0
	void DeleteBuffer(uint32_t& buffer);

	// update an SSBO buffer
	void UpdateSSBO(GLuint ssbo, size_t offset, size_t size, const void* data);

	// Pack an RGBA color as a uint (1 byte per component, LSB is red)
	uint32_t PackColor(const glm::vec4& color);

	// Unpack an RGBA color from a uint (1 byte per component, LSB is red)
	glm::vec4 UnpackColor(const uint32_t color);
}