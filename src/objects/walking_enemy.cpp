#include "walking_enemy.hpp"

using biv::WalkingEnemy;

WalkingEnemy::WalkingEnemy(
	const Coord& top_left, const int width, const int height
) : Enemy(top_left, width, height, 0.2f) {}

void WalkingEnemy::stand_on(Rect* obj) noexcept {
	top_left.x += hspeed;
	if (!has_collision(obj)) {
		process_horizontal_static_collision(obj);
	} else {
		top_left.x -= hspeed;
	}
}