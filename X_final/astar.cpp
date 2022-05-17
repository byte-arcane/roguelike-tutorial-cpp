#include "astar.h"
#include <queue>
#include <glm/gtx/hash.hpp>
#include "grid.h"


using namespace glm;
using namespace std;

namespace rlf
{
	// useful for our priority queue
	struct ivec2_and_weight_t {
		ivec2 p;
		float w;
		bool operator < (const ivec2_and_weight_t& other) const { return w < other.w; }
	};

	bool pointInMapBounds(const ivec2& p, const ivec2& mapSize)
	{
		return p.x >= 0 && p.x < mapSize.x&& p.y >= 0 && p.y < mapSize.y;
	}

	void reconstructPath(vector<ivec2>& path, const ivec2& goal, const unordered_map<ivec2, ivec2>& cameFrom)
	{
		path.push_back(goal);
		auto it = cameFrom.find(goal);
		while (it != cameFrom.end())
		{
			const auto& p = it->second;
			path.push_back(p);
			it = cameFrom.find(p);
		}
		// remove last element, which should be the start point (we don't need it)
		path.pop_back();
		// reverse the path, as currently it's from goal to start
		std::reverse(path.begin(), path.end());
	}

	vector<ivec2> calcPath(const ivec2& start, const ivec2& goal, const ivec2& mapSize, const function<float(const glm::ivec2&)>& fnCost)
	{
		constexpr float INF_COST = std::numeric_limits<float>::infinity();
		
		// sanity check for our coordinates: start/goal being different, in map bounds, and goal not being unattainable
		if (!(start != goal && pointInMapBounds(start, mapSize) && pointInMapBounds(goal, mapSize)))
			return {};

		// Heuristic is the manhattan distance to goal
		const auto fnHeuristic = [&goal](const ivec2& p) {
			auto v = abs(p - goal);
			return v.x + v.y;
		};
		
		std::unordered_map<ivec2, float> gScore;
		unordered_map<ivec2, ivec2> cameFrom;
		priority_queue< ivec2_and_weight_t> frontier;

		frontier.push({ start, 0 });
		gScore[start] = 0.0f;

		std::vector<ivec2> path;
		while (!frontier.empty())
		{
			auto current = frontier.top().p;
			frontier.pop();
			if (current == goal)
			{
				reconstructPath(path, goal, cameFrom);
				break;
			}	

			auto gScoreCurrent = gScore.at(current);
			for (const auto& nbOffset : Nb4())
			{
				auto nb = current + nbOffset;
				if (pointInMapBounds(nb, mapSize))
				{
					// fscore = gScore + hScore => (actual cost from start to current) + (estimated cost from current to goal)
					// if it's the goal, allow any cost really (even inf)
					auto costNb = nb == goal ? 1.0f : fnCost(nb); 
					if (costNb != INF_COST)
					{
						auto gScoreNbNew = gScoreCurrent + costNb;
						auto gScoreNbCurIt = gScore.find(nb);
						if (gScoreNbCurIt == gScore.end() || gScoreNbCurIt->second > gScoreNbNew)
						{
							gScore[nb] = gScoreNbNew;
							auto fScore = gScoreNbNew + fnHeuristic(nb);
							frontier.push({ nb, -fScore });
							cameFrom[nb] = current;
						}
					}
				}
			}
		}

		return path;
	}
}