#version 430

 in vec2 uv;
 flat in ivec2 cell_idx;
 flat in uvec2 spriteIndexAndColor;
 

uniform sampler2D tilemap;
uniform ivec2 tilemap_tile_num;
uniform ivec2 tilemap_tile_size;
uniform float show_in_explored_areas;

uniform sampler2D fow;

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
	// calc the visibility value: 0 (invisible) 1 (explored) or 2 (visible)
	float visibility = texelFetch(fow, cell_idx,0).x * 255; 
	// if "show_in_explored_areas" is 0, then visibility values lower than 2 will become zero
	// if "show_in_explored_areas" is 1, then visibility values lower than 1 will become zero
	visibility *= step(1.5-show_in_explored_areas, visibility);
	// Also multiply by the sprite color that we've set in the map, and that's it!
	frag_color = sprite_data * sprite_color * (visibility * 0.5);
}