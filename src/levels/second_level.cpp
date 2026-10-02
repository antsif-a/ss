#include "second_level.hpp"

#include "third_level.hpp"

using biv::SecondLevel;

SecondLevel::SecondLevel(UIFactory* ui_factory) : GameLevel(ui_factory) {
	init_data();
}

biv::GameLevel* SecondLevel::get_next() {
	if (!next) {
		clear_data();
		next = new biv::ThirdLevel(ui_factory);
	}
	return next;
}

// ----------------------------------------------------------------------------
// 									PROTECTED
// ----------------------------------------------------------------------------
void SecondLevel::init_data() {
	// Марио создаётся первым: движущиеся платформы запоминают его как пассажира.
	ui_factory->create_mario({39, 10}, 3, 3);
	
	// Стартовый корабль.
	ui_factory->create_ship({20, 25}, 40, 2);
	ui_factory->create_full_box({50, 16}, 5, 3);
	
	// Первая переправа: платформа на уровне корабля, над ней кружит враг.
	ui_factory->create_moving_platform({60, 25}, 10, 2, 85);
	ui_factory->create_flyable_enemy({65, 19}, 3, 2, 88, 2);
	
	ui_factory->create_ship({95, 25}, 20, 2);
	ui_factory->create_enemy({105, 5}, 3, 2);
	
	// Высокая мачта и вторая переправа.
	ui_factory->create_ship({125, 20}, 10, 7);
	ui_factory->create_moving_platform({135, 20}, 8, 2, 165);
	ui_factory->create_flyable_enemy({140, 14}, 3, 2, 165, 2);
	
	ui_factory->create_ship({180, 25}, 15, 2);
	ui_factory->create_full_box({186, 16}, 5, 3);
	
	// Третья переправа: на платформу нужно запрыгнуть.
	ui_factory->create_moving_platform({198, 22}, 8, 2, 225);
	ui_factory->create_flyable_enemy({200, 15}, 3, 2, 228, 3);
	
	// Финиш: последний статический объект уровня.
	ui_factory->create_ship({245, 20}, 20, 7);
}
