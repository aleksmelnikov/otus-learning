# ============================================================================
# Тестирование контейнеров.
#
# Тесты для std::vector и std::list собираются в один исполняемый файл
# containers_tests. Способ подключения по умолчанию Google Test — через
# FetchContent: CMake сам скачает и соберёт нужную версию библиотеки.
# ============================================================================

cmake_minimum_required(VERSION 3.14)

project(homework_07_containers_tests LANGUAGES CXX)

# Используем возможности C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Загружаем архив Google Test указанной версии.
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.tar.gz
)

# В MSVC статистические библиотеки Google Test должны использовать
# ту же модель CRT, что и тестовое приложение.
if (MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif ()

FetchContent_MakeAvailable(googletest)

# Один тестовый бинарник для обоих контейнеров.
add_executable(containers_tests
    test/vector_tests.cpp
    test/list_tests.cpp
)

# Заголовок tracked_type.hpp лежит рядом с тестами.
target_include_directories(containers_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/test"
)

# gtest_main предоставляет готовую функцию main() для запуска тестов.
target_link_libraries(containers_tests PRIVATE
    gtest
    gtest_main
)

# Включаем предупреждения и трактуем их как ошибки.
if (MSVC)
    target_compile_options(containers_tests PRIVATE /W4)
else ()
    target_compile_options(containers_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()

# Регистрируем тесты в CTest и разрешаем запуск через ctest.
enable_testing()

include(GoogleTest)
gtest_discover_tests(containers_tests)
