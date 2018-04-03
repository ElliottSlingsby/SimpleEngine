#pragma once

#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include <vector>

class DebugDraw {
public:
	struct Line {
		glm::dvec3 from;
		glm::dvec3 to;
		glm::tvec4<uint8_t> rgba = { 1.0, 1.0, 1.0, 1.0 };
		double width = 1.0;
		bool depth = true;

		bool active = true;
	};

	struct Point {
		glm::dvec3 position;
		glm::tvec4<uint8_t> rgba = { 1.0, 1.0, 1.0, 1.0 };
		double size = 4.0;
		bool depth = true;

		bool active = true;
	};

private:
	std::vector<Line> _lines;
	std::vector<Point> _points;

	bool _changed = false;

public:
	inline uint32_t bufferLine(const Line& line);
	inline uint32_t bufferPoint(const Point& point);

	inline uint32_t bufferLines(const std::vector<Line>& lines);
	inline uint32_t bufferPoints(const std::vector<Point>& points);

	inline void toggleLines(uint32_t start, uint32_t length, bool value);
	inline void togglePoints(uint32_t start, uint32_t length, bool value);

	inline uint32_t pointCount() const;
	inline uint32_t lineCount() const;

	inline void reset();

	friend class Renderer;
};

uint32_t DebugDraw::bufferLine(const Line& line) {
	_changed = true;
	_lines.push_back(line);
}

uint32_t DebugDraw::bufferPoint(const Point& point) {
	_changed = true;
	_points.push_back(point);
}

uint32_t DebugDraw::bufferLines(const std::vector<Line>& lines) {
	if (!lines.size())
		return;	

	_changed = true;
	_lines.insert(_lines.end(), lines.begin(), lines.end());
}

uint32_t DebugDraw::bufferPoints(const std::vector<Point>& points) {
	if (!points.size())
		return;

	_changed = true;
	_points.insert(_points.end(), points.begin(), points.end());
}

void DebugDraw::toggleLines(uint32_t start, uint32_t length, bool value) {
	if (!length || start + length > _lines.size())
		return;

	_changed = true;

	for (uint32_t i = start; i < start + length; i++) 
		_lines[i].active = value;
}

void DebugDraw::togglePoints(uint32_t start, uint32_t length, bool value) {
	if (!length || start + length > _points.size())
		return;

	_changed = true;

	for (uint32_t i = start; i < start + length; i++)
		_points[i].active = value;
}

uint32_t DebugDraw::pointCount() const {
	return static_cast<uint32_t>(_points.size());
}

uint32_t DebugDraw::lineCount() const {
	return static_cast<uint32_t>(_lines.size());
}

void DebugDraw::reset() {
	_changed = true;

	_points.clear();
	_lines.clear();
}