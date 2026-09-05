#pragma once

#include "memory.hpp"

#include <cstdio>
#include <cstring>
#include <exception>
#include <utility>
#if defined(__unix__)
#include <sys/resource.h>
#endif

namespace lab_test {
struct Failure {
    const char* expression;
    const char* file;
    int line;
};

struct Test {
    const char* name;
    void (*run)();
    Test* next;
    static inline Test* first = nullptr;
    Test(const char* n, void (*f)()) : name(n), run(f), next(first) { first = this; }
};

struct Tracked {
    static inline int alive = 0;
    static inline int destroyed = 0;
    int value;
    explicit Tracked(int v = 0) : value(v) { ++alive; }
    Tracked(const Tracked&) = delete;
    Tracked(Tracked&&) = delete;
    ~Tracked() { --alive; ++destroyed; }
};
} // namespace lab_test / 命名空间 lab_test

// Not assert(): checks remain active even if a student defines NDEBUG.
// 不使用 assert()：即使学生定义 NDEBUG，检查仍然有效。
#define CHECK(...) do { if (!(__VA_ARGS__)) \
    throw lab_test::Failure{#__VA_ARGS__, __FILE__, __LINE__}; } while (false)
#define TEST(name) static void name(); \
    static lab_test::Test register_##name(#name, &name); static void name()

int main(int argc, char** argv) {
#if defined(__unix__)
    const rlimit no_core{0, 0};
    setrlimit(RLIMIT_CORE, &no_core);
#endif
    if (argc != 2) {
        std::fprintf(stderr, "Expected a case name or --list\n");
        return 2;
    }
    if (std::strcmp(argv[1], "--list") == 0) {
        for (auto* t = lab_test::Test::first; t; t = t->next) std::puts(t->name);
        return 0;
    }
    for (auto* t = lab_test::Test::first; t; t = t->next) {
        if (std::strcmp(argv[1], t->name) != 0) continue;
        const auto before = lab_memory::outstanding();
        bool passed = true;
        try {
            t->run();
        } catch (const lab_test::Failure& error) {
            std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n",
                         error.file, error.line, error.expression);
            passed = false;
        } catch (const std::exception& error) {
            std::fprintf(stderr, "Unexpected exception: %s\n", error.what());
            passed = false;
        } catch (...) {
            std::fprintf(stderr, "Unexpected non-standard exception\n");
            passed = false;
        }
        const auto delta = lab_memory::outstanding() - before;
        if (delta != 0) {
            std::fprintf(stderr, "Allocation imbalance: %+ld live C++ allocation(s)\n", delta);
            passed = false;
        }
        if (!passed) return 1;
        std::printf("PASS %s\n", t->name);
        return 0;
    }
    std::fprintf(stderr, "Unknown case: %s\n", argv[1]);
    return 2;
}
