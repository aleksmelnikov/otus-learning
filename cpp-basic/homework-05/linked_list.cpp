// ============================================================================
// Контейнеры спискового типа.
//
// В отличие от последовательного контейнера элементы здесь НЕ лежат подряд
// в памяти. Каждый элемент хранится в отдельной структурке — узле (Node),
// а связь между узлами организуется через указатели. Из-за этого:
//   - арифметика указателей для доступа к элементам невозможна;
//   - зато вставка и удаление узлов дёшевы (нужно лишь переставить
//     несколько указателей);
//   - доступ по индексу — обход по цепочке указателей, поэтому O(n).
//
// ----------------------------------------------------------------------------
// Два варианта (доп. задание 2):
//
//   1) DoublyLinkedList<T> — двунаправленный список.
//      Каждый узел знает следующий И предыдущий:
//
//          Node { Node* prev; Node* next; T data; }
//
//          m_head -> [data|->] <-> [data|->] <-> ... <-> [data] <- m_tail
//
//      push_back / push_front — O(1) (есть голова и хвост).
//      insert / erase / operator[] — O(n): идём по цепочке до индекса.
//
//   2) SinglyLinkedList<T> — однонаправленный список.
//      Каждый узел знает ТОЛЬКО следующий:
//
//          Node { Node* next; T data; }
//
//          m_head -> [data|->] -> [data|->] -> ... -> [data] -> nullptr <- m_tail
//
//      Отличия от двунаправленного:
//        - нет указателя prev;
//        - при insert/erase  приходится идти до узла pos-1 (на один шаг
//          дальше), чтобы переставить его next;
//        - push_back остаётся O(1), потому что хвост m_tail сохраняется.
//
// ----------------------------------------------------------------------------
// Состояние каждого списка — три поля:
//   Node*  m_head — указатель на первый узел (пусто — nullptr);
//   Node*  m_tail — указатель на последний узел (удобно для push_back);
//   size_t m_size — число узлов.
//
// ----------------------------------------------------------------------------
// Итератор (доп. задание 4):
//   вложенный класс iterator хранит указатель на узел. «Переход к следующему»
//   — это переход по указателю next внутри узла. Методы:
//     operator++()  — переход к следующему элементу;
//     operator*()   — разыменование, возвращает T&;
//     get()         — альтернативный способ получения значения, T&;
//     равно / не равно — сравнение для условия цикла.
//   begin() возвращает итератор на m_head,
//   end()   — итератор на nullptr-узел («за последним»).
//   Итератор вложенный, поэтому он может работать с приватной структурой Node.
//
// ----------------------------------------------------------------------------
// Копирование и перемещение (доп. задание 3):
//   - копирующий конструктор: обходит исходный список и делает push_back;
//   - копирующий operator=: идиома copy-and-swap;
//   - перемещающий конструктор: «крадёт» узлы у источника, не копируя их;
//   - перемещающий operator=: удаляет свои узлы и забирает чужие;
//   - push_back/... принимают r-value ссылку на пользовательский объект.
//
// ----------------------------------------------------------------------------
// Основной интерфейс (требования задания):
//   push_back(value)    — добавление элемента в конец (O(1));
//   push_front(value)   — добавление элемента в начало (O(1));
//   insert(pos, value)  — вставка в произвольную позицию (O(n));
//   erase(pos)          — удаление элемента по позиции (O(n));
//   size()              — текущий размер контейнера;
//   operator[]          — доступ по индексу (O(n), проход по узлам);
//   at(index)           — то же, но с проверкой границ (исключение);
//   begin() / end()     — итераторы для обхода.
// ============================================================================

#include <cstddef>
#include <iostream>
#include <stdexcept>   // std::out_of_range
#include <utility>     // std::move

// ============================================================================
// Двунаправленный список: каждый узел хранит указатель на следующий
// и на предыдущий элемент.
// ============================================================================
template <typename T>
class DoublyLinkedList {

    // Узел списка: пользовательские данные плюс связка с соседями.
    struct Node {
        Node* prev; // указатель на предыдущий узел (nullptr у первого)
        Node* next; // указатель на следующий узел (nullptr у последнего)
        T data;     // пользовательские данные
    };

public:
    // Итератор, инкапсулирующий логику обхода. Хранит указатель на узел;
    // переход к следующему элементу — это переход по указателю next.
    class iterator {
    public:
        // Конструктор по умолчанию: пустой итератор.
        iterator() : m_node(nullptr) {}

        // Конструктор по указателю на узел.
        explicit iterator(Node* node) : m_node(node) {}

        // Префиксный ++: переходим к следующему узлу.
        iterator& operator++() { m_node = m_node->next; return *this; }

        // Постфиксный ++: запоминаем старый итератор, потом сдвигаемся.
        iterator operator++(int) { iterator tmp(*this); m_node = m_node->next; return tmp; }

        // Разыменование — возвращаем ссылку на хранимый объект.
        T& operator*() const { return m_node->data; }

        // Альтернативный способ получения значения (доп. задание 4).
        T& get() const { return m_node->data; }

        // Сравнение итераторов (равно / не равно) — для условия цикла.
        bool operator==(const iterator& other) const { return m_node == other.m_node; }
        bool operator!=(const iterator& other) const { return m_node != other.m_node; }

    private:
        Node* m_node;
    };

    // Конструктор по умолчанию: пустой список, голова и хвост отсутствуют.
    DoublyLinkedList() : m_head(nullptr), m_tail(nullptr), m_size(0) {}

    // Деструктор: удаляем все узлы по цепочке от головы к хвосту.
    ~DoublyLinkedList() {
        Node* cur = m_head;
        while (cur != nullptr) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
    }

    // Копирующий конструктор: обходим исходный список и последовательно
    // добавляем каждый элемент в новый.
    DoublyLinkedList(const DoublyLinkedList& other) : m_head(nullptr), m_tail(nullptr), m_size(0) {
        Node* cur = other.m_head;
        while (cur != nullptr) {
            push_back(cur->data);
            cur = cur->next;
        }
    }

    // Копирующее присваивание по идиоме copy-and-swap.
    DoublyLinkedList& operator=(const DoublyLinkedList& other) {
        if (this != &other) {                    // защита от a = a
            DoublyLinkedList tmp(other);          // копия (fail-safe)
            swap(tmp);                            // обмен полями
        }
        return *this;
    }

    // Перемещающий конструктор: «крадём» узлы у источника (r-value),
    // копирования нет. Источник обнуляем, чтобы его деструктор ничего
    // не удалил.
    DoublyLinkedList(DoublyLinkedList&& other)
        : m_head(other.m_head), m_tail(other.m_tail), m_size(other.m_size) {
        other.m_head = nullptr;
        other.m_tail = nullptr;
        other.m_size = 0;
    }

    // Перемещающее присваивание: сначала удаляем свои узлы,
    // затем забираем узлы источника.
    DoublyLinkedList& operator=(DoublyLinkedList&& other) {
        if (this != &other) {
            Node* cur = m_head;
            while (cur != nullptr) {
                Node* next = cur->next;
                delete cur;
                cur = next;
            }
            m_head = other.m_head;
            m_tail = other.m_tail;
            m_size = other.m_size;
            other.m_head = nullptr;
            other.m_tail = nullptr;
            other.m_size = 0;
        }
        return *this;
    }

    // Добавление элемента в конец. O(1): достаточно переставить указатели
    // у старого хвоста и обновить m_tail.
    void push_back(const T& value) {
        Node* node = new Node{ m_tail, nullptr, value };
        if (m_tail != nullptr) {
            m_tail->next = node;   // старый хвост указывает на новый узел
        } else {
            m_head = node;         // список был пуст — узел становится головой
        }
        m_tail = node;             // новый узел становится хвостом
        ++m_size;
    }

    // Перегрузка для r-value ссылки (доп. задание 3).
    void push_back(T&& value) {
        Node* node = new Node{ m_tail, nullptr, std::move(value) };
        if (m_tail != nullptr) {
            m_tail->next = node;
        } else {
            m_head = node;
        }
        m_tail = node;
        ++m_size;
    }

    // Добавление элемента в начало. O(1): аналогично push_back,
    // но работаем с головой.
    void push_front(const T& value) {
        Node* node = new Node{ nullptr, m_head, value };
        if (m_head != nullptr) {
            m_head->prev = node;   // старый первый узел — предок новый
        } else {
            m_tail = node;         // список был пуст — узел является хвостом
        }
        m_head = node;             // новый узел становится головой
        ++m_size;
    }

    // Перегрузка для r-value ссылки (доп. задание 3).
    void push_front(T&& value) {
        Node* node = new Node{ nullptr, m_head, std::move(value) };
        if (m_head != nullptr) {
            m_head->prev = node;   // старый первый узел — предок новый
        } else {
            m_tail = node;         // список был пуст — узел является хвостом
        }
        m_head = node;             // новый узел становится головой
        ++m_size;
    }

    // Вставка в произвольную позицию. O(n): доходим до узла, стоящего
    // в позиции pos, и вплетаем новый узел перед ним.
    void insert(size_t pos, const T& value) {
        if (pos > m_size) {
            throw std::out_of_range("insert: position out of range");
        }
        if (pos == m_size) {       // вставка после последнего — это push_back
            push_back(value);
            return;
        }
        if (pos == 0) {            // вставка перед первым — это push_front
            push_front(value);
            return;
        }
        // Находим узел, перед которым вставляем новый.
        Node* cur = m_head;
        for (size_t i = 0; i < pos; ++i) {
            cur = cur->next;
        }
        Node* node = new Node{ cur->prev, cur, value };
        cur->prev->next = node;    // предок узла cur теперь указывает на новый
        cur->prev = node;          // новый узел становится предком cur
        ++m_size;
    }

    // Перегрузка, принимающая r-value ссылку на пользовательский объект
    // (доп. задание 3).
    void insert(size_t pos, T&& value) {
        if (pos > m_size) {
            throw std::out_of_range("insert: position out of range");
        }
        if (pos == m_size) {       // вставка после последнего — это push_back
            push_back(std::move(value));
            return;
        }
        if (pos == 0) {            // вставка перед первым — это push_front
            push_front(std::move(value));
            return;
        }
        // Находим узел, перед которым вставляем новый.
        Node* cur = m_head;
        for (size_t i = 0; i < pos; ++i) {
            cur = cur->next;
        }
        Node* node = new Node{ cur->prev, cur, std::move(value) };
        cur->prev->next = node;    // предок узла cur теперь указывает на новый
        cur->prev = node;          // новый узел становится предком cur
        ++m_size;
    }

    // Удаление элемента по позиции. O(n): доходим до узла и «отвязываем» его:
    // соседи начинают указывать друг на друга, узел удаляется.
    void erase(size_t pos) {
        if (pos >= m_size) {
            throw std::out_of_range("erase: position out of range");
        }
        Node* cur = m_head;
        for (size_t i = 0; i < pos; ++i) {
            cur = cur->next;
        }
        if (cur->prev != nullptr) {
            cur->prev->next = cur->next;   // предок смотрит на потомка
        } else {
            m_head = cur->next;            // удаляли голову — сдвигаем голову
        }
        if (cur->next != nullptr) {
            cur->next->prev = cur->prev;   // потомок смотрит на предка
        } else {
            m_tail = cur->prev;            // удаляли хвост — сдвигаем хвост
        }
        delete cur;
        --m_size;
    }

    size_t size() const { return m_size; }

    // Доступ по индексу: O(n), проход по цепочке от головы.
    T& operator[](size_t index) {
        Node* cur = m_head;
        for (size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur->data;
    }

    // Версия для константного контейнера.
    const T& operator[](size_t index) const {
        Node* cur = m_head;
        for (size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur->data;
    }

    // at() — то же, что operator[], но с проверкой границ: при выходе
    // индекса за пределы списка бросается исключение.
    T& at(size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("at: index out of range");
        }
        Node* cur = m_head;
        for (size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur->data;
    }

    // Версия для константного контейнера.
    const T& at(size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("at: index out of range");
        }
        Node* cur = m_head;
        for (size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur->data;
    }

    // begin() — итератор на первый узел,
    // end() — итератор «за последним» (nullptr-узел), читать нельзя.
    iterator begin() { return iterator(m_head); }
    iterator end() { return iterator(nullptr); }

    // Версии для константного контейнера: обход без права изменения.
    const iterator begin() const { return iterator(m_head); }
    const iterator end() const { return iterator(nullptr); }

private:
    // Меняет местами три поля — используется в copy-and-swap.
    void swap(DoublyLinkedList& other) {
        Node* tmp_head = other.m_head;
        Node* tmp_tail = other.m_tail;
        size_t tmp_size = other.m_size;
        other.m_head = m_head;
        other.m_tail = m_tail;
        other.m_size = m_size;
        m_head = tmp_head;
        m_tail = tmp_tail;
        m_size = tmp_size;
    }

    Node* m_head;   // первый узел
    Node* m_tail;   // последний узел
    size_t m_size;  // число узлов
};

// ============================================================================
// Однонаправленный список: каждый узел хранит указатель только
// на следующий элемент.
// ============================================================================
template <typename T>
class SinglyLinkedList {

    // Узел: данные и указатель только на следующий узел.
    struct Node {
        Node* next; // указатель на следующий узел (nullptr у последнего)
        T data;     // пользовательские данные
    };

public:
    // Итератор — как в двунаправленном списке: переход вперёд по next.
    class iterator {
    public:
        // Конструктор по умолчанию: пустой итератор.
        iterator() : m_node(nullptr) {}

        // Конструктор по указателю на узел.
        explicit iterator(Node* node) : m_node(node) {}

        // Префиксный ++: переходим к следующему узлу.
        iterator& operator++() { m_node = m_node->next; return *this; }

        // Постфиксный ++: запоминаем старый итератор, потом сдвигаемся.
        iterator operator++(int) { iterator tmp(*this); m_node = m_node->next; return tmp; }

        // Разыменование — возвращаем ссылку на хранимый объект.
        T& operator*() const { return m_node->data; }

        // Альтернативный способ получения значения (доп. задание 4).
        T& get() const { return m_node->data; }

        // Сравнение итераторов (равно / не равно) — для условия цикла.
        bool operator==(const iterator& other) const { return m_node == other.m_node; }
        bool operator!=(const iterator& other) const { return m_node != other.m_node; }

    private:
        Node* m_node;
    };

    // Конструктор по умолчанию: пустой список.
    SinglyLinkedList() : m_head(nullptr), m_tail(nullptr), m_size(0) {}

    // Деструктор: удаляем все узлы по цепочке от головы.
    ~SinglyLinkedList() {
        Node* cur = m_head;
        while (cur != nullptr) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
    }

    // Копирующий конструктор: обходим исходный список и добавляем элементы.
    SinglyLinkedList(const SinglyLinkedList& other) : m_head(nullptr), m_tail(nullptr), m_size(0) {
        Node* cur = other.m_head;
        while (cur != nullptr) {
            push_back(cur->data);
            cur = cur->next;
        }
    }

    // Копирующее присваивание по идиоме copy-and-swap.
    SinglyLinkedList& operator=(const SinglyLinkedList& other) {
        if (this != &other) {                    // защита от a = a
            SinglyLinkedList tmp(other);          // копия (fail-safe)
            swap(tmp);                            // обмен полями
        }
        return *this;
    }

    // Перемещающий конструктор: «крадём» узлы у источника.
    SinglyLinkedList(SinglyLinkedList&& other)
        : m_head(other.m_head), m_tail(other.m_tail), m_size(other.m_size) {
        other.m_head = nullptr;
        other.m_tail = nullptr;
        other.m_size = 0;
    }

    // Перемещающее присваивание: удаляем свои узлы, забираем чужие.
    SinglyLinkedList& operator=(SinglyLinkedList&& other) {
        if (this != &other) {
            Node* cur = m_head;
            while (cur != nullptr) {
                Node* next = cur->next;
                delete cur;
                cur = next;
            }
            m_head = other.m_head;
            m_tail = other.m_tail;
            m_size = other.m_size;
            other.m_head = nullptr;
            other.m_tail = nullptr;
            other.m_size = 0;
        }
        return *this;
    }

    // Добавление в конец O(1): новый узел встаёт после хвоста, m_tail
    // обновляется. Хвост хранится специально ради этой операции.
    void push_back(const T& value) {
        Node* node = new Node{ nullptr, value };
        if (m_tail != nullptr) {
            m_tail->next = node;   // старый хвост указывает на новый узел
        } else {
            m_head = node;         // список был пуст — узел является головой
        }
        m_tail = node;             // новый узел становится хвостом
        ++m_size;
    }

    // Перегрузка для r-value ссылки (доп. задание 3).
    void push_back(T&& value) {
        Node* node = new Node{ nullptr, std::move(value) };
        if (m_tail != nullptr) {
            m_tail->next = node;
        } else {
            m_head = node;
        }
        m_tail = node;
        ++m_size;
    }

    // Добавление в начало O(1): новый узел встаёт перед головой, m_head
    // обновляется.
    void push_front(const T& value) {
        Node* node = new Node{ m_head, value };
        if (m_head == nullptr) {
            m_tail = node;         // список был пуст — узел является хвостом
        }
        m_head = node;             // новый узел становится головой
        ++m_size;
    }

    // Перегрузка для r-value ссылки (доп. задание 3).
    void push_front(T&& value) {
        Node* node = new Node{ m_head, std::move(value) };
        if (m_head == nullptr) {
            m_tail = node;         // список был пуст — узел является хвостом
        }
        m_head = node;             // новый узел становится головой
        ++m_size;
    }

    // Вставка в произвольную позицию O(n). Так как указателя prev нет,
    // идём до узла pos-1 и вставляем новый сразу за ним.
    void insert(size_t pos, const T& value) {
        if (pos > m_size) {
            throw std::out_of_range("insert: position out of range");
        }
        if (pos == m_size) {       // вставка после последнего — это push_back
            push_back(value);
            return;
        }
        if (pos == 0) {            // вставка перед первым — это push_front
            push_front(value);
            return;
        }
        // Находим узел, после которого вставляем новый.
        Node* cur = m_head;
        for (size_t i = 0; i < pos - 1; ++i) {
            cur = cur->next;
        }
        Node* node = new Node{ cur->next, value };    
        cur->next = node;          // предыдущий узел теперь указывает на новый
        ++m_size;
    }

    // Перегрузка, принимающая r-value ссылку на пользовательский объект
    // (доп. задание 3).
    void insert(size_t pos, T&& value) {
        if (pos > m_size) {
            throw std::out_of_range("insert: position out of range");
        }
        if (pos == m_size) {       // вставка после последнего — это push_back
            push_back(std::move(value));
            return;
        }
        if (pos == 0) {            // вставка перед первым — это push_front
            push_front(std::move(value));
            return;
        }
        // Находим узел, после которого вставляем новый.
        Node* cur = m_head;
        for (size_t i = 0; i < pos - 1; ++i) {
            cur = cur->next;
        }
        Node* node = new Node{ cur->next, std::move(value) };
        cur->next = node;          // предыдущий узел теперь указывает на новый
        ++m_size;
    }

    // Удаление элемента по позиции O(n). Если удаляется голова — просто
    // сдвигаем m_head, иначе идём до узла pos-1 и переставляем его next.
    void erase(size_t pos) {
        if (pos >= m_size) {
            throw std::out_of_range("erase: position out of range");
        }
        Node* to_delete;
        if (pos == 0) {
            to_delete = m_head;
            m_head = m_head->next;
            if (m_head == nullptr) {
                m_tail = nullptr;  // список опустел — хвоста нет
            }
        } else {
            // Находим узел перед удаляемым.
            Node* cur = m_head;
            for (size_t i = 0; i < pos - 1; ++i) {
                cur = cur->next;
            }
            to_delete = cur->next;
            cur->next = to_delete->next;   // обходим удаляемый узел
            if (to_delete == m_tail) {
                m_tail = cur;              // удалили хвост — хвост сдвигаем
            }
        }
        delete to_delete;
        --m_size;
    }

    size_t size() const { return m_size; }

    // Доступ по индексу: O(n), проход по цепочке от головы.
    T& operator[](size_t index) {
        Node* cur = m_head;
        for (size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur->data;
    }

    // Версия для константного контейнера.
    const T& operator[](size_t index) const {
        Node* cur = m_head;
        for (size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur->data;
    }

    // at() — то же, что operator[], но с проверкой границ: при выходе
    // индекса за пределы списка бросается исключение.
    T& at(size_t index) {
        if (index >= m_size) {
            throw std::out_of_range("at: index out of range");
        }
        Node* cur = m_head;
        for (size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur->data;
    }

    // Версия для константного контейнера.
    const T& at(size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range("at: index out of range");
        }
        Node* cur = m_head;
        for (size_t i = 0; i < index; ++i) {
            cur = cur->next;
        }
        return cur->data;
    }

    // begin() — итератор на первый узел,
    // end() — итератор «за последним» (nullptr-узел).
    iterator begin() { return iterator(m_head); }
    iterator end() { return iterator(nullptr); }

    // Версии для константного контейнера: обход без права изменения.
    const iterator begin() const { return iterator(m_head); }
    const iterator end() const { return iterator(nullptr); }

private:
    // Меняет местами три поля — используется в copy-and-swap.
    void swap(SinglyLinkedList& other) {
        Node* tmp_head = other.m_head;
        Node* tmp_tail = other.m_tail;
        size_t tmp_size = other.m_size;
        other.m_head = m_head;
        other.m_tail = m_tail;
        other.m_size = m_size;
        m_head = tmp_head;
        m_tail = tmp_tail;
        m_size = tmp_size;
    }

    Node* m_head;   // первый узел
    Node* m_tail;   // последний узел
    size_t m_size;  // число узлов
};

// ============================================================================
// Демонстрация возможностей контейнеров (вызывается из main).
// ============================================================================

// Вывод содержимого контейнера через итератор: begin()..end(),
// значение берём методом get() (можно и operator*).
// Шаблонная функция Container работает и с двунаправленным,
// и с однонаправленным списками.
template <typename Container>
void print(const char* label, Container& values) {
    std::cout << label;
    bool first = true;
    for (typename Container::iterator it = values.begin();
         it != values.end(); ++it) {
        if (!first) {
            std::cout << ", ";
        }
        std::cout << it.get();
        first = false;
    }
    std::cout << std::endl;
}

// Обязательный набор действий из задания (шаги 1..12) для любого списка:
//   - создать контейнер для int;
//   - добавить 10 элементов (0, 1 ... 9);
//   - вывести содержимое и размер;
//   - удалить 3-й, 5-й и 7-й элементы;
//   - добавить 10 в начало, 20 в середину, 30 в конец.
// Три erase выполняются в обратном порядке (индексы 6, 4, 2), чтобы
// индексы соответствовали элементам ИСХОДНОГО контейнера: после каждого
// удаления элементы сдвигаются влево.
template <typename Container>
void demo(const char* name, const char* one_way) {
    std::cout << name << " (" << one_way << "):" << std::endl;
    Container values;

    // 1-2. Заполнение контейнера десятью элементами (0, 1 ... 9).
    for (int i = 0; i < 10; ++i) {
        values.push_back(i);
    }

    // 3. Вывод содержимого контейнера.
    print("  initial content: ", values);

    // 4. Вывод размера контейнера.
    std::cout << "  size: " << values.size() << std::endl;

    // 5. Удаление третьего, пятого и седьмого (по счёту) элементов.
    values.erase(6); // 7-й элемент
    values.erase(4); // 5-й элемент
    values.erase(2); // 3-й элемент

    // 6. Вывод содержимого контейнера.
    print("  after erase: ", values);

    // 7. Добавление элемента 10 в начало контейнера (позиция 0).
    values.insert(0, 10);

    // 8. Вывод содержимого контейнера.
    print("  after insert 10 at begin: ", values);

    // 9. Добавление элемента 20 в середину контейнера
    //    (середина — index = size() / 2).
    values.insert(values.size() / 2, 20);

    // 10. Вывод содержимого контейнера.
    print("  after insert 20 in middle: ", values);

    // 11. Добавление элемента 30 в конец контейнера.
    values.push_back(30);

    // 12. Вывод содержимого контейнера.
    print("  after push_back 30: ", values);

    // Демонстрация перемещающего конструктора: источник становится пустым.
    Container src;
    src.push_back(100);
    src.push_back(200);
    Container dst(std::move(src));
    std::cout << "  after move-ctor: dst size = " << dst.size()
          << ", src size = " << src.size() << std::endl;

    // Демонстрация перемещающего присваивания.
    Container other;
    other = std::move(dst);
    std::cout << "  after move-assign: " << other[0] << ", " << other[1] << std::endl;

    // Демонстрация приёма r-value ссылки в push_front и insert.
    // other = [100, 200]; push_front(0) -> [0, 100, 200];
    // insert(1, 9) -> [0, 9, 100, 200].
    other.push_front(0);   // срабатывает push_front(T&&)
    other.insert(1, 9);    // срабатывает insert(pos, T&&)
    print("  after rvalue push_front/insert: ", other);

    // Демонстрация at() с проверкой границ: первый и последний элементы
    // доступны, запрос за пределы списка бросает исключение out_of_range.
    std::cout << "  at(0) = " << other.at(0)
              << ", at(last) = " << other.at(other.size() - 1) << std::endl;
    try {
        other.at(other.size());
        std::cout << "  at(size()): no exception" << std::endl;
    } catch (const std::out_of_range&) {
        std::cout << "  at(size()): out_of_range thrown" << std::endl;
    }

    // Демонстрация константных begin()/end(): сравнение хранимых адресов.
    // Если адреса не равны — контейнер непустой, если равны — пустой.
    // В выводе true означает «адреса не равны, контейнер непустой».
    const Container& const_view = other;
    std::cout << "  const begin()/end(): first = " << *(const_view.begin())
              << ", last = " << const_view[const_view.size() - 1]
              << ", begin() != end(): "
              << std::boolalpha
              << (const_view.begin() != const_view.end())
              << std::endl;
}

int main() {
    // Доп. задание 2: демонстрируем оба варианта спискового контейнера.
    demo<DoublyLinkedList<int> >("DoublyLinkedList", "has prev and next");
    demo<SinglyLinkedList<int> >("SinglyLinkedList", "only next");
    return 0;
}
