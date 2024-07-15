//
// Created by gbian on 28/06/2024.
//
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../headers.hpp"
#include "vulkrt/cast/BaseCast.hpp"
#include "Times.hpp"
#include "timeFactors.hpp"

class Timestep {
public:
    explicit Timestep(const float time = 0.0F) noexcept : m_Time(C_LD(time)) {}
    explicit Timestep(const double time = 0.0) noexcept : m_Time(C_LD(time)) {}
    explicit Timestep(const long double time = 0.0L) noexcept : m_Time(time) {}
    explicit Timestep(const ch::duration<long double> &time) noexcept : m_Time(time.count()) {}

    explicit operator float() const noexcept { return C_F(m_Time); }
    explicit operator double() const noexcept { return C_D(m_Time); }
    explicit operator long double() const noexcept { return m_Time; }

    [[nodiscard]] long double GetSeconds() const noexcept { return m_Time; }
    [[nodiscard]] long double GetMilliseconds() const noexcept { return m_Time * vnd::STOMSFACTOR; }

private:
    long double m_Time;
};
// NOLINTEND(*-include-cleaner)