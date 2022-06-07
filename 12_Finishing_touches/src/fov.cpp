#include "fov.h"

// This is a port off the roguebasin implementation, changing types as necessary
// http://www.roguebasin.com/index.php/C%2B%2B_shadowcasting_implementation

#include <cmath>

typedef unsigned int uint;

static int multipliers[4][8] = {
    {1, 0, 0, -1, -1, 0, 0, 1},
    {0, 1, -1, 0, 0, -1, 1, 0},
    {0, 1, 1, 0, 0, -1, -1, 0},
    {1, 0, 0, 1, -1, 0, 0, -1}
};

void cast_light(
    const glm::ivec2& point,
    uint radius, 
    const glm::ivec2& map_size,
    uint row, 
    float start_slope, 
    float end_slope, 
    uint xx, 
    uint xy, 
    uint yx, 
    uint yy, 
    const std::function<bool(const glm::ivec2&)>& cb_is_opaque, 
    const std::function<void(const glm::ivec2&)>& cb_on_visible
) {
    if (start_slope < end_slope) {
        return;
    }
    float next_start_slope = start_slope;
    for (uint i = row; i <= radius; i++) {
        bool blocked = false;
        for (int dx = -i, dy = -i; dx <= 0; dx++) {
            float l_slope = (dx - 0.5) / (dy + 0.5);
            float r_slope = (dx + 0.5) / (dy - 0.5);
            if (start_slope < r_slope) {
                continue;
            }
            else if (end_slope > l_slope) {
                break;
            }

            int sax = dx * xx + dy * xy;
            int say = dx * yx + dy * yy;
            if ((sax < 0 && (uint)std::abs(sax) > point.x) ||
                (say < 0 && (uint)std::abs(say) > point.y)) {
                continue;
            }
            uint ax = point.x + sax;
            uint ay = point.y + say;
            if (ax >= map_size.x || ay >= map_size.y) {
                continue;
            }

            uint radius2 = radius * radius;
            if ((uint)(dx * dx + dy * dy) < radius2) {
                cb_on_visible({ ax,ay });
            }

            if (blocked) {
                if (cb_is_opaque({ ax, ay })) {
                    next_start_slope = r_slope;
                    continue;
                }
                else {
                    blocked = false;
                    start_slope = next_start_slope;
                }
            }
            else if (cb_is_opaque({ ax, ay })) {
                blocked = true;
                next_start_slope = r_slope;
                cast_light(point, radius, map_size, i + 1, start_slope, l_slope, xx,
                    xy, yx, yy, cb_is_opaque, cb_on_visible);
            }
        }
        if (blocked) {
            break;
        }
    }
}

namespace rlf
{
	void CalculateFieldOfView(
        const glm::ivec2& start, 
        int radius, 
        const glm::ivec2& map_size, 
        const std::function<bool(const glm::ivec2&)>& cb_is_opaque, 
        const std::function<void(const glm::ivec2&)>& cb_on_visible
    )
	{
        cb_on_visible(start);
        for (uint i = 0; i < 8; i++) {
            cast_light(start, radius, map_size, 1, 1.0, 0.0, multipliers[0][i],
                multipliers[1][i], multipliers[2][i], multipliers[3][i], cb_is_opaque, cb_on_visible);
        }
	}
}