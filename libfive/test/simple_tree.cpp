/*
libfive: a CAD kernel for modeling with implicit functions

Copyright (C) 2020  Matt Keeter

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this file,
You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include "catch.hpp"

#include <array>
#include <future>

#include "libfive/tree/simple_tree.hpp"
#include "util/oracles.hpp"

using namespace libfive;

TEST_CASE("SimpleTree: basic operations")
{
    auto t = SimpleTree::X() + 1;
    REQUIRE(t.op() == Opcode::OP_ADD);
    REQUIRE(t.lhs().op() == Opcode::VAR_X);
    REQUIRE(t.rhs().op() == Opcode::CONSTANT);
    REQUIRE(t.rhs().value() == 1);
}

TEST_CASE("SimpleTree: remap")
{
    auto t = SimpleTree::X();
    auto y = SimpleTree::Y();
    for (unsigned i=0; i < 32768; ++i) {
        t = t + y * i;
    }
    auto z = t.remap(SimpleTree::Z(), SimpleTree::X(), SimpleTree::Y());

    // Make sure the original hasn't changed
    REQUIRE(t.op() == Opcode::OP_ADD);
    REQUIRE(t.lhs().op() == Opcode::OP_ADD);
    REQUIRE(t.rhs().op() == Opcode::OP_MUL);
    REQUIRE(t.rhs().lhs().op() == Opcode::VAR_Y);
    REQUIRE(t.rhs().rhs().op() == Opcode::CONSTANT);
    REQUIRE(t.lhs().rhs().lhs().op() == Opcode::VAR_Y);
    REQUIRE(t.lhs().rhs().rhs().op() == Opcode::CONSTANT);

    // Check that the remapping went through
    REQUIRE(z.op() == Opcode::OP_ADD);
    REQUIRE(z.lhs().op() == Opcode::OP_ADD);
    REQUIRE(z.rhs().op() == Opcode::OP_MUL);
    REQUIRE(z.rhs().lhs().op() == Opcode::VAR_X);
    REQUIRE(z.rhs().rhs().op() == Opcode::CONSTANT);
    REQUIRE(z.lhs().rhs().lhs().op() == Opcode::VAR_X);
    REQUIRE(z.lhs().rhs().rhs().op() == Opcode::CONSTANT);

    auto f = SimpleTree::X();
    f = f * 2 + f * 3 + f;
    REQUIRE(f.size() == 9);

    // Remapping should also deduplicate the X values
    auto g = f.remap(SimpleTree::Y(), SimpleTree::Y(), SimpleTree::Z());
    REQUIRE(g.size() == 7);
}

TEST_CASE("SimpleTree: count")
{
    auto x = SimpleTree::X() + 1;
    REQUIRE(x.size() == 3);

    auto y = SimpleTree::Y();
    REQUIRE(y.size() == 1);

    auto t = x + y;
    REQUIRE(t.size() == 5);

    // X, 1, (X + 1), (X + 1) + (X + 1)
    // (testing special-casing for binary operations with the same arguments)
    auto z = x + x;
    REQUIRE(z.size() == 4);

    auto q = x + SimpleTree::X();
    REQUIRE(q.size() == 5);
}
