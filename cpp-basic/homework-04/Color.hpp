#pragma once
#include <istream>

class Color {
  public:
    Color();
    Color(double red, double green, double blue);
    double red() const;
    double green() const;
    double blue() const;

  private:
    double r{};
    double g{};
    double b{};
};

// Чтение цвета из потока: три составляющие идут подряд.
std::istream& operator>>(std::istream& stream, Color& color);
