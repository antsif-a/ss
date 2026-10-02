#include "console_flyable_enemy.hpp"

using biv::ConsoleFlyableEnemy;

ConsoleFlyableEnemy::ConsoleFlyableEnemy(
	const Coord& top_left, 
	const int width, const int height, 
	const int right_bound, const int amplitude
) : FlyableEnemy(top_left, width, height, right_bound, amplitude) {}

char ConsoleFlyableEnemy::get_brush() const noexcept {
	return 'f';
}