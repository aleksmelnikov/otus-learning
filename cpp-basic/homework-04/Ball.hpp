#pragma once
#include "Painter.hpp"
#include "Point.hpp"
#include "Velocity.hpp"

// Шар для физического симулятора. Свойства шара спрятаны в приватные
// поля, доступ к ним — только через методы (инкапсуляция).
class Ball {
  public:
    // Конструктор: создаёт шар со всеми характеристиками сразу.
    // Конструктор нужен, чтобы в World::World собирать шары из прочитанных
    // из файла значений одной строкой.
    Ball(const Point& center, const Velocity& velocity, const Color& color,
         double radius);

    // Задать скорость шара
    void setVelocity(const Velocity& velocity);
    // Получить скорость шара
    Velocity getVelocity() const;

    // Задать координаты центра шара
    void setCenter(const Point& center);
    // Получить центр шара
    Point getCenter() const;

    // Получить радиус шара
    double getRadius() const;
    // Получить массу шара
    double getMass() const;

    // Нарисовать шар, используя painter
    void draw(Painter& painter) const;

  private:
    Point center;      // координаты центра шара
    Velocity velocity; // текущая скорость шара
    Color color;       // цвет заливки шара
    double radius;     // радиус шара (константа, сеттера нет)
};