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

	// test if a point is within the map bounds
	bool PointInMapBounds(const ivec2& p, const ivec2& mapSize)
	{
		return p.x >= 0 && p.x < mapSize.x&& p.y >= 0 && p.y < mapSize.y;
	}

	// Reconstruct the path from start to goal. 
	// Actually do it backwards (goal to start) and reverse in the end
	void ReconstructPath(vector<ivec2>& path, const ivec2& goal, const unordered_map<ivec2, ivec2>& cameFrom)
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

	vector<ivec2> CalculatePath(const ivec2& start, const ivec2& goal, const ivec2& mapSize, const function<float(const glm::ivec2&)>& fnCost)
	{
		// This means "impassable"
		constexpr float INF_COST = std::numeric_limits<float>::infinity();
		
		// sanity check for our coordinates: start/goal being different, in map bounds, and goal not being unattainable
		if (!(start != goal && PointInMapBounds(start, mapSize) && PointInMapBounds(goal, mapSize)))
			return {};

		// Heuristic is the manhattan distance to goal
		const auto fnHeuristic = [&goal](const ivec2& p) {
			auto v = abs(p - goal);
			return v.x + v.y;
		};
		
		// This will store the g score: accumulated path cost from start to a point (which is the key to this map)
		std::unordered_map<ivec2, float> gScore;
		// This stores a map between a point (the key) and the point it came from (the value)
		unordered_map<ivec2, ivec2> cameFrom;
		// This is the frontier that we use to pick our candidate points. It's a priority queue, so we always want to pick the best candidate
		priority_queue< ivec2_and_weight_t> frontier;

		// start at the starting point
		frontier.push({ start, 0 });
		gScore[start] = 0.0f;

		std::vector<ivec2> path;

		// while we do have elements in the frontier to process
		while (!frontier.empty())
		{
			// get the best candidate and remove it from the frontier
			auto current = frontier.top().p;
			frontier.pop();
			// If it's the goal, reconstruct the path and exit
			if (current == goal)
			{
				ReconstructPath(path, goal, cameFrom);
				break;
			}	

			// get the g-score at this point (should be written)
			auto gScoreCurrent = gScore.at(current);
			// now check all 4 neighbours
			for (const auto& nbOffset : Nb4())
			{
				// if a neighbour is within the map bounds
				auto nb = current + nbOffset;
				if (PointInMapBounds(nb, mapSize))
				{
					// calculate the cost to go to that neighbour					
					// if it's the goal, allow any cost really (even inf), as we might plot a path to a blocker
					auto costNb = nb == goal ? 1.0f : fnCost(nb); 
					// if the cost is not "impassable"
					if (costNb != INF_COST)
					{
						// calculate the new g-score, and see if it's better than an already recorded g-score for this tile, or if no entry is recorded yet
						auto gScoreNbNew = gScoreCurrent + costNb;
						auto gScoreNbCurIt = gScore.find(nb);
						if (gScoreNbCurIt == gScore.end() || gScoreNbCurIt->second > gScoreNbNew)
						{
							// record the gscore
							gScore[nb] = gScoreNbNew;
							// calculate the fscore using the heuristic
							// fscore = gScore + hScore => (actual cost from start to current) + (estimated cost from current to goal)
							auto fScore = gScoreNbNew + fnHeuristic(nb);
							// put this point into the frontier. Flip the sign of the fscore so that when we pick a point from the priority queue, it's the one with the closest-to-zero weight
							frontier.push({ nb, -fScore });
							// store where did the neighbour tile came from, for this recorded g-score
							cameFrom[nb] = current;
						}
					}
				}
			}
		}

		return path;
	}
}