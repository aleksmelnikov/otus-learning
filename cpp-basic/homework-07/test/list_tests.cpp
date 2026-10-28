// ============================================================================
// Тесты спискового контейнера std::list.
//
// Проверяются основные сценарии работы: создание, вставка в начало,
// середину и конец, удаление с начала, середины и конца, доступ к элементам,
// получение размера, копирование, перемещение и уничтожение.
//
// Имена тестов имеют вид list.<сценарий>, поэтому можно запускать только
// эту группу: ./containers_tests --gtest_filter=list.*
// ============================================================================

#include "tracked_type.hpp"

#include <gtest/gtest.h>

#include <list>
#include <iterator>
#include <utility>

// Созданный по умолчанию список пуст, его размер равен нулю.
TEST(list, create_empty) {
    const std::list<int> values;

    EXPECT_TRUE(values.empty());
    EXPECT_EQ(values.size(), 0u);
}

// Добавление элементов в конец сохраняет порядок и увеличивает размер.
TEST(list, push_back) {
    std::list<int> values;

    values.push_back(1);
    values.push_back(2);
    values.push_back(3);

    const std::list<int> expected{1, 2, 3};
    EXPECT_EQ(values, expected);
    EXPECT_EQ(values.size(), expected.size());
}

// У std::list есть собственная операция добавления в начало — push_front.
TEST(list, push_front) {
    std::list<int> values;

    values.push_front(2);
    values.push_front(1);
    values.push_front(0);

    const std::list<int> expected{0, 1, 2};
    EXPECT_EQ(values, expected);
}

// Вставка в середину: сначала сдвигаем итератор на нужную позицию,
// затем вставляем элемент перед ней.
TEST(list, insert_middle) {
    std::list<int> values{1, 2, 4, 5};

    std::list<int>::iterator position = values.begin();
    std::advance(position, 2);
    values.insert(position, 3);

    const std::list<int> expected{1, 2, 3, 4, 5};
    EXPECT_EQ(values, expected);
}

// Удаление последнего элемента уменьшает размер на единицу.
TEST(list, pop_back) {
    std::list<int> values{1, 2, 3};

    values.pop_back();

    const std::list<int> expected{1, 2};
    EXPECT_EQ(values, expected);
}

// Удаление первого элемента выполняется собственной операцией pop_front.
TEST(list, pop_front) {
    std::list<int> values{1, 2, 3};

    values.pop_front();

    const std::list<int> expected{2, 3};
    EXPECT_EQ(values, expected);
}

// Удаление из середины по итератору, полученному через std::advance.
TEST(list, erase_middle) {
    std::list<int> values{1, 2, 3, 4};

    std::list<int>::iterator position = values.begin();
    std::advance(position, 1);
    values.erase(position);

    const std::list<int> expected{1, 3, 4};
    EXPECT_EQ(values, expected);
}

// Получение элементов: первый и последний через front()/back(),
// элемент в середине — через итератор.
TEST(list, get_elements) {
    std::list<int> values{10, 20, 30};

    EXPECT_EQ(values.front(), 10);
    EXPECT_EQ(values.back(), 30);

    std::list<int>::const_iterator position = values.begin();
    std::advance(position, 1);
    EXPECT_EQ(*position, 20);
}

// Размер пустого списка равен 0, после двух вставок — 2.
TEST(list, size) {
    std::list<int> values;

    EXPECT_EQ(values.size(), 0u);

    values.push_back(1);
    values.push_front(0);

    EXPECT_EQ(values.size(), 2u);
}

// Доп. задание 1: после копирования содержимое обоих контейнеров одинаково,
// а изменение копии не влияет на оригинал.
TEST(list, copy) {
    const std::list<int> source{1, 2, 3};

    std::list<int> copyConstructed(source);
    std::list<int> copyAssigned;
    copyAssigned = source;

    EXPECT_EQ(copyConstructed, source);
    EXPECT_EQ(copyAssigned, source);

    copyConstructed.push_back(4);
    EXPECT_NE(copyConstructed, source);
    EXPECT_EQ(source, (std::list<int>{1, 2, 3}));
}

// Доп. задание 3: перемещающее присваивание переносит содержимое
// в целевой контейнер, а источник остаётся пустым.
TEST(list, move_assignment) {
    std::list<int> source{1, 2, 3};
    std::list<int> target{9};

    target = std::move(source);

    const std::list<int> expected{1, 2, 3};
    EXPECT_EQ(target, expected);
    EXPECT_TRUE(source.empty());
}

// Доп. задание 2: при уничтожении контейнера вызываются деструкторы всех
// хранящихся в нём элементов. Проверяем это по счётчику живых объектов.
TEST(list, destruction) {
    const std::size_t aliveBefore = TrackedType::aliveCount();

    {
        std::list<TrackedType> values;
        values.emplace_back();
        values.emplace_back();
        values.emplace_back();

        // Пока контейнер жив, все три объекта существуют.
        EXPECT_EQ(TrackedType::aliveCount(), aliveBefore + values.size());
    }

    // После выхода из области видимости контейнер уничтожил все элементы.
    EXPECT_EQ(TrackedType::aliveCount(), aliveBefore);
}
