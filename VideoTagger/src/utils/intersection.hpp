#pragma once
#include <cmath>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace vt::utils::intersection
{
	inline constexpr float cross_product(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2)
	{
		return (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
	}

	inline constexpr float dot_product(const ImVec2& p1, const ImVec2& p2)
	{
		return p1.x * p2.x + p1.y * p2.y;
	}

	inline bool is_convex_polygon(const std::vector<ImVec2>& points)
	{
		if (points.size() < 3) return false;

		bool has_positive = false;
		bool has_negative = false;

		for (size_t i = 0; i < points.size(); ++i)
		{
			const ImVec2& p0 = points[i];
			const ImVec2& p1 = points[(i + 1) % points.size()];
			const ImVec2& p2 = points[(i + 2) % points.size()];

			float cross = cross_product(p0, p1, p2);

			if (cross > 0)
			{
				has_positive = true;
			}
			else if (cross < 0)
			{
				has_negative = true;
			}

			if (has_positive and has_negative) return false;
		}
		return true;
	}

	inline bool is_concave_polygon(const std::vector<ImVec2>& points)
	{
		return !is_convex_polygon(points);
	}

	inline float length(const ImVec2& vec)
	{
		return std::sqrt(vec.x * vec.x + vec.y * vec.y);
	}

	inline float distance(const ImVec2& p1, const ImVec2& p2)
	{
		return length(p1 - p2);
	}

	inline float distance_to_segment(const ImVec2& point, const ImVec2& p1, const ImVec2& p2)
	{
		ImVec2 v = p2 - p1;
		ImVec2 w = point - p1;

		float c1 = dot_product(w, v);
		if (c1 <= 0)
		{
			return length(w);
		}

		float c2 = dot_product(v, v);
		if (c2 <= c1)
		{
			return length(point - p2);
		}

		float b = c1 / c2;
		ImVec2 pb = p1 + v * b;
		return length(point - pb);
	}

	inline bool is_in_circle(const ImVec2& pos, const ImVec2& circle_pos, float radius)
	{
		return length(pos - circle_pos) <= radius;
	}

	inline bool is_in_rect(const ImVec2& pos, const ImRect& rect)
	{
		return (pos.x >= rect.Min.x and pos.x <= rect.Max.x) and (pos.y >= rect.Min.y and pos.y <= rect.Max.y);
	}

	inline bool is_on_line(const ImVec2& pos, const ImVec2& p1, const ImVec2& p2)
	{
		if ((pos.y < std::min(p1.y, p2.y)) or (pos.y > std::max(p1.y, p2.y))) return false;
		if ((pos.x < std::min(p1.x, p2.x)) or (pos.x > std::max(p1.x, p2.x))) return false;

		float cross = cross_product(p1, pos, p2);

		const float epsilon = 1e-6f;
		return std::abs(cross) < epsilon;
	}

	inline bool is_in_polygon(const ImVec2& pos, const std::vector<ImVec2>& points)
	{
		size_t n = points.size();
		bool inside = false;
		if (n < 3) return false;

		ImVec2 min = points[0];
		ImVec2 max = points[0];

		for (size_t i = 1; i < n; ++i)
		{
			const ImVec2& q = points[i];
			min.x = std::min(q.x, min.x);
			max.x = std::max(q.x, max.x);
			min.y = std::min(q.y, min.y);
			max.y = std::max(q.y, max.y);
		}

		if (pos.x < min.x or pos.x > max.x or pos.y < min.y or pos.y > max.y) return false;

		//https://wrfranklin.org/Research/Short_Notes/pnpoly.html		
		for (size_t i = 0, j = n - 1; i < n; j = i++)
		{
			if ((points[i].y > pos.y) != (points[j].y > pos.y) and pos.x < (points[j].x - points[i].x) * (pos.y - points[i].y) / (points[j].y - points[i].y) + points[i].x)
			{
				inside = !inside;
			}
		}
		return inside;
	}
}
