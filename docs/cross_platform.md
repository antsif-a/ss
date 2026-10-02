# Кроссплатформенность в приложении

Задание 6 из `README.md`:

> Изучить, как построена кроссплатформенность в приложении.

---

## 1. Идея

Логика игры не должна знать, на какой ОС запущена. Всё, что зависит от
ОС, вынесено в один каталог — `src/os_controls/`. Игровой код работает
только с двумя абстракциями и никогда не включает `windows.h` или
`ncurses.h` сам.

```
src/os_controls/
├── os_api/                     ← ЧТО именно должно уметь приложение
│   ├── keyboard.hpp            KeyBoard, UserInput
│   ├── os_control_settings.hpp OSControlSettings
│   └── CMakeLists.txt
├── windows/                    ← КАК это делается на Windows
│   ├── windows_keyboard.{hpp,cpp}
│   ├── windows_control_settings.{hpp,cpp}
│   └── CMakeLists.txt
├── linux/                      ← КАК это делается на Linux
│   ├── linux_keyboard.{hpp,cpp}
│   ├── linux_control_settings.{hpp,cpp}
│   └── CMakeLists.txt
└── CMakeLists.txt
```

Ключевая мысль: каталог `os_api` не содержит ни строки кода, специфичного
для ОС. Это чистый контракт, общий для всех платформ. Слово **«api»**
означает, что это интерфейс, а не реализация.

---

## 2. Контракт: два интерфейса

`src/os_controls/os_api/keyboard.hpp`:

```cpp
enum class UserInput {
    EXIT, MAP_LEFT, MAP_RIGHT, MARIO_JUMP, NO_INPUT
};

class KeyBoard {
    public:
        virtual UserInput get_user_input() = 0;
        virtual void off() = 0;
        virtual void on()  = 0;
};
```

`src/os_controls/os_api/os_control_settings.hpp`:

```cpp
class OSControlSettings {
    public:
        virtual void init() = 0;
        virtual void set_cursor_start_position() = 0;
};
```

Важно, что `UserInput` — это **общий словарь**. Он объявлен в
платформенно-независимом заголовке, хотя и выглядит «про ввод». Именно
поэтому игровой цикл в `main.cpp` может писать
`case biv::UserInput::MAP_LEFT:` не зная, откуда пришёл ввод.

Заметьте, что интерфейс описывает не «клавиши», а **намерения**
(«двигать карту влево», «прыгнуть», «выйти»). Это признак правильной
абстракции: контракт не протекает деталями реализации.

---

## 3. Реализации

### Windows

```cpp
// windows_keyboard.cpp
UserInput WindowsKeyBoard::get_user_input() {
    if      (GetKeyState('A') < 0)     return UserInput::MAP_RIGHT;
    else if (GetKeyState('D') < 0)     return UserInput::MAP_LEFT;
    else if (GetKeyState(VK_SPACE) < 0) return UserInput::MARIO_JUMP;
    else if (GetKeyState('Q') < 0)     return UserInput::EXIT;
    else                               return UserInput::NO_INPUT;
}
```

```cpp
// windows_control_settings.cpp
void WindowsControlSettings::init() {
    void* handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO structCursorInfo;
    GetConsoleCursorInfo(handle, &structCursorInfo);
    structCursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(handle, &structCursorInfo);
}
```

### Linux (ncurses)

```cpp
// linux_keyboard.cpp
UserInput LinuxKeyboard::get_user_input() {
    int c = getch();
    switch (c) {
        case 'd': prev_input = UserInput::MAP_LEFT;  return UserInput::MAP_LEFT;
        case 'a': prev_input = UserInput::MAP_RIGHT; return UserInput::MAP_RIGHT;
        case 's': prev_input = UserInput::NO_INPUT; return UserInput::NO_INPUT;
        case ' ': return UserInput::MARIO_JUMP;
        case 'q': return UserInput::EXIT;
    }
    switch (prev_input) {                 // ← вот и «эмуляция» удержания
        case UserInput::MAP_LEFT:  return UserInput::MAP_LEFT;
        case UserInput::MAP_RIGHT: return UserInput::MAP_RIGHT;
        default:                   return UserInput::NO_INPUT;
    }
}
```

```cpp
// linux_control_settings.cpp
void LinuxControlSettings::init() {
    initscr();  cbreak();  noecho();
    keypad(stdscr, TRUE);   // трансляция стрелок в escape-последовательности
    nodelay(stdscr, TRUE);  // getch() не блокирует
    curs_set(0);            // скрыть курсор
    getmaxyx(stdscr, height, width);
}
```

---

## 4. Почему управления на двух ОС различаются

Это **центральный** вопрос задания, и он уже сформулирован в `README.md`.
Разберём его строго.

### Windows: система даёт СОСТОЯНИЕ

`GetKeyState('A')` в любой момент возвращает «нажата ли сейчас клавиша A».
Это **уровень**, а не событие. Отсюда:

- можно опрашивать клавиатуру каждый кадр и получать осмысленный ответ;
- «отпустить» клавишу — это просто состояние «не нажата ни одна»;
- поэтому отдельной клавиши «стоп» не требуется: отпустил `A` — и Марио стоит.

```cpp
GetKeyState('A') < 0   // < 0  означает "клавиша нажата прямо сейчас"
```

### Linux: терминал даёт ПОТОК СИМВОЛОВ

Терминал изначально проектировался как последовательный порт: он не
имеет понятия «клавиша нажата». Он умеет только одно — выдать символ в
момент нажатия. Даже в режиме `cbreak()` повторное удержание клавиши
даёт **повторные символы**, а не состояние. Иными словами, терминал
сообщает только **фронты** (edges), а не **уровни** (levels).

Отсюда необходимость `prev_input`: если в этом кадре `getch()` ничего не
вернул (ключ всё ещё зажат, но нового символа нет), надо **помнить**
последнее направление и вернуть его снова. Это и делает второй `switch`.

Клавиша `S` существует только потому, что в потоке символов **невозможно
передать отпускание**. Единственный способ сказать терминалу «стоп» —
это явное событие, то есть отдельная нажатая клавиша.

### Следствие: разная задержка кадра

```cpp
#ifdef WINDOWS_CONSOLE
    frame_delay = std::chrono::milliseconds(10);
#elif defined(LINUX_CONSOLE)
    frame_delay = std::chrono::milliseconds(50);
#endif
```

`GetKeyState` — мгновенный вызов, опрашивать дёшево. `getch()` даже под
`nodelay(stdscr, TRUE)` работает через терминальный драйвер, что дороже.
Поэтому Linux идёт с шагом 50 мс.

Это, кстати, заметная проблема дизайна: **стоимость ввода задана
магическим числом в `main.cpp`**, а не свойством самого слоя ввода.
Правильнее было бы спрятать частоту опроса в реализацию `KeyBoard`.

---

## 5. Как выбирается реализация

Выбор происходит **на этапе сборки**, в три шага.

### Шаг 1. Скрипт задаёт `GAME_TYPE`

```bash
# build.sh
GAME_TYPE="LinuxConsole"
cmake -G Ninja -DGAME_TYPE="${GAME_TYPE}" ../src
```

```bat
:: build.bat
set GAME_TYPE=WindowsConsole
cmake -G Ninja -DGAME_TYPE=%GAME_TYPE% ..\src
```

### Шаг 2. CMake превращает это в макрос и подключает нужные .cpp

`src/CMakeLists.txt`:

```cmake
if(GAME_TYPE STREQUAL "WindowsConsole")
    target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOWS_CONSOLE)
elseif(GAME_TYPE STREQUAL "LinuxConsole")
    target_link_libraries(${PROJECT_NAME} ncurses pthread)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LINUX_CONSOLE)
endif()
```

`src/os_controls/CMakeLists.txt`:

```cmake
add_subdirectory(os_api)          # интерфейсы — всегда

if(GAME_TYPE STREQUAL "WindowsConsole")
    add_subdirectory(windows)      # реализация — только своя
elseif(GAME_TYPE STREQUAL "LinuxConsole")
    add_subdirectory(linux)
endif()
```

