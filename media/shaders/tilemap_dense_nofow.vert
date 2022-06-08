#version 430

// The dense shader is used for rendering a single quad, which we split into a grid
// Inputs:
//	 Uniforms:
//		tilemap texture, tile size, tile num
//		2d texture with all sprite indices for this layer
//			For each cell, we need (sprite_index, color_index)
//		2d offset to accomodate the camera location, in relevant layers

layout (location = 0) in vec3 aPos;

out vec2 uv;

void main()
{
	uv = aPos.xy;
	gl_Position = vec4(aPos.xy*2-1, aPos.z, 1.0);
}