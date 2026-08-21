// Продолжение braces_basics.cpp: интересные случаи различия
// инициализации круглыми () и фигурными {} скобками в C++.
//
// Сборка:
//   g++ (Linux / MinGW на Windows):
//     g++ -std=c++17 -Wall -Wextra -Wpedantic braces_gotchas.cpp -o braces_gotchas
//     g++ -std=c++20 -Wall -Wextra -Wpedantic braces_gotchas.cpp -o braces_gotchas  # для пункта 7
//   MSVC (Developer Command Prompt для Visual Studio):
//     cl /std:c++20 /W4 /EHsc braces_gotchas.cpp
//     (сразу C++20: пункт 7 требует его, а .exe создаётся сам)
//
// Примечание: при сборке появятся 3 предупреждения — это намеренно,
// они подсвечивают грабли из пунктов 1 и 2.
//
// Ожидаемый вывод:
//   == 1. Most vexing parse ==
//   w1 - это функция: true
//   w2.value = 42
//   == 2. Сужающие преобразования ==
//   int a(3.14) -> 3
//   char c(300) -> 44 (обрезка, implementation-defined)
//   == 3. Значения по умолчанию ==
//   zero = 0, p2.x = 0, p2.y = 0
//   arr2: 0 0 0
//   == 4. auto и фигурные скобки ==
//   auto n2 = {5}: элементов = 1
//   == 5. Один аргумент у vector ==
//   v1(5): 0 0 0 0 0
//   v2{5}: 5
//   == 6. Инициализация полей класса ==
//   x = 0, y = 0
//   == 7. Дизайнерские инициализаторы ==
//   p.x = 1, p.y = 2  (или подсказка, если компиляция не в режиме C++20)

#include <initializer_list>
#include <iostream>
#include <type_traits>
#include <vector>

// ---------- 1. Most vexing parse ----------
struct Widget {
    int value = 42;
};

// ---------- 6. Инициализация полей класса ----------
struct S {
    int x{0};    // ок: фигурные скобки
    int y = 0;   // ок: копирующая инициализация
    // int z(0); // ОШИБКА КОМПИЛЯЦИИ: круглые скобки здесь запрещены
};

int main() {

    // == 1. Most vexing parse ============================================
    // Круглые скобки без аргументов парсятся как ОБЪЯВЛЕНИЕ ФУНКЦИИ,
    // а не как создание объекта!
    Widget w1();   // это функция w1(), возвращающая Widget (без тела)
    Widget w2{};   // а вот это настоящий объект

    std::cout << "== 1. Most vexing parse ==\n";
    std::cout << std::boolalpha
              << "w1 - это функция: " << std::is_function_v<decltype(w1)> << "\n";
    std::cout << "w2.value = " << w2.value << "\n";
    // std::cout << w1.value;  // ошибка: у функции нет поля value

    // == 2. Сужающие преобразования ======================================
    // Фигурные скобки запрещают сужающие преобразования (narrowing),
    // круглые — разрешают (значение молча обрезается).
    int a(3.14);     // ок: обрезка до 3
    // int b{3.14};  // ОШИБКА КОМПИЛЯЦИИ: double -> int сужает значение
    char c(300);     // ок: переполнение char (implementation-defined)
    // char d{300};  // ОШИБКА КОМПИЛЯЦИИ: 300 не помещается в char

    std::cout << "\n== 2. Сужающие преобразования ==\n";
    std::cout << "int a(3.14) -> " << a << "\n";
    std::cout << "char c(300) -> " << static_cast<int>(c)
              << " (обрезка, implementation-defined)\n";

    // == 3. Значения по умолчанию ========================================
    // {} даёт нулевую инициализацию, () без аргументов или без ничего —
    // оставляет переменную с мусорным значением.
    [[maybe_unused]] int uninit;         // мусор
    int zero{};                          // гарантированный 0

    struct Point { int x, y; };
    [[maybe_unused]] Point garbage;      // x, y — мусор
    Point p2{};                          // x, y == 0

    [[maybe_unused]] int arr1[3];        // мусор
    int arr2[3]{};                       // все нули

    std::cout << "\n== 3. Значения по умолчанию ==\n";
    std::cout << "zero = " << zero
              << ", p2.x = " << p2.x
              << ", p2.y = " << p2.y << "\n";
    std::cout << "arr2: ";
    for (int val : arr2) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    // std::cout << uninit;  // раскомментируй — увидишь мусор
    //                        (формально чтение неинициализированной
    //                         переменной — undefined behavior)

    // == 4. auto и фигурные скобки =======================================
    // Внимание, сюрпризы с выводом типа:
    auto n1{5};     // C++11/14: initializer_list<int>! C++17+: int
    auto n2 = {5};  // ВСЕГДА initializer_list<int> (из-за знака =)
    auto n3(5);     // всегда int

    static_assert(std::is_same_v<decltype(n1), int>);
    static_assert(std::is_same_v<decltype(n2), std::initializer_list<int>>);
    static_assert(std::is_same_v<decltype(n3), int>);

    std::cout << "\n== 4. auto и фигурные скобки ==\n";
    std::cout << "auto n2 = {5}: элементов = " << n2.size()
              << " (тип проверен через static_assert)\n";

    // == 5. Один аргумент у vector =======================================
    // Частная, но частая ловушка из braces_basics.cpp:
    std::vector<int> v1(5);  // конструктор заполнения: пять нулей
    std::vector<int> v2{5};  // initializer_list: один элемент 5

    std::cout << "\n== 5. Один аргумент у vector ==\n";
    std::cout << "v1(5): ";
    for (int val : v1) {
        std::cout << val << " ";
    }
    std::cout << "\nv2{5}: ";
    for (int val : v2) {
        std::cout << val << " ";
    }
    std::cout << "\n";

    // == 6. Инициализация полей класса ===================================
    // Круглые скобки в объявлении полей запрещены (см. struct S выше).
    S s;
    std::cout << "\n== 6. Инициализация полей класса ==\n";
    std::cout << "x = " << s.x << ", y = " << s.y << "\n";

    // == 7. Дизайнерские инициализаторы (C++20) ==========================
    // Работают ТОЛЬКО с фигурными скобками.
    std::cout << "\n== 7. Дизайнерские инициализаторы ==\n";
#if __cplusplus >= 202002L
    Point p{.x = 1, .y = 2};
    std::cout << "p.x = " << p.x << ", p.y = " << p.y << "\n";
#else
    std::cout << "(пересобери с -std=c++20, чтобы увидеть этот пункт)\n";
#endif

}
