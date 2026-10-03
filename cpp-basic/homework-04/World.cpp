#include "World.hpp"
#include "Painter.hpp"
#include <fstream>

// Длительность одного тика симуляции.
// Подробнее см. update()
// Изменять не следует
static constexpr double timePerTick = 0.001;

/**
 * Конструирует объект мира для симуляции
 * @param worldFilePath путь к файлу модели мира
 */
World::World(const std::string& worldFilePath) {

    std::ifstream stream(worldFilePath);
    // Чтение границ мира из модели упрощено: для Point перегружен
    // operator>>, поэтому координаты читаются сразу в объекты.
    stream >> topLeft >> bottomRight;
    physics.setWorldBox(topLeft, bottomRight);

    // Переменные для чтения характеристик одного шара из модели.
    Point center;   // координаты центра шара
    Point velocity; // вектор скорости шара (vx, vy)
    Color color;    // цвет заливки шара (red, green, blue)
    double radius;  // радиус шара
    bool isCollidable; // обрабатывать ли столкновения шара

    // Читаем шары до конца файла.
    while (stream.peek(), stream.good()) {
        // Читаем координаты центра и вектор скорости через operator>>.
        stream >> center >> velocity;
        // Читаем цвет через operator>>.
        stream >> color;
        // Читаем радиус шара.
        stream >> radius;
        // Читаем свойство isCollidable. В базовой части задания
        // поведение шаров с isCollidable == false не отличается.
        stream >> std::boolalpha >> isCollidable;

        // Конструируем шар со свойствами, прочитанными выше,
        // и помещаем его в контейнер шаров.
        Ball ball(center, Velocity(velocity), color, radius);
        balls.push_back(ball);
    }
}

/// @brief Отображает состояние мира
void World::show(Painter& painter) const {
    // Рисуем белый прямоугольник, отображающий границу
    // мира
    painter.draw(topLeft, bottomRight, Color(1, 1, 1));

    // Вызываем отрисовку каждого шара
    for (const Ball& ball : balls) {
        ball.draw(painter);
    }
}

/// @brief Обновляет состояние мира
void World::update(double time) {
    /**
     * В реальном мире время течет непрерывно. Однако
     * компьютеры дискретны по своей природе. Поэтому
     * симуляцию взаимодействия шаров выполняем дискретными
     * "тиками". Т.е. если с момента прошлой симуляции
     * прошло time секунд, time / timePerTick раз обновляем
     * состояние мира. Каждое такое обновление - тик -
     * в physics.update() перемещаем шары и обрабатываем
     * коллизии - ситуации, когда в результате перемещения
     * один шар пересекается с другим или с границей мира.
     * В общем случае время не делится нацело на
     * длительность тика, сохраняем остаток в restTime
     * и обрабатываем на следующей итерации.
     */

    // учитываем остаток времени, который мы не "доработали" при прошлом update
    time += restTime;
    const auto ticks = static_cast<size_t>(std::floor(time / timePerTick));
    restTime = time - double(ticks) * timePerTick;

    physics.update(balls, ticks);
}