#include "grid.h"

#include <algorithm>

using namespace glm;

namespace rlf
{
	const std::vector<glm::ivec2>& Nb4() 
	{ 
		static std::vector<glm::ivec2> v{ {1,0},{0,1},{-1,0},{0,-1} }; 
		return v; 
	}

	const std::vector<glm::ivec2>& Nb8() 
	{ 
		static std::vector<glm::ivec2> v{ {1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1} }; 
		return v; 
	}

	void Line(std::vector<glm::ivec2>& points, const glm::ivec2& start, const glm::ivec2& end)
	{
		auto dxy = abs(end - start);
		ivec2 sxy;
		sxy.x = start.x < end.x ? 1 : -1;
		sxy.y = start.y < end.y ? 1 : -1;
		auto err = dxy.x - dxy.y;

		points.resize(0);
		auto pt = start;
		while (true)
		{
			points.push_back(pt);
			if (pt == end) 
				break;
			auto e2 = 2 * err;
			if (e2 > -dxy.y)
			{
				err -= dxy.y;
				pt.x += sxy.x;
			}
			if (pt == end)
			{
				points.push_back(pt);
				break;
			}
			if (e2 < dxy.x)
			{
				err += dxy.x;
				pt.y += sxy.y;
			}
		}
	}

	void Line4(std::vector<glm::ivec2>& points, const glm::ivec2& start, const glm::ivec2& end)
	{
		points.resize(0);
		int x0 = start.x;
		int x1 = end.x;
		int y0 = start.y;
		int y1 = end.y;

		auto dx = abs(x1 - x0);   //# distance to travel in X
		auto dy = abs(y1 - y0);	//# distance to travel in Y

		int ix, iy;
		if (x0 < x1)
			ix = 1;//           # x will increase at each step
		else
			ix = -1;

		if (y0 < y1)
			iy = 1;//           # y will increase at each step
		else
			iy = -1;//          # y will decrease at each step

		float e = 0.0f;                //# Current error

		int tot = dx + dy;
		for (int i = 0; i < tot; ++i)
		{
			points.push_back(ivec2(x0, y0));
			float e1 = e + dy;
			float e2 = e - dx;
			if (abs(e1) < abs(e2))
			{
				//# Error will be smaller moving on X
				x0 += ix;
				e = e1;
			}
			else
			{
				//Error will be smaller moving on Y
				y0 += iy;
				e = e2;
			}
		}
		points.push_back(end);
	}

	void Circle(std::vector<glm::ivec2>& points, const glm::ivec2& center, int radius, bool sortByDistance)
	{
		static const auto fnSqLength = [](int x, int y) {return x * x + y * y; };
		points.resize(0);
		auto radius2 = radius * radius;
		for (int y = -radius; y <= radius; ++y)
			for (int x = -radius; x <= radius; ++x)
				if (fnSqLength(x,y) <= radius2)
					points.push_back({ x,y });
		if (sortByDistance)
			std::sort(points.begin(), points.end(), [](const ivec2& lhs, const ivec2& rhs) {return fnSqLength(lhs.x, lhs.y) < fnSqLength(rhs.x, rhs.y); });
		for (auto& p : points)
			p += center;
	}

	void Square(std::vector<glm::ivec2>& points, const glm::ivec2& center, int radius, bool sortByDistance)
	{
		static const auto fnLength = [](int x, int y) {return max(abs(x),abs(y)); };
		points.resize(0);
		for (int y = -radius; y <= radius; ++y)
			for (int x = -radius; x <= radius; ++x)
				if (fnLength(x, y) <= radius)
					points.push_back({ x,y });
		if (sortByDistance)
			std::sort(points.begin(), points.end(), [](const ivec2& lhs, const ivec2& rhs) {return fnLength(lhs.x, lhs.y) < fnLength(rhs.x, rhs.y); });
		for (auto& p : points)
			p += center;
	}

	void Diamond(std::vector<glm::ivec2>& points, const glm::ivec2& center, int radius, bool sortByDistance)
	{
		static const auto fnLength = [](int x, int y) {return abs(x) + abs(y); };
		points.resize(0);
		for (int y = -radius; y <= radius; ++y)
			for (int x = -radius; x <= radius; ++x)
				if (fnLength(x, y) <= radius)
					points.push_back({ x,y });
		if (sortByDistance)
			std::sort(points.begin(), points.end(), [](const ivec2& lhs, const ivec2& rhs) {return fnLength(lhs.x, lhs.y) < fnLength(rhs.x, rhs.y); });
		for (auto& p : points)
			p += center;
	}
}