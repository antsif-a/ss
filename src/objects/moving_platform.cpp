#include "moving_platform.hpp"

using biv::MovingPlatform;

MovingPlatform::MovingPlatform(
	const Coord& top_left, 
	const int width, const int height, 
	const int right_bound, Movable* rider
) : Box(top_left, width, height),
	Movable(top_left, width, height, 0, 0),
	rider(rider),
	left_bound(top_left.x),
	right_bound(right_bound) {}

bool MovingPlatform::is_rider_standing() const noexcept {
	return (
		rider != nullptr &&
		rider->get_vspeed() >= 0 &&
		rider->get_right() > get_left() &&
		rider->get_left() < get_right() &&
		rider->get_bottom() >= get_top() &&
		rider->get_bottom() <= get_top() + 1
	);
}

void MovingPlatform::step_sideways() noexcept {
	const float start_x = top_left.x;
	const float pos = top_left.x - get_map_offset();
	float next = pos + STEP * direction;
	if (next >= right_bound) {
		next = right_bound;
		direction = -1;
	} else if (next <= left_bound) {
		next = left_bound;
		direction = 1;
	}
	top_left.x += next - pos;
	
	if (is_rider_standing()) {
		rider->move_horizontal_offset(top_left.x - start_x);
	}
}

void MovingPlatform::move_horizontally() noexcept {
	if (frames_left-- > 0) {
		return;
	}
	frames_left = STEP_INTERVAL - 1;
	step_sideways();
}

void MovingPlatform::move_vertically() noexcept {}