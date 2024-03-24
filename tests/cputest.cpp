
#include "catch2/catch.hpp"
#include "../CPU.h"

TEST_CASE("LDA works properly", "[cpu]") {
    CPU cpu;

    SECTION("LDA stores proper value into accumulator") {
        cpu.LDA(0x00);
        REQUIRE(cpu.get_a() == 0);

        cpu.LDA(0x03);
        REQUIRE(cpu.get_a() == 0x03);

        cpu.LDA(0xFC);
        REQUIRE(cpu.get_a() == 0xFC);

        cpu.LDA(0x7F);
        REQUIRE(cpu.get_a() == 0x7F);

        cpu.LDA(0x80);
        REQUIRE(cpu.get_a() == 0x80);
    }

    SECTION("LDA properly updates zero flag") {
        cpu.LDA(0x64);
        REQUIRE(cpu.get_flag(ZERO) == 0);

        cpu.LDA(0x9C);
        REQUIRE(cpu.get_flag(ZERO) == 0);

        cpu.LDA(0);
        REQUIRE(cpu.get_flag(ZERO) == 1);

        cpu.LDA(0x9C);
        REQUIRE(cpu.get_flag(ZERO) == 1);

        cpu.LDA(0x64);
        REQUIRE(cpu.get_flag(ZERO) == 1);
    }

    SECTION("LDA properly updates negative flag") {
        cpu.LDA(0x7F);
        REQUIRE(cpu.get_flag(NEGATIVE) == 0);

        cpu.LDA(0x14);
        REQUIRE(cpu.get_flag(NEGATIVE) == 0);

        cpu.LDA(0);
        REQUIRE(cpu.get_flag(NEGATIVE) == 0);

        cpu.LDA(0xFF);
        REQUIRE(cpu.get_flag(NEGATIVE) == 1);

        cpu.LDA(0x14);
        REQUIRE(cpu.get_flag(NEGATIVE) == 1);

        cpu.LDA(0);
        REQUIRE(cpu.get_flag(NEGATIVE) == 1);
    }
}

TEST_CASE("TAX works properly", "[cpu]") {
    CPU cpu;

    SECTION("TAX puts the value of A into X") {
        cpu.LDA(0x10);

        REQUIRE(cpu.get_x() == 0);

        cpu.TAX();

        REQUIRE(cpu.get_x() == 0x10);

        cpu.LDA(0xFF);
        cpu.TAX();

        REQUIRE(cpu.get_x() == 0xFF);
    }

    SECTION("TAX properly updates zero flag") {
        cpu.set_a(0x10);
        cpu.TAX();

        REQUIRE(cpu.get_flag(ZERO) == 0);

        cpu.set_a(0x0);
        cpu.TAX();

        REQUIRE(cpu.get_flag(ZERO) == 1);

        cpu.set_a(0xFF);
        cpu.TAX();

        REQUIRE(cpu.get_flag(ZERO) == 1);

        cpu.set_a(0x20);
        cpu.TAX();

        REQUIRE(cpu.get_flag(ZERO) == 1);
    }

    SECTION("TAX properly updates negative flag") {
        REQUIRE(cpu.get_flag(NEGATIVE) == 0);

        cpu.set_a(0x10);
        cpu.TAX();

        REQUIRE(cpu.get_flag(NEGATIVE) == 0);

        cpu.set_a(0x0);
        cpu.TAX();

        REQUIRE(cpu.get_flag(NEGATIVE) == 0);

        cpu.set_a(0xFF);
        cpu.TAX();

        REQUIRE(cpu.get_flag(NEGATIVE) == 1);

        cpu.set_a(0x20);
        cpu.TAX();

        REQUIRE(cpu.get_flag(NEGATIVE) == 1);
    }
}