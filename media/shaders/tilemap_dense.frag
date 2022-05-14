#version 430
in vec2 uv;

uniform sampler2D tilemap;
uniform ivec2 tilemap_tile_num;
uniform ivec2 tilemap_tile_size;

uniform usampler2D spritemap;
uniform ivec2 camera_offset;
uniform ivec2 screen_grid_size;

uniform sampler2D fow;

out vec4 frag_color;

vec4 unpack_color(uint value)
{
	return vec4( value & 255, (value>>8) & 255, (value>>16) & 255, (value>>24) & 255) / 255.0;
}

void main()
{
	// uv is in [0,1], so multiply it to get the cell coordinate
	vec2 uvs = uv * screen_grid_size;
	// calculate the 2d grid cell index
	ivec2 cell_idx = camera_offset+ivec2(floor(uvs));
	ivec2 spritemap_size = textureSize(spritemap,0);
	if(cell_idx.x < 0 || cell_idx.x >= spritemap_size.x || cell_idx.y < 0 || cell_idx.y >= spritemap_size.y)
		discard;

	// calculate the offset within a cell
	vec2 rect_uv = fract(uvs);
	// get the cell data
	uvec2 cell_data = texelFetch(spritemap, cell_idx,0).xy;
	// sprite index is the 1st component
	int sprite_index = int(cell_data.x);
	// ... and from its linear form, convert it to 2d
	ivec2 sprite_index_2d = ivec2(sprite_index% tilemap_tile_num.x, sprite_index / tilemap_tile_num.x);
	// sprite color is the 2nd component
	vec4 sprite_color = unpack_color(cell_data.y);
	// calculate the size (in UV space) for each sprite of the tilemap. 
	vec2 sprite_step = 1.0 / tilemap_tile_num;
	// invert the rect's y coordinate so that we sample the tilemap sprite correctly
	rect_uv.y = 1-rect_uv.y;
	// calculate the exact uv coordinate for the sprite
	vec2 sprite_uv = sprite_step * (sprite_index_2d + rect_uv);
	// ... and use it to get the data. 
    vec4 sprite_data = texture(tilemap, sprite_uv);
	// Also multiply by the sprite color that we've set in the map, and that's it!
	frag_color = sprite_data * sprite_color * (texelFetch(fow, cell_idx,0).x * 0.5 * 255);
}