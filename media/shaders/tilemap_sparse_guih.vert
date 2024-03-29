#version 430

// The sparse shader is used for rendering instanced pairs of (position, sprite), e.g. creatures, doors, etc
// Inputs:
//	 Uniforms:
//		tilemap texture, tile size, tile num
//		A 1d texture buffer, with all instance data for this layer
//			For each instance, we need (position, sprite_index, color_index)

layout (location = 0) in vec3 aPos;

uniform ivec2 screen_grid_size;
uniform int targetIdx;

out vec2 uv;
flat out ivec2 cell_idx;
flat out uvec2 spriteIndexAndColor;

// https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object
struct SparseInstanceData
{
    uvec2 screen_pos; 
    uint sprite_index;
    uint color;
};

// Uniform block named InstanceBlock, follows std140 alignment rules
layout (std140, binding = 0) 
readonly buffer SparseInstanceDataBufferLayout {
  SparseInstanceData instances[];
};

void main()
{
	uv = aPos.xy;

	SparseInstanceData sid = instances[gl_InstanceID];
	spriteIndexAndColor = uvec2(sid.sprite_index, sid.color);

	cell_idx = ivec2(sid.screen_pos);

	// Calculate the position in [0,1] space
	vec2 pos = (sid.screen_pos+aPos.xy) / vec2(screen_grid_size);
	// expand it in [-1,1], for OpenGL
	pos = pos*2-1;

	// when we're supposed to be blinking, push the quad far away to make it disappear for the blink duration
	if(gl_InstanceID == targetIdx)
		pos += 1000000;

	gl_Position = vec4(pos,aPos.z,1.0f);
}