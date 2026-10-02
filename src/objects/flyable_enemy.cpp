#include "flyable_enemy.hpp"

#include <cmath>

using biv::FlyableEnemy;

FlyableEnemy::FlyableEnemy(
	const Coord& top_left, 
	const int width, const int height, 
	const int right_bound, const int amplitude
) : Enemy(top_left, width, height, 0.1f),
	left_bound(top_left.x),
	right_bound(right_bound),
	origin_y(top_left.y),
	amplitude(amplitude) {}

void FlyableEnemy::move_horizontally() noexcept {
	const float pos = top_left.x - get_map_offset();
	float next = pos + hspeed;
	if (next >= right_bound) {
		next = right_bound;
		hspeed = -hspeed;
	} else if (next <= left_bound) {
		next = left_bound;
		hspeed = -hspeed;
	}
	top_left.x += next - pos;
}

void FlyableEnemy::move_vertically() noexcept {
	phase += 0.1f;
	top_left.y = origin_y + amplitude * std::sin(phase);
}

void FlyableEnemy::stand_on(Rect* obj) noexcept {}