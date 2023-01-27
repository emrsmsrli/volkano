/*
 * Copyright (C) 2020  emrsmsrli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <exception>

#include <doctest/doctest.h>

#define VKE_ASSERT_MSG(predicate, ...) CHECK(predicate)
#include "core/container/static_vector.h"

namespace {

template<typename T>
using svector = volkano::static_vector<T, 1>;

struct move_only {
    move_only() = default;
    move_only(move_only&&) noexcept = default;
    move_only(const move_only&) = delete;
    move_only& operator=(move_only&&) noexcept = default;
    move_only& operator=(const move_only&) = delete;
};

struct lifetime {
    enum class status { none, moved_from, moved_to, copied_from, copied_to, destructed };

    status status{status::none};

    lifetime() = default;
    ~lifetime() { status = status::destructed; }

    lifetime(lifetime &&other) noexcept {
        other.status = status::moved_from;
        status = status::moved_to;
    };

    lifetime(const lifetime &other) {
        const_cast<lifetime &>(other).status = status::copied_from;
        status = status::copied_to;
    };

    lifetime &operator=(lifetime &&other) noexcept {
        other.status = status::moved_from;
        status = status::moved_to;
        return *this;
    };

    lifetime &operator=(const lifetime &other) {
        const_cast<lifetime &>(other).status = status::copied_from;
        status = status::copied_to;
        return *this;
    };
};

static_assert(noexcept(std::declval<svector<move_only>>()));

TEST_CASE("static vector - size") {
    svector<int> v;
    REQUIRE(v.empty());
    REQUIRE(v.size() == 0);

    v.push_back(0);
    REQUIRE(!v.empty());
    REQUIRE(v.size() == 1);

    v.clear();
    REQUIRE(v.empty());
    REQUIRE(v.size() == 0);
}

TEST_CASE("static vector - push more than capacity" * doctest::should_fail()) {
    svector<int> v;
    v.push_back(1);
    v.push_back(1); // should step into debugger then terminate
}

TEST_CASE("static vector - copy/move") {
    svector<move_only> v;
    v.emplace_back();

    // auto v2 = v; should not compile
    auto v2 = std::move(v);

    svector<lifetime> mc1;
    mc1.emplace_back();
    REQUIRE(mc1[0].status == lifetime::status::none);

    auto mc2 = mc1;
    REQUIRE(mc1[0].status == lifetime::status::copied_from);
    REQUIRE(mc2[0].status == lifetime::status::copied_to);

    auto mc3 = std::move(mc1);
    REQUIRE(mc1[0].status == lifetime::status::moved_from); // UB but it's OK for tests
    REQUIRE(mc3[0].status == lifetime::status::moved_to);
}

TEST_CASE("static_vector - destruct") {
    svector<lifetime> l;
    l.emplace_back();

    REQUIRE(l[0].status == lifetime::status::none);

    auto l2 = l;
    l.clear();
    REQUIRE(l[0].status == lifetime::status::destructed); // UB but it's OK for tests
    REQUIRE(l2[0].status != lifetime::status::destructed);

    l2.clear();
    REQUIRE(l2[0].status == lifetime::status::destructed); // UB but it's OK for tests
}

TEST_CASE("static_vector - swap") {
    svector<int> s1 = {1};
    svector<int> s2 = {2};

    std::swap(s1, s2);
    REQUIRE(s1[0] == 2);
    REQUIRE(s2[0] == 1);
}

TEST_CASE("static_vector - compare") {
    svector<int> s1 = {1};
    svector<int> s2 = {2};
    REQUIRE(s1 < s2);

    volkano::static_vector<int, 2> s3 = {1, 2};
    volkano::static_vector<int, 2> s4 = {2};
    REQUIRE(s4 > s3);
}

}
