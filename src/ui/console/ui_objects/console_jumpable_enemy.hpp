#pragma once

#include "console_ui_obj_rect_adapter.hpp"
#include "jumpable_enemy.hpp"

namespace biv {
	class ConsoleJumpableEnemy : public JumpableEnemy, public ConsoleUIObjectRectAdapter {
		public:
			ConsoleJumpableEnemy(const Coord& top_left, const int width, const int height);

			char get_brush() const noexcept override;
	};
}
