#include "rect_map_movable_adapter.hpp"

using biv::RectMapMovableAdapter;

RectMapMovableAdapter::RectMapMovableAdapter(
	const Coord& top_left, const int width, const int height) {
	this->top_left = top_left;
	this->width = width;
	this->height = height;
}

int RectMapMovableAdapter::get_map_offset() const noexcept {
	return map_offset;
}

void RectMapMovableAdapter::move_map_left() noexcept {
	top_left.x -= MapMovable::MAP_STEP;
	map_offset--;
}

void RectMapMovableAdapter::move_map_right() noexcept {
	top_left.x += MapMovable::MAP_STEP;
	map_offset++;
}
