/*
 * Copyright (C) 2023 Emre Simsirli
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <array>

#include <doctest/doctest.h>
#include "core/util/string_utils.h"

using namespace volkano::string;

TEST_CASE("split string")
{
    SUBCASE("empty") {
        std::string e;
        std::vector<std::string_view> v = split(e, ",");
        REQUIRE(v.empty());
    }

    SUBCASE("default params") {
        std::string e = "tok1 tok2\ntok3\ttok4";
        std::vector<std::string_view> v = split(e, " \n\t");
        REQUIRE(v.size() == 4);
        REQUIRE((v[0] == "tok1"));
        REQUIRE((v[1] == "tok2"));
        REQUIRE((v[2] == "tok3"));
        REQUIRE((v[3] == "tok4"));
    }

    SUBCASE("custom param") {
        std::string e = "tok1 tok2\ntok3\ttok4.tok5";
        std::vector<std::string_view> v = split(e, ".");
        REQUIRE(v.size() == 2);
        REQUIRE((v[0] == "tok1 tok2\ntok3\ttok4"));
        REQUIRE((v[1] == "tok5"));
    }

    SUBCASE("cull empty at begin") {
        std::string e = "\n\ntok1\n\ntok2";
        std::vector<std::string_view> v = split(e, "\n");

        REQUIRE(v.size() == 2);
        REQUIRE((v[0] == "tok1"));
        REQUIRE((v[1] == "tok2"));
    }

    SUBCASE("cull empty at middle") {
        std::string e = "tok1\n\ntok2";
        std::vector<std::string_view> v = split(e, "\n");

        REQUIRE(v.size() == 2);
        REQUIRE((v[0] == "tok1"));
        REQUIRE((v[1] == "tok2"));
    }

    SUBCASE("cull empty at end") {
        std::string e = "tok1\n\ntok2\n\n";
        std::vector<std::string_view> v = split(e, "\n");

        REQUIRE(v.size() == 2);
        REQUIRE((v[0] == "tok1"));
        REQUIRE((v[1] == "tok2"));
    }

    SUBCASE("preserve empty") {
        std::string e = "tok1\n\ntok2\n\n";
        std::vector<std::string_view> v = split({
          .src = e,
          .delims = "\n",
          .cull_empty = false
        });

        REQUIRE(v.size() == 5);
        REQUIRE((v[0] == "tok1"));
        REQUIRE(v[1].empty());
        REQUIRE((v[2] == "tok2"));
        REQUIRE(v[3].empty());
        REQUIRE(v[4].empty());
    }
}

TEST_CASE("join string")
{
    SUBCASE("join empty container") {
        std::vector<std::string> strs{};
        std::string joined = join(join_params{strs});
        REQUIRE(joined.empty());
    }

    SUBCASE("join empty delimiter") {
        std::vector<std::string> strs{"a", "b", "c", "d"};
        std::string joined = join(join_params{strs, ""});
        REQUIRE_EQ(joined, "abcd");
    }

    SUBCASE("join vector of std::string") {
        std::vector<std::string> strs{"a", "b", "c", "d"};
        std::string joined = join(join_params{strs});
        REQUIRE_EQ(joined, "a, b, c, d");
    }

    SUBCASE("join vector of std::string_view") {
        std::vector<std::string_view> strs{"a", "b", "c", "d"};
        std::string joined = join(join_params{strs});
        REQUIRE_EQ(joined, "a, b, c, d");
    }

    SUBCASE("join array of const char*") {
        std::array strs{"a", "b", "c", "d"};
        std::string joined = join(join_params{strs});
        REQUIRE_EQ(joined, "a, b, c, d");
    }

    SUBCASE("join with different separator") {
        std::array strs{"a", "b", "c", "d"};
        std::string joined = join(join_params{strs, "|"});
        REQUIRE_EQ(joined, "a|b|c|d");
    }

    SUBCASE("join with begin end") {
        std::array strs{"a", "b", "c", "d"};
        std::string joined = join(join_params{strs, "|", "{", "}"});
        REQUIRE_EQ(joined, "{a|b|c|d}");
    }
}
