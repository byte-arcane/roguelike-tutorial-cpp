#include "spritemap.h"

#include <gl/glew.h>
#include <glm/glm.hpp>

#include "utility.h"

using namespace glm;

namespace rlf
{
	void Spritemap::Init( const ivec2& size, const uvec2 * data)
	{
		texLayer = rlf::createTexture(size, GL_RG, GL_RG32UI);
		glBindTexture(GL_TEXTURE_2D, texLayer);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RG_INTEGER, GL_UNSIGNED_INT, data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Spritemap::Draw(uint32_t program) const
	{
		glBindTextureUnit(1, texLayer);
		glUniform1i(glGetUniformLocation(program, "spritemap"), 1);

		// Render the quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void Spritemap::Dispose()
	{
		rlf::deleteTexture(texLayer);
	}
}