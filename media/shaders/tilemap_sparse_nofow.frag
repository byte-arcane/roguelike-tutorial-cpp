#version 430

 in vec2 uv;
 flat in ivec2 cell_idx;
 flat in uvec2 spriteIndexAndColor;
 

uniform sampler2D tilemap;
uniform ivec2 tilemap_tile_num;
uniform ivec2 tilemap_tile_size;

out vec4 frag_color;

vec4 unpack_color(uint value)
{
	return vec4( value & 255, (value>>8) & 255, (value>>16) & 255, (value>>24) & 255) / 255.0;
}

void main()
{
	vec2 rect_uv = uv;
	int sprite_index = int(spriteIndexAndColor.x);
	// ... and from its linear form, convert it to 2d
	ivec2 sprite_index_2d = ivec2(sprite_index% tilemap_tile_num.x, sprite_index / tilemap_tile_num.x);
	// sprite color is the 2nd component
	vec4 sprite_color = unpack_color(spriteIndexAndColor.y);
	// calculate the size (in UV space) for each sprite of the tilemap. 
	vec2 sprite_step = 1.0 / tilemap_tile_num;
	// invert the rect's y coordinate so that we sample the tilemap sprite correctly
	rect_uv.y = 1-rect_uv.y;
	// calculate the exact uv coordinate for the sprite
	vec2 sprite_uv = sprite_step * (sprite_index_2d + rect_uv);
	// ... and use it to get the data. 
    vec4 sprite_data = texture(tilemap, sprite_uv);
	// Also multiply by the sprite color that we've set in the map, and that's it!
	frag_color = sprite_data * sprite_color;
}