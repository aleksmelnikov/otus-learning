// ============================================================================
// Расчёт статистических характеристик последовательности.
//
// Приложение читает последовательность числовых значений из стандартного
// ввода (до признака EOF: Ctrl+D в Linux, Ctrl+Z и Enter в Windows) и для
// неё считает набор статистических характеристик:
//   - min    — минимальное значение;
//   - max    — максимальное значение;
//   - mean   — арифметическое среднее;
//   - std    — среднеквадратическое отклонение (по всей выборке);
//   - pct90  — 90-й процентиль (задача со звёздочкой);
//   - pct95  — 95-й процентиль (задача со звёздочкой).
//
// Замечание: вывод сохраняется при любом объёме входных данных, включая
// один элемент. Если вход пустой, вывод не выполняется вовсе.
//
// Ключевая идея задания — полиморфизм: каждый вид статистики реализован
// отдельным классом-наследником абстрактного интерфейса IStatistics.
// Для добавления новой характеристики достаточно написать новый класс,
// реализующий update/eval/name, и не трогать цикл чтения в main.
// ============================================================================

#include <algorithm> // std::copy, std::sort
#include <cmath>     // std::sqrt
#include <iostream>
#include <limits>    // std::numeric_limits
#include <vector>

// Базовый интерфейс: контракт, который обязана соблюдать любая статистика.
//   - update(next) — «скормить» очередное значение последовательности;
//   - eval()       — вернуть вычисленный результат;
//   - name()       — имя характеристики для вывода (например, "min").
// Чисто виртуальные функции делают класс абстрактным: объект такого класса
// создать нельзя, использование возможно только через наследников.
class IStatistics {
public:
    virtual ~IStatistics() {}

    virtual void update(double next) = 0;
    virtual double eval() const = 0;
    virtual const char* name() const = 0;
};

// ----------------------------------------------------------------------------
// min: берём за отправную точку «плюс бесконечность», чтобы любое первое
// значение оказалось меньше и стало новым минимумом.
// ----------------------------------------------------------------------------
class Min : public IStatistics {
public:
    Min() : m_min{std::numeric_limits<double>::infinity()} {}

    void update(double next) override {
        if (next < m_min) {
            m_min = next;
        }
    }

    double eval() const override {
        return m_min;
    }

    const char* name() const override {
        return "min";
    }

private:
    double m_min;
};

// ----------------------------------------------------------------------------
// max: зеркально min, отправная точка — «минус бесконечность».
// ----------------------------------------------------------------------------
class Max : public IStatistics {
public:
    Max() : m_max{-std::numeric_limits<double>::infinity()} {}

    void update(double next) override {
        if (next > m_max) {
            m_max = next;
        }
    }

    double eval() const override {
        return m_max;
    }

    const char* name() const override {
        return "max";
    }

private:
    double m_max;
};

// ----------------------------------------------------------------------------
// mean: среднее арифметическое. Не храним сами значения, а накапливаем
// сумму и число элементов, после чего делим одно на другое.
// ----------------------------------------------------------------------------
class Mean : public IStatistics {
public:
    Mean() : m_sum{0.0}, m_count{0} {}

    void update(double next) override {
        m_sum += next;
        ++m_count;
    }

    double eval() const override {
        return m_sum / static_cast<double>(m_count);
    }

    const char* name() const override {
        return "mean";
    }

private:
    double m_sum;
    size_t m_count;
};

// ----------------------------------------------------------------------------
// std: среднеквадратическое отклонение (по всей выборке, делитель n).
// Используем известное соотношение: Dispersia[сумма квадратов / n - mean^2].
// Первый проход не нужен — суммы накапливаются «на лету». Из-за ошибок
// округления маленькая дисперсия может выйти чуть отрицательной, поэтому
// перед извлечением корня ограничиваем её нулём снизу.
// ----------------------------------------------------------------------------
class Std : public IStatistics {
public:
    Std() : m_sum{0.0}, m_square_sum{0.0}, m_count{0} {}

    void update(double next) override {
        m_sum += next;
        m_square_sum += next * next;
        ++m_count;
    }

    double eval() const override {
        const double mean = m_sum / static_cast<double>(m_count);
        const double variance =
            m_square_sum / static_cast<double>(m_count) - mean * mean;
        return std::sqrt(variance > 0.0 ? variance : 0.0);
    }

    const char* name() const override {
        return "std";
    }

private:
    double m_sum;
    double m_square_sum;
    size_t m_count;
};

// ----------------------------------------------------------------------------
// Базовый класс для процентилей. В отличие от остальных характеристик
// посчитать процентиль «на лету» нельзя: нужен отсортированный ряд,
// поэтому храним все поступившие значения в векторе.
//   - update  сохраняет очередное значение;
//   - percentile(q) сортирует копию ряда и берёт элемент по «рангу»:
//     ранг = ceil(q * n) (ближайший ранг), результат — элемент под рангом.
//     Например, для ряда 0..10 (n = 11) при q = 0.95 ранг равен ceil(10.45)
//     = 11, то есть берётся последний элемент — 10.
// Сам класс абстрактный: конкретные 90-й и 95-й процентили задаются
// наследниками (они фиксируют долю q и имя для вывода).
// ----------------------------------------------------------------------------
class Percentile : public IStatistics {
public:
    void update(double next) override {
        m_values.push_back(next);
    }

protected:
    // Возвращает значение процентиля для доли q (от 0 до 1).
    double percentile(double q) const {
        if (m_values.empty()) {
            return 0.0;
        }

        // Работаем с копией, чтобы не менять исходный порядок данных.
        std::vector<double> sorted = m_values;
        std::sort(sorted.begin(), sorted.end());

        // Ранг считается с единицы (первый элемент отсортированного ряда).
        const size_t count = sorted.size();
        const size_t rank = static_cast<size_t>(std::ceil(q * static_cast<double>(count)));

        // Ранг не должен выходить за пределы ряда.
        const size_t index = (rank > 0 ? rank : 1) - 1;
        return sorted[index < count ? index : count - 1];
    }

private:
    std::vector<double> m_values;
};

// ----------------------------------------------------------------------------
// 90-й процентиль: «90% значений ряда не превосходят этого числа».
// ----------------------------------------------------------------------------
class Pct90 : public Percentile {
public:
    double eval() const override {
        return percentile(0.90);
    }

    const char* name() const override {
        return "pct90";
    }
};

// ----------------------------------------------------------------------------
// 95-й процентиль: «95% значений ряда не превосходят этого числа».
// ----------------------------------------------------------------------------
class Pct95 : public Percentile {
public:
    double eval() const override {
        return percentile(0.95);
    }

    const char* name() const override {
        return "pct95";
    }
};

int main() {
    // Набор рассчитываемых характеристик. Полиморфизм: все классы разные,
    // но обращаться с ними можно одинаково — как с IStatistics.
    const size_t statistics_count = 6;
    IStatistics* statistics[statistics_count] = {
        new Min{},
        new Max{},
        new Mean{},
        new Std{},
        new Pct90{},
        new Pct95{},
    };

    // Читаем значения до конца файла (EOF). Каждое переданное значение
    // «скармливаем» всем статистикам одновременно.
    double value = 0;
    size_t value_count = 0;
    while (std::cin >> value) {
        ++value_count;
        for (size_t i = 0; i < statistics_count; ++i) {
            statistics[i]->update(value);
        }
    }

    // Признак ошибки ввода: данные не закончились (EOF не встречен),
    // но чтение прекратилось — значит, во входе встретился не числовой токен.
    if (!std::cin.eof() && !std::cin.good()) {
        std::cerr << "Invalid input data" << std::endl;
        for (size_t i = 0; i < statistics_count; ++i) {
            delete statistics[i];
        }
        return 1;
    }

    // Печатаем результаты, только если последовательность не пустая
    // (пустому входу статистические характеристики не отвечают).
    if (value_count > 0) {
        for (size_t i = 0; i < statistics_count; ++i) {
            std::cout << statistics[i]->name() << " = "
                      << statistics[i]->eval() << std::endl;
        }
    }

    // Освобождаем память созданных объектов.
    for (size_t i = 0; i < statistics_count; ++i) {
        delete statistics[i];
    }

    return 0;
}
