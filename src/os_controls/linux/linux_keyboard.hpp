#pragma once

#include <string>
#include <unordered_map>

#include "keyboard.hpp"

namespace biv {
	/**
		Клавиатура Linux читает события напрямую из устройства ввода
		/dev/input/eventN (evdev). В отличие от потока символов терминала,
		evdev сообщает о нажатии, удержании и отпускании каждой клавиши.

		Путь к устройству можно задать переменной окружения MARIO_KEYBOARD.
		Если устройство открыть не удалось (например, нет прав на чтение),
		используется ввод через ncurses.
	*/
	class LinuxKeyboard : public KeyBoard {
		private:
			// Значения поля value в событии клавиши (struct input_event).
			enum class KeyState {
				RELEASE = 0,
				PRESS = 1,
				REPEAT = 2
			};

			std::string device_path;
			int input_fd = -1;
			std::unordered_map<unsigned short, KeyState> key_states;
			UserInput prev_input = UserInput::NO_INPUT;

		public:
			LinuxKeyboard();
			~LinuxKeyboard();

			UserInput get_user_input() override;
			void on() override;
			void off() override;

		private:
			static std::string find_device_path();

			UserInput get_terminal_input();
			bool is_pressed(const unsigned short key);
			bool is_pressed_or_repeated(const unsigned short key);
			void poll_events();
	};
}
