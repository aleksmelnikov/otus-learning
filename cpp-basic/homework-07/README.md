# Тестирование контейнеров

Unit-тесты для `std::vector` и `std::list` на Google Test. Все тесты
собираются в один исполняемый файл `containers_tests`.

## Сборка и запуск

Из текущей папки:

```bash
cmake -S . -B build
cmake --build build -j
./build/containers_tests
```

Запуск через CTest:

```bash
ctest --test-dir build --output-on-failure
```

## Полезные варианты запуска

```bash
./build/containers_tests --gtest_filter=vector.*
./build/containers_tests --gtest_filter=list.*
./build/containers_tests --gtest_repeat=10
./build/containers_tests --gtest_output=xml:report.xml
```

## Использование локальных исходников Google Test

По умолчанию CMake скачивает архив Google Test через `FetchContent`.

Если исходники Google Test находятся в локальном каталоге (например,
в `./googletest`), тогда путь к ним можно передать
через `FETCHCONTENT_SOURCE_DIR_GOOGLETEST`, и скачивание не потребуется.

Запуск из текущей папки:

```bash
cmake -S . -B build -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=./googletest
cmake --build build -j
./build/containers_tests
```
