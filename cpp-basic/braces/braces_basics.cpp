// Пример показывает разницу между инициализацией круглыми скобками ()
// и фигурными скобками {} для std::vector (см. Скотт Мейерс,
// "Effective Modern C++", правило 7 про различие () и {}).
//
// Сборка:
//   g++ (Linux / MinGW на Windows):
//     g++ -std=c++17 -Wall -Wextra -Wpedantic braces_basics.cpp -o braces_basics
//   MSVC (Developer Command Prompt для Visual Studio):
//     cl /std:c++17 /W4 /EHsc braces_basics.cpp
//
// Ожидаемый вывод:
//   x: 20 20 20 20 20 20 20 20 20 20
//   y: 10 20
//   z: hi hi hi hi hi hi hi hi hi hi

#include <iostream>
#include <string>
#include <vector>

int main() {

    // Круглые скобки => вызывается конструктор заполнения
    // vector(size_type count, const T& value):
    // x содержит 10 элементов, каждый из которых равен 20
    auto x = std::vector<int>(10, 20);

    // Фигурные скобки => приоритет у конструктора от
    // std::initializer_list<int>, поэтому это НЕ "10 копий 20",
    // а список элементов:
    // y содержит 2 элемента: числа 10 и 20
    auto y = std::vector<int>{10, 20};

    // Хитрый случай: кажется, что должен сработать initializer_list,
    // но элемент 10 типа int НЕЛЬЗЯ неявно преобразовать в std::string
    // (у std::string нет конструктора от int). Значит, конструктор от
    // initializer_list<std::string> не подходит, и компилятор выбирает
    // обычный конструктор заполнения (count=10, value="hi"):
    // z содержит 10 строк, каждая из которых равна "hi"
    auto z = std::vector<std::string>{10, "hi"};

    // Вывод вектора x: 10 двадцаток
    std::cout << "x: ";
    for (int val : x) {
        std::cout << val << " ";
    }
    std::cout << "\n";

    // Вывод вектора y: два числа 10 и 20
    std::cout << "y: ";
    for (int val : y) {
        std::cout << val << " ";
    }
    std::cout << "\n";

    // Вывод вектора z: десять строк "hi"
    std::cout << "z: ";
    for (const std::string& val : z) {
        std::cout << val << " ";
    }
    std::cout << "\n";

}
