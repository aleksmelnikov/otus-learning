#include "Color.hpp"

Color::Color() = default;

Color::Color(double red, double green, double blue)
    : r{red}, g{green}, b{blue} {}

double Color::red() const {
    return r;
}

double Color::green() const {
    return g;
}

double Color::blue() const {
    return b;
}

std::istream& operator>>(std::istream& stream, Color& color) {
    // Читаем три составляющие во временные переменные и собираем
    // объект Color через его конструктор.
    double red = 0.;
    double green = 0.;
    double blue = 0.;
    stream >> red >> green >> blue;
    color = Color(red, green, blue);
    return stream;
}
