#include "console_jumpable_enemy.hpp"

using biv::ConsoleJumpableEnemy;

ConsoleJumpableEnemy::ConsoleJumpableEnemy(
	const Coord& top_left, const int width, const int height
) : JumpableEnemy(top_left, width, height) {}

char ConsoleJumpableEnemy::get_brush() const noexcept {
	return 'j';
}