То есть файлы чужой платформы **даже не компилируются**. В сборке для
Linux физически нет ни одного `#include <windows.h>`.

### Шаг 3. `main.cpp` выбирает объекты через `#ifdef`

```cpp
biv::OSControlSettings* control_settings = nullptr;
biv::KeyBoard* keyboard = nullptr;
#ifdef WINDOWS_CONSOLE
    control_settings = new biv::WindowsControlSettings();
    keyboard        = new biv::WindowsKeyBoard();
#elif defined(LINUX_CONSOLE)
    control_settings = new biv::LinuxControlSettings(map_height, map_weight);
    keyboard        = new biv::LinuxKeyboard();
#endif

control_settings->init();
keyboard->on();
```

**Дальше во всём игровом цикле** (около 90 строк: ввод, обновление,
отрисовка, проверка конца) используются **только** вызовы через
указатели на базовые типы:

```cpp
user_input   = keyboard->get_user_input();       // виртуальный вызов
control_settings->set_cursor_start_position();   // виртуальный вызов
```

Ни одного `if (windows)`, ни одного `GetKeyState` в логике игры. Это и
означает «логика не зависит от ОС».

---

## 6. Границы изоляции: где протекает ОС

Изоляция **почти** полная. Осталось два места, где `#ifdef` есть
за пределами `os_controls`.

### `ConsoleGameMap::show()`

```cpp
void ConsoleGameMap::show() const noexcept {
    #ifdef WINDOWS_CONSOLE
        for (int i = 0; i < height; i++) std::cout << map[i];
    #elif defined(LINUX_CONSOLE)
        for (int i = 0; i < height; i++) { move(i, 0); addstr(map[i]); }
        ::refresh();
    #endif
}
```

Причина техническая: на Windows вывод идёт через `iostream` в перенаправленный
stdout, на Linux — через ncurses с позиционированием. При этом
`console_game_map.cpp` включает `<ncurses.h>` под `#ifdef LINUX_CONSOLE`.

Это всё ещё в слое представления, а не в модели, — формально
допустимо. Но правильнее было бы спрятать и это: добавить в
`OSControlSettings` метод `put_line(int y, const char* str)` и оставить
`ConsoleGameMap` полностью переносимым. Тогда платформенных ветвлений
в проекте не останется вообще.

### Комментарии в `LinuxControlSettings::init()`

В начале функции лежит закомментированный блок из пяти вызовов ncurses —
следы итераций. Мёртвый код, но безвредный.

---

## 7. Ловушка при чтении кода: инвертированные имена

Обратите внимание на уже приведённый код `linux_keyboard.cpp`:

```cpp
case 'd': return UserInput::MAP_LEFT;    // D  → MAP_LEFT  ?!
case 'a': return UserInput::MAP_RIGHT;   // A  → MAP_RIGHT ?!
```

`WindowsKeyBoard` сделано **точно так же** — так что хотя бы платформы
согласованы между собой, но само имя сбивает с толку.

Причина в том, что `MAP_LEFT` / `MAP_RIGHT` названы **с точки зрения
карты**, а не персонажа. В `main.cpp`:

```cpp
case biv::UserInput::MAP_LEFT:
    mario->move_map_left();
    if (!game.check_static_collisions(mario)) {
        game.move_map_left();     // мир едет влево
    }
    mario->move_map_right();
```

Мир сдвигается влево ⇒ Марио **визуально движется вправо**. Поэтому `D`
(вправо) возвращает `MAP_LEFT`. В `README.md` описано поведение
персонажа (`A` — влево, `D` — вправо), и оно **правильное**; расходится
только наименование.

Полезно проговорить на защите, потому что при чтении `switch` в
`linux_keyboard.cpp` первым делом бросается в глаза «перевёрнутое»
направление.

---

## 8. Критика

### Что сделано хорошо

1. **Чистое разделение контракта и реализации.** Каталог `os_api` не
   содержит платформенного кода. Это то, что нужно было сделать, и это
   сделано.
