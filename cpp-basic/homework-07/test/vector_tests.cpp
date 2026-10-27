// ============================================================================
// Тесты последовательного контейнера std::vector.
//
// Проверяются основные сценарии работы: создание, вставка в начало,
// середину и конец, удаление с начала, середины и конца, доступ к элементам,
// получение размера, копирование, перемещение и уничтожение.
//
// Имена тестов имеют вид vector.<сценарий>, поэтому можно запускать только
// эту группу: ./containers_tests --gtest_filter=vector.*
// ============================================================================

#include "tracked_type.hpp"

#include <gtest/gtest.h>

#include <vector>
#include <stdexcept>
#include <utility>

// Созданный по умолчанию вектор пуст, его размер равен нулю.
TEST(vector, create_empty) {
    const std::vector<int> values;

    EXPECT_TRUE(values.empty());
    EXPECT_EQ(values.size(), 0u);
}

// Добавление элементов в конец сохраняет порядок и увеличивает размер.
TEST(vector, push_back) {
    std::vector<int> values;

    values.push_back(1);
    values.push_back(2);
    values.push_back(3);

    const std::vector<int> expected{1, 2, 3};
    EXPECT_EQ(values, expected);
    EXPECT_EQ(values.size(), expected.size());
}

// У std::vector нет push_front, поэтому вставка в начало выполняется
// через insert(begin(), value).
TEST(vector, push_front) {
    std::vector<int> values{2, 3};

    values.insert(values.begin(), 1);
    values.insert(values.begin(), 0);

    const std::vector<int> expected{0, 1, 2, 3};
    EXPECT_EQ(values, expected);
}

// Вставка в середину по итератору: элемент 3 встаёт перед элементом 4.
TEST(vector, insert_middle) {
    std::vector<int> values{1, 2, 4, 5};

    values.insert(values.begin() + 2, 3);

    const std::vector<int> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(values, expected);
}

// Удаление последнего элемента уменьшает размер на единицу.
TEST(vector, pop_back) {
    std::vector<int> values{1, 2, 3};

    values.pop_back();

    const std::vector<int> expected{1, 2};
    EXPECT_EQ(values, expected);
}

// Удаление первого элемента выполняется через erase(begin()).
TEST(vector, pop_front) {
    std::vector<int> values{1, 2, 3};

    values.erase(values.begin());

    const std::vector<int> expected{2, 3};
    EXPECT_EQ(values, expected);
}

// Удаление из середины: элементы после удалённого сдвигаются влево.
TEST(vector, erase_middle) {
    std::vector<int> values{1, 2, 3, 4};

    values.erase(values.begin() + 1);

    const std::vector<int> expected{1, 3, 4};
    EXPECT_EQ(values, expected);
}

// Получение элементов разными способами: front(), back(), [] и at().
TEST(vector, get_elements) {
    std::vector<int> values{10, 20, 30};

    EXPECT_EQ(values.front(), 10);
    EXPECT_EQ(values.back(), 30);
    EXPECT_EQ(values[1], 20);
    EXPECT_EQ(values.at(2), 30);
}

// Размер пустого вектора равен 0, после двух вставок — 2.
TEST(vector, size) {
    std::vector<int> values;

    EXPECT_EQ(values.size(), 0u);

    values.push_back(1);
    values.push_back(2);

    EXPECT_EQ(values.size(), 2u);
}

// at() проверяет границы и бросает std::out_of_range при неверном индексе.
TEST(vector, at_out_of_range) {
    std::vector<int> values{1, 2, 3};

    EXPECT_THROW(static_cast<void>(values.at(values.size())), std::out_of_range);
}

// Доп. задание 1: после копирования содержимое обоих контейнеров одинаково,
// а изменение копии не влияет на оригинал.
TEST(vector, copy) {
    const std::vector<int> source{1, 2, 3};

    std::vector<int> copyConstructed(source);
    std::vector<int> copyAssigned;
    copyAssigned = source;

    EXPECT_EQ(copyConstructed, source);
    EXPECT_EQ(copyAssigned, source);

    copyConstructed.push_back(4);
    EXPECT_NE(copyConstructed, source);
    EXPECT_EQ(source, (std::vector<int>{1, 2, 3}));
}

// Доп. задание 3: перемещающее присваивание переносит содержимое
// в целевой контейнер, а источник остаётся пустым.
TEST(vector, move_assignment) {
    std::vector<int> source{1, 2, 3};
    std::vector<int> target{9};

    target = std::move(source);

    const std::vector<int> expected{1, 2, 3};
    EXPECT_EQ(target, expected);
    EXPECT_TRUE(source.empty());
}

// Доп. задание 2: при уничтожении контейнера вызываются деструкторы всех
// хранящихся в нём элементов. Проверяем это по счётчику живых объектов.
TEST(vector, destruction) {
    const std::size_t aliveBefore = TrackedType::aliveCount();

    {
        std::vector<TrackedType> values;
        values.reserve(3);
        values.emplace_back();
        values.emplace_back();
        values.emplace_back();

        // Пока контейнер жив, все три объекта существуют.
        EXPECT_EQ(TrackedType::aliveCount(), aliveBefore + values.size());
    }

    // После выхода из области видимости контейнер уничтожил все элементы.
    EXPECT_EQ(TrackedType::aliveCount(), aliveBefore);
}
