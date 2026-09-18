// ============================================================================
// Последовательный контейнер динамического размера.
//
// SequentialContainer<T> — контейнер, в котором элементы лежат ПОДРЯД в
// памяти (непрерывная область), как в обычном массиве в стиле Си:
//
//      [ 0 ][ 1 ][ 2 ][ 3 ] ... [ n-1 ]
//
// Благодаря этому работает арифметика указателей: взяв указатель на
// i-ый элемент, можно получить указатель на (i+1)-ый простым ++.
//
// ----------------------------------------------------------------------------
// Состояние контейнера — три поля:
//   T*      m_data     — указатель на массив элементов;
//   size_t  m_size     — реальное число сохранённых элементов;
//   size_t  m_capacity — сколько ячеек выделено (может быть больше m_size).
//
// ----------------------------------------------------------------------------
// Резервирование памяти заранее (доп. задание 1):
// при расширении grow() выделяет область в 1.5 раза больше текущей.
// Из-за этого ёмкость растёт по экспоненте, и при серии push_back
// выделение памяти и копирование случаются редко (логарифмически),
// а не на каждое добавление.
//
// ----------------------------------------------------------------------------
// Копирование и перемещение (доп. задание 3):
//   - копирующий конструктор: выделяет новый массив и копирует элементы;
//   - копирующий operator=: идиома copy-and-swap (сначала копия во временный
//     объект, затем обмен полями — если память не выделится, оригинал цел);
//   - перемещающий конструктор: принимает r-value ссылку и «крадёт»
//     указатель у источника, без выделения памяти и копирования;
//   - перемещающий operator=: то же, но предварительно удаляя свой массив;
//   - push_back(T&&), insert(pos, T&&): принимают r-value ссылку на
//     пользовательский объект.
//
// ----------------------------------------------------------------------------
// Итератор (доп. задание 4):
//   class iterator — «умный указатель» на обход контейнера:
//     operator++()  — переход к следующему элементу;
//     operator*()   — разыменование, возвращает T&;
//     get()         — альтернативный способ получения значения, T&;
//   operator== / operator!=  — сравнение итераторов (равно / не равно),
//     нужно для условия цикла «пока итератор не равен end()».
//   begin() возвращает итератор на первый элемент (m_data),
//   end()   — итератор «за последний» (m_data + m_size).
//
// ----------------------------------------------------------------------------
// Основной интерфейс (требования задания):
//   push_back(value)         — добавление элемента в конец;
//   insert(pos, value)       — вставка в произвольную позицию;
//   erase(pos)               — удаление элемента по позиции;
//   size()                   — текущий размер контейнера;
//   operator[]               — доступ по индексу (O(1));
//   at(index)                — то же, но с проверкой границ (исключение);
//   capacity()               — вспомогательный: зарезервированный объём.
// ============================================================================

#include <iostream>
#include <new>         // ::operator new и размещающий new (placement new)
#include <stdexcept>   // std::out_of_range
#include <utility>     // std::move

template <typename T>
class SequentialContainer {

public:
    // Итератор — инкапсулирует логику обхода контейнера.
    // Для последовательного контейнера обход сводится к арифметике
    // указателей, поэтому внутри хранится обычный T* m_ptr.
    class iterator {
    public:
        // Конструктор по умолчанию: пустой итератор, никуда не указывает.
        iterator() : m_ptr(nullptr) {}

        // Конструктор по указателю. explicit запрещает неявное
        // преобразование T* в iterator.
        explicit iterator(T* ptr) : m_ptr(ptr) {}

        // Префиксный ++: переход к следующему элементу (сдвиг вперёд).
        iterator& operator++() { ++m_ptr; return *this; }

        // Постфиксный ++: запоминаем старое значение, сдвигаемся,
        // возвращаем копию старого итератора.
        iterator operator++(int) { iterator tmp(*this); ++m_ptr; return tmp; }

        // Унарный оператор разыменования — возвращает ссылку на элемент.
        T& operator*() const { return *m_ptr; }

        // Альтернативный способ получения значения (доп. задание 4).
        T& get() const { return *m_ptr; }

        // Сравнение итераторов (равно / не равно).
        // Нужно для условия цикла: идти, пока итератор не равен концу.
        bool operator==(const iterator& other) const { return m_ptr == other.m_ptr; }
        bool operator!=(const iterator& other) const { return m_ptr != other.m_ptr; }

    private:
        T* m_ptr;
    };

    // Конструктор по умолчанию: поля инициализированы прямо при
    // объявлении (m_data = nullptr, m_size = 0, m_capacity = 0).
    SequentialContainer() {}

    // Деструктор: сначала разрушаем живые элементы по очереди, затем
    // освобождаем сырую память блока (::operator delete).
    ~SequentialContainer() {
        for (size_t i = 0; i < m_size; ++i) {
            m_data[i].~T();
        }
        ::operator delete(m_data);
    }

    // Копирующий конструктор: выделяем сырую память по размеру копии
    // и в каждой ячейке размещающим new создаём копию элемента.
    // Это не требует конструктора по умолчанию у T.
    SequentialContainer(const SequentialContainer& other) {
        m_size = other.m_size;
        m_capacity = other.m_size;
        m_data = static_cast<T*>(::operator new(m_size * sizeof(T)));
        for (size_t i = 0; i < m_size; ++i) {
            new (m_data + i) T(other.m_data[i]);
        }
    }

    // Копирующее присваивание по идиоме copy-and-swap:
    // сначала создаётся временная копия, затем поля обмениваются.
    SequentialContainer& operator=(const SequentialContainer& other) {
        if (this != &other) {                    // защита от a = a
            SequentialContainer tmp(other);       // копия (fail-safe)
            swap(tmp);                            // обмен полями
        }
        return *this;
    }

    // Перемещающий конструктор: принимает r-value ссылку и «крадёт»
    // указатель у источника. Выделения памяти и копирования нет.
    // Источник обнуляем, чтобы его деструктор не удалил забранную память.
    SequentialContainer(SequentialContainer&& other) {
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    // Перемещающее присваивание: то же самое, но предварительно
    // освобождаем собственную область памяти.
    SequentialContainer& operator=(SequentialContainer&& other) {
        if (this != &other) {
            for (size_t i = 0; i < m_size; ++i) {
                m_data[i].~T();                  // разрушаем свои элементы
            }
            ::operator delete(m_data);           // освобождаем свою память
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    // Добавление элемента в конец: если место закончилось, расширяемся;
    // новый элемент кладётся на индекс m_size (элементы идут подряд с 0).
    void push_back(const T& value) {
        if (m_size == m_capacity) {
            grow();                              // место закончилось
        }
        new (m_data + m_size) T(value);          // создаём в свободной ячейке
        ++m_size;
    }

    // Перегрузка, принимающая r-value ссылку на пользовательский объект
    // (доп. задание 3).
    void push_back(T&& value) {
        if (m_size == m_capacity) {
            grow();
        }
        new (m_data + m_size) T(std::move(value));
        ++m_size;
    }

    // Добавление элемента в начало: по сути это вставка в позицию 0.
    void push_front(const T& value) {
        insert(0, value);
    }

    // Перегрузка, принимающая r-value ссылку на пользовательский объект
    // (доп. задание 3).
    void push_front(T&& value) {
        insert(0, std::move(value));
    }

    // Вставка в произвольную позицию: сначала проверка границ и рост,
    // затем цикл сдвигает хвост на одну ячейку вправо, освобождая место
    // под pos, и в неё записывается новое значение.
    void insert(size_t pos, const T& value) {
        T copy = value;                     // копия ДО возможного grow()
        if (pos > m_size) {
            throw std::out_of_range("insert: position out of range");
        }
        if (m_size == m_capacity) {
            grow();
        }
        for (size_t i = m_size; i > pos; --i) {
            if (i == m_size) {
                // Хвостовая ячейка свободна — переносим в неё последний
                // элемент созданием (placement new).
                new (m_data + i) T(std::move(m_data[i - 1]));
            } else {
                m_data[i] = std::move(m_data[i - 1]);       // сдвиг вправо
            }
        }
        new (m_data + pos) T(std::move(copy));
        ++m_size;
    }

    // Перегрузка, принимающая r-value ссылку на пользовательский объект
    // (доп. задание 3). Сначала забираем значение в локальную переменную,
    // чтобы grow() (при вставке в себя) не испортил источник.
    void insert(size_t pos, T&& value) {
        T tmp(std::move(value));
        if (pos > m_size) {
            throw std::out_of_range("insert: position out of range");
        }
        if (m_size == m_capacity) {
            grow();
        }
        for (size_t i = m_size; i > pos; --i) {
            if (i == m_size) {
                // Хвостовая ячейка свободна — переносим в неё последний
                // элемент созданием (placement new).
                new (m_data + i) T(std::move(m_data[i - 1]));
            } else {
                m_data[i] = std::move(m_data[i - 1]);       // сдвиг вправо
            }
        }
        new (m_data + pos) T(std::move(tmp));
        ++m_size;
    }

    // Удаление элемента по позиции: следующие элементы сдвигаются на
    // ячейку влево (перекрывая удалённый), размер уменьшается, а
    // освободившаяся ячейка уничтожается.
    void erase(size_t pos) {
        if (pos >= m_size) {
            throw std::out_of_range("erase: position out of range");
        }
        for (size_t i = pos; i < m_size - 1; ++i) {
            m_data[i] = std::move(m_data[i + 1]);           // сдвиг влево
        }
        --m_size;
        m_data[m_size].~T();   // уничтожаем объект в освободившейся ячейке
    }

    // size() — число элементов, capacity() — зарезервированный объём.
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }

    // Доступ по индексу — прямая индексация массива, O(1), как у массива
    // Си. Две версии: для неконстантного и константного контейнера.
    T& operator[](size_t index) { return m_data[index]; }
    const T& operator[](size_t index) const { return m_data[index]; }

    // at() — то же, что operator[], но с проверкой границ: при выходе
    // индекса за пределы бросается исключение.
    T& at(size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("at: index out of range");
        }
        return m_data[index];
    }

    // Версия для константного контейнера.
    const T& at(size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("at: index out of range");
        }
        return m_data[index];
    }

    // begin() — на первый элемент (m_data),
    // end() — «за последний» (m_data + m_size) — элемент нельзя читать,
    // это только признак конца цикла.
    iterator begin() { return iterator(m_data); }
    iterator end() { return iterator(m_data + m_size); }

    // Версии для константного контейнера: обход без права изменения.
    const iterator begin() const { return iterator(m_data); }
    const iterator end() const { return iterator(m_data + m_size); }

private:
    // Расширение области памяти (доп. задание 1): выделяем на 50% больше
    // элементов, перемещаем старые, старую область удаляем.
    void grow() {
        size_t new_capacity = (m_capacity == 0)
                                  ? 4
                                  : static_cast<size_t>(m_capacity * 1.5);
        if (new_capacity <= m_size) {
            new_capacity = m_size + 1;
        }
        T* new_data =
            static_cast<T*>(::operator new(new_capacity * sizeof(T)));
        for (size_t i = 0; i < m_size; ++i) {
            new (new_data + i) T(std::move(m_data[i]));  // перемещаем
        }
        for (size_t i = 0; i < m_size; ++i) {
            m_data[i].~T();                      // разрушаем старые элементы
        }
        ::operator delete(m_data);               // освобождаем старую область
        m_data = new_data;
        m_capacity = new_capacity;
    }

    // Меняет местами три поля — используется в copy-and-swap.
    void swap(SequentialContainer& other) {
        T* tmp_data = other.m_data;
        size_t tmp_size = other.m_size;
        size_t tmp_capacity = other.m_capacity;
        other.m_data = m_data;
        other.m_size = m_size;
        other.m_capacity = m_capacity;
        m_data = tmp_data;
        m_size = tmp_size;
        m_capacity = tmp_capacity;
    }

    // Состояние контейнера. Инициализация при объявлении, поэтому
    // конструктор по умолчанию пустой.
    T* m_data = nullptr;        // указатель на массив элементов
    size_t m_size = 0;          // сколько элементов лежит
    size_t m_capacity = 0;      // сколько ячеек выделено
};

// ============================================================================
// Демонстрация возможностей контейнера (вызывается из main).
// ============================================================================

// Вывод содержимого контейнера через итератор: проходим begin()..end()
// и берём значение методом get() (можно было бы и operator*).
template <typename T>
void print(const char* label, SequentialContainer<T>& values) {
    std::cout << label;
    bool first = true;
    for (typename SequentialContainer<T>::iterator it = values.begin();
         it != values.end(); ++it) {
        if (!first) {
            std::cout << ", ";
        }
        std::cout << it.get();
        first = false;
    }
    std::cout << std::endl;
}

// Обязательный набор действий из задания (шаги 1..12):
//   - создать контейнер для int;
//   - добавить 10 элементов (0, 1 ... 9);
//   - вывести содержимое и размер;
//   - удалить 3-й, 5-й и 7-й элементы;
//   - добавить 10 в начало, 20 в середину, 30 в конец.
// Три erase выполняются в обратном порядке (индексы 6, 4, 2), чтобы
// индексы соответствовали элементам ИСХОДНОГО контейнера (0, 1 ... 9):
// после каждого удаления элементы сдвигаются влево.
template <typename T>
void demo_sequence() {
    SequentialContainer<T> values;

    // 1-2. Заполнение контейнера десятью элементами (0, 1 ... 9).
    for (int i = 0; i < 10; ++i) {
        values.push_back(static_cast<T>(i));
    }

    // 3. Вывод содержимого контейнера.
    print("initial content: ", values);

    // 4. Вывод размера контейнера.
    std::cout << "size: " << values.size() << std::endl;

    // 5. Удаление третьего, пятого и седьмого (по счёту) элементов.
    values.erase(6); // 7-й элемент
    values.erase(4); // 5-й элемент
    values.erase(2); // 3-й элемент

    // 6. Вывод содержимого контейнера.
    print("after erase: ", values);

    // 7. Добавление элемента 10 в начало контейнера (позиция 0).
    values.insert(0, static_cast<T>(10));

    // 8. Вывод содержимого контейнера.
    print("after insert 10 at begin: ", values);

    // 9. Добавление элемента 20 в середину контейнера
    //    (середина — index = size() / 2).
    values.insert(values.size() / 2, static_cast<T>(20));

    // 10. Вывод содержимого контейнера.
    print("after insert 20 in middle: ", values);

    // 11. Добавление элемента 30 в конец контейнера.
    values.push_back(static_cast<T>(30));

    // 12. Вывод содержимого контейнера.
    print("after push_back 30: ", values);

    // Вспомогательный вывод: зарезервированный объём памяти.
    std::cout << "capacity: " << values.capacity() << std::endl;

    // Демонстрация at() с проверкой границ: доступ в пределах размера
    // работает, выход за границу порождает исключение out_of_range.
    std::cout << "at(0) = " << values.at(0) << std::endl;
    try {
        values.at(99);
        std::cout << "at(99): no exception" << std::endl;
    } catch (const std::out_of_range&) {
        std::cout << "at(99): out_of_range thrown" << std::endl;
    }

    // Демонстрация константных begin()/end(): сравнение хранимых адресов.
    // Если адреса не равны — контейнер непустой, если равны — пустой.
    // В выводе true означает «адреса не равны, контейнер непустой».
    const SequentialContainer<T>& const_view = values;
    std::cout << "const begin()/end(): first = " << *(const_view.begin())
              << ", last = " << const_view[const_view.size() - 1]
              << ", begin() != end(): "
              << std::boolalpha
              << (const_view.begin() != const_view.end())
              << std::endl;
}

// Демонстрация перемещающих конструктора и оператора присваивания,
// а также приёма r-value ссылки на объект (доп. задание 3).
void demo_move() {
    // Приём r-value: 42 попадает в push_back(T&&).
    SequentialContainer<int> src;
    src.push_back(42);

    // Перемещающий конструктор: dst забирает данные у src.
    SequentialContainer<int> dst(std::move(src));
    std::cout << "after move-ctor: dst size = " << dst.size()
              << ", src size = " << src.size() << std::endl;

    // Перемещающее присваивание.
    SequentialContainer<int> other;
    other = std::move(dst);
    std::cout << "after move-assign: " << other[0] << std::endl;

    // Демонстрация приёма r-value ссылки в push_front и insert.
    // other = [42]; push_front(7) -> [7, 42]; insert(1, 9) -> [7, 9, 42].
    other.push_front(7);   // срабатывает push_front(T&&)
    other.insert(1, 9);    // срабатывает insert(pos, T&&)
    print("after rvalue push_front/insert: ", other);
}

// Демонстрация работы с типом БЕЗ конструктора по умолчанию.
    // У Price есть только конструктор от числа, «создаться из ничего»
    // он не умеет.
struct Price {
    long long rubles;
    explicit Price(long long r) : rubles(r) {}
};

// Для печати контейнера с Price используем ту же функцию print(),
// поэтому задаём вывод значения через operator<<.
std::ostream& operator<<(std::ostream& out, const Price& price) {
    return out << price.rubles;
}

void demo_nodefault() {
    SequentialContainer<Price> prices;
    prices.push_back(Price(99));     // Price создан до вставки
    prices.insert(1, Price(120));    // вставка в позицию 1
    prices.push_front(Price(49));    // в начало

    print("no-default type: ", prices);
    std::cout << "first: " << prices[0] << std::endl;
    std::cout << "size: " << prices.size() << std::endl;
}

int main() {
    demo_sequence<int>(); // обязательный набор действий из задания
    demo_move();          // семантика перемещения
    demo_nodefault();     // тип без конструктора по умолчанию
    return 0;
}