2. **Абстракция выражена намерениями, а не клавишами.** `UserInput`
   не «KEY_A», а «MAP_LEFT». Смена раскладки или переход на геймпад не
   затронули бы игровой цикл.
3. **Вынесено всё, что реально различается.** Различия не выдуманы, а
   вытекают из устройства ОС, и для каждого есть внятное объяснение
   (уровень против фронта, дешёвый опрос против дорогого).
4. **Чужие файлы не компилируются.** Условный `add_subdirectory` —
   более строгое решение, чем `#ifdef` внутри одного `.cpp`.
5. **Полиморфизм использован по назначению.** Указатели
   `KeyBoard*` и `OSControlSettings*` — обычный полиморфизм, без
   приведения типов вниз.

### Что можно улучшить

1. **Выбор платформы — на этапе сборки, а не во время выполнения.**
   Одна сборка работает только на одной ОС. Формально приложение
   «кроссплатформенное» (логика переносима), но не «универсальное»
   (один бинарник на все ОС). Полноценное решение — регистрировать
   фабрики платформ и выбирать в рантайме по `ifdef`-ам при старте
   либо через подключаемые модули.
2. **Два одинаковых `target_include_directories`** подряд в каждом
   `CMakeLists.txt`. Безвредно, но это мусор.
3. **Сырые `new` для объектов ОС** без `delete`. Для двух объектов,
   живущих всю программу, несущественно, но непоследовательно.
4. **`frame_delay` — платформенный магический литерал в `main.cpp`.**
   Скорость опроса ввода — свойство реализации `KeyBoard`, а не
   константа вызывающего кода.
5. **`on()`/`off()` — чистые виртуальные с пустыми реализациями.**
   `WindowsKeyBoard::on()` и `off()` пусты, а `LinuxKeyboard::off()`
   сбрасывает `prev_input`. То есть `off()` означает «отпусти
   защёлку» на Linux и «ничего» на Windows. Контракт выглядит
   одинаковым, но семантика разная. Реализацию по умолчанию можно
   было дать в базовом классе.
6. **`map_weight` вместо `map_width`** — опечатка в `main.cpp`
   (`map_weight = 200`), к тому же «вес» вместо «ширины».
7. **Нет третьей платформы.** Архитектура её выдержит, но цепочка
   `if/elseif` в трёх местах (`src/CMakeLists.txt`,
   `src/os_controls/CMakeLists.txt`, `main.cpp`) потребует расширения.
   Шаблон или единая точка регистрации были бы устойчивее.
8. **Ранний выход, если `GAME_TYPE` не задан.** При сборке без флага
   ни `control_settings`, ни `keyboard` не будут созданы, и
   `main.cpp` упадёт на разыменовании `nullptr` вместо внятной
   диагностики.

---

## 9. Рецепт: как добавить третью ОС

Практический вывод, который обычно и требуется на защите.

1. Создать `src/os_controls/<os>/` с реализациями `KeyBoard` и
   `OSControlSettings`. **Интерфейсы не трогать** — если понадобился
   новый метод, значит, абстракция неполна.
2. Описать в `src/os_controls/<os>/CMakeLists.txt` исходники и
   `target_include_directories`.
3. Добавить ветку в `src/os_controls/CMakeLists.txt`:
   `add_subdirectory(<os>)`.
4. Добавить ветку в `src/CMakeLists.txt`: макрос определения и
   необходимые библиотеки.
5. Добавить `#elif` в `main.cpp` — три строки создания объектов.
6. Добавить ветку в `ConsoleGameMap::show()`, **если** вывод отличается.

Ни один файл модели, контроллера, уровней или объектов править **не
требуется**. Это и есть проверка качества абстракции: если при
добавлении платформы приходится трогать `Game` или `GameLevel` —
значит, ОС-логика протекла куда-то ещё.

Именно это стоит сказать в ответе на задание 6: ценность конструкции не
в том, что сегодня есть две ОС, а в том, что игровая логика физически не
может узнать, на какой ОС работает.
