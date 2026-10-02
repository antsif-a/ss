#include "console_moving_platform.hpp"

using biv::ConsoleMovingPlatform;

ConsoleMovingPlatform::ConsoleMovingPlatform(
	const Coord& top_left, 
	const int width, const int height, 
	const int right_bound, Movable* rider
) : MovingPlatform(top_left, width, height, right_bound, rider) {}

char ConsoleMovingPlatform::get_brush() const noexcept {
	return '=';
}