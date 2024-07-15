//
// Created by gbian on 06/06/2024.
//
// NOLINTBEGIN(*-easily-swappable-parameters)
#pragma once

#include "timeFactors.hpp"

DISABLE_WARNINGS_PUSH(26447 26481)

namespace vnd {
    class TimeValues {
    public:
        TimeValues() noexcept = default;
        ~TimeValues() = default;

        explicit TimeValues(const long double nanoseconds_) noexcept
          : seconds(nanoseconds_ / SECONDSFACTOR), millis(nanoseconds_ / MILLISECONDSFACTOR), micro(nanoseconds_ / MICROSECONDSFACTOR),
            nano(nanoseconds_) {
            if(minutes > 0) { LINFO(MINUTESFACTOR); }
        }

        TimeValues(const long double seconds_, const long double millis_, const long double micro_, const long double nano_) noexcept
          : seconds(seconds_), millis(millis_), micro(micro_), nano(nano_) {}

        TimeValues(const TimeValues &other) = default;
        TimeValues(TimeValues &&other) noexcept = default;
        TimeValues &operator=(const TimeValues &other) = default;
        TimeValues &operator=(TimeValues &&other) noexcept = default;

        [[nodiscard]] const long double &get_minutes() const noexcept { return minutes; }
        [[nodiscard]] const long double &get_seconds() const noexcept { return seconds; }
        [[nodiscard]] const long double &get_millis() const noexcept { return millis; }
        [[nodiscard]] const long double &get_micro() const noexcept { return micro; }
        [[nodiscard]] const long double &get_nano() const noexcept { return nano; }

    private:
        long double minutes{};
        long double seconds{};
        long double millis{};
        long double micro{};
        long double nano{};
    };

    class ValueLable {
    public:
        ValueLable() noexcept = default;
        ~ValueLable() = default;
        ValueLable(const long double time_val, const std::string_view time_label) noexcept : timeVal(time_val), timeLabel(time_label) {}
        ValueLable(const ValueLable &other) = default;
        ValueLable(ValueLable &&other) noexcept = default;
        ValueLable &operator=(const ValueLable &other) = default;
        ValueLable &operator=(ValueLable &&other) noexcept = default;

        [[nodiscard]] static std::tuple<long double, long double> calculateTransformTimeMicro(long double inputTimeMicro) noexcept {
            const ch::duration<long double, std::micro> durationmicros(inputTimeMicro);

            const auto durationUs = ch::duration_cast<ch::microseconds>(durationmicros);
            const auto durationNs = ch::duration_cast<ch::nanoseconds>(durationmicros - durationUs);
            return std::make_tuple(C_LD(durationUs.count()), C_LD(durationNs.count()));
        }
        [[nodiscard]] static std::tuple<long double, long double, long double> calculateTransformTimeMilli(
            long double inputTimeMilli) noexcept {
            const ch::duration<long double, std::milli> durationmils(inputTimeMilli);

            const auto durationMs = ch::duration_cast<ch::milliseconds>(durationmils);
            const auto durationUs = ch::duration_cast<ch::microseconds>(durationmils - durationMs);
            const auto durationNs = ch::duration_cast<ch::nanoseconds>(durationmils - durationMs - durationUs);
            return std::make_tuple(C_LD(durationMs.count()), C_LD(durationUs.count()), C_LD(durationNs.count()));
        }
        [[nodiscard]] static std::tuple<long double, long double, long double, long double> calculateTransformTimeSeconds(
            long double inputTimeSeconds) noexcept {
            const ch::duration<long double> durationSecs(inputTimeSeconds);

            const auto durationSc = ch::duration_cast<ch::seconds>(durationSecs);
            const auto durationMs = ch::duration_cast<ch::milliseconds>(durationSecs - durationSc);
            const auto durationUs = ch::duration_cast<ch::microseconds>(durationSecs - durationSc - durationMs);
            const auto durationNs = ch::duration_cast<ch::nanoseconds>(durationSecs - durationSc - durationMs - durationUs);

            return std::make_tuple(C_LD(durationSc.count()), C_LD(durationMs.count()), C_LD(durationUs.count()), C_LD(durationNs.count()));
        }
        /*[[nodiscard]] static std::tuple<long double, long double, long double, long double, long double> calculateTransformTimeMinutes(
            long double inputTimeSeconds) noexcept {
            const ch::duration<long double> durationMinutes(inputTimeSeconds);

            const auto durationMt = ch::duration_cast<ch::minutes>(durationMinutes);
            const auto durationSc = ch::duration_cast<ch::seconds>(durationMinutes - durationMt);
            const auto durationMs = ch::duration_cast<ch::milliseconds>(durationMinutes - durationMt - durationSc);
            const auto durationUs = ch::duration_cast<ch::microseconds>(durationMinutes - durationMt - durationSc - durationMs);
            const auto durationNs = ch::duration_cast<ch::nanoseconds>(durationMinutes - durationMt - durationSc - durationMs - durationUs);

            return std::make_tuple(C_LD(durationMt.count()), C_LD(durationSc.count()), C_LD(durationMs.count()), C_LD(durationUs.count()),
                                   C_LD(durationNs.count()));
        }*/

        [[nodiscard]] static std::tuple<long double, long double, long double, long double, long double> calculateTransformTimeMinutes(
            long double inputTimeMinutes) noexcept {
            const ch::duration<long double, std::ratio<60>> durationMins(inputTimeMinutes);

            const auto durationM = ch::duration_cast<ch::minutes>(durationMins);
            const auto durationSecs = ch::duration_cast<ch::seconds>(durationMins - durationM);
            const auto durationMs = ch::duration_cast<ch::milliseconds>(durationMins - durationM - durationSecs);
            const auto durationUs = ch::duration_cast<ch::microseconds>(durationMins - durationM - durationSecs - durationMs);
            const auto durationNs = ch::duration_cast<ch::nanoseconds>(durationMins - durationM - durationSecs - durationMs - durationUs);

            return std::make_tuple(C_LD(durationM.count()), C_LD(durationSecs.count()), C_LD(durationMs.count()), C_LD(durationUs.count()),
                                   C_LD(durationNs.count()));
        }

        [[nodiscard]] static std::string transformTimeMicro(long double inputTimeMicro) noexcept {
            const auto &[us, ns] = calculateTransformTimeMicro(inputTimeMicro);
            return FORMAT("{}us,{}ns", us, ns);
        }

        [[nodiscard]] static std::string transformTimeMilli(long double inputTimeMilli) noexcept {
            const auto &[ms, us, ns] = calculateTransformTimeMilli(inputTimeMilli);
            return FORMAT("{}ms,{}us,{}ns", ms, us, ns);
        }

        [[nodiscard]] static std::string transformTimeSeconds(long double inputTimeSeconds) noexcept {
            const auto &[s, ms, us, ns] = calculateTransformTimeSeconds(inputTimeSeconds);
            return FORMAT("{}s,{}ms,{}us,{}ns", s, ms, us, ns);
        }
        [[nodiscard]] static std::string transformTimeMinutes(long double inputTimeMinutes) noexcept {
            const auto &[m, s, ms, us, ns] = calculateTransformTimeMinutes(inputTimeMinutes);
            return FORMAT("{}m,{}s,{}ms,{}us,{}ns", m, s, ms, us, ns);
        }
        [[nodiscard]] std::string toString() const noexcept {
            if(timeLabel == "m") { return transformTimeMinutes(timeVal); }
            if(timeLabel == "s") { return transformTimeSeconds(timeVal); }
            if(timeLabel == "ms") { return transformTimeMilli(timeVal); }
            if(timeLabel == "us") { return transformTimeMicro(timeVal); }
            return FORMAT("{}{}", timeVal, timeLabel);
        }

        [[nodiscard]] std::string transformTimeMicroSTD(long double inputTimeMicro) const noexcept {
            const auto &[us, ns] = calculateTransformTimeMicro(inputTimeMicro);
            return FORMATST("{}us,{}ns", us, ns);
        }

        [[nodiscard]] std::string transformTimeMilliSTD(long double inputTimeMilli) const noexcept {
            const auto &[ms, us, ns] = calculateTransformTimeMilli(inputTimeMilli);

            return FORMATST("{}ms,{}us,{}ns", ms, us, ns);
        }

        [[nodiscard]] std::string transformTimeSecondsSTD(long double inputTimeSeconds) const noexcept {
            const auto &[s, ms, us, ns] = calculateTransformTimeSeconds(inputTimeSeconds);
            return FORMATST("{}s,{}ms,{}us,{}ns", s, ms, us, ns);
        }
        [[nodiscard]] static std::string transformTimeMinutesSTD(long double inputTimeMinutes) noexcept {
            const auto &[m, s, ms, us, ns] = calculateTransformTimeMinutes(inputTimeMinutes);
            return FORMATST("{}m,{}s,{}ms,{}us,{}ns", m, s, ms, us, ns);
        }
        [[nodiscard]] std::string toStringSTD() const noexcept {
            if(timeLabel == "m") { return transformTimeMinutesSTD(timeVal); }
            if(timeLabel == "s") { return transformTimeSecondsSTD(timeVal); }
            if(timeLabel == "ms") { return transformTimeMilliSTD(timeVal); }
            if(timeLabel == "us") { return transformTimeMicroSTD(timeVal); }
            return FORMATST("{}{}", timeVal, timeLabel);
        }

    private:
        long double timeVal{};
        std::string timeLabel{""};
    };

    class Times {
    public:
        Times() noexcept = default;
        ~Times() = default;
        explicit Times(const long double nanoseconds_) noexcept : values(nanoseconds_) {}

        explicit Times(const TimeValues &time_values) noexcept : values(time_values) {}

        Times(const TimeValues &time_values, const std::string_view labelseconds_, const std::string_view labelmillis_,
              const std::string_view labelmicro_, std::string_view labelnano_) noexcept
          : values(time_values), labelseconds(labelseconds_), labelmillis(labelmillis_), labelmicro(labelmicro_), labelnano(labelnano_) {}

        Times(const Times &other) = default;
        Times(Times &&other) noexcept = default;
        Times &operator=(const Times &other) = default;
        Times &operator=(Times &&other) noexcept = default;

        [[nodiscard]] ValueLable getRelevantTimeframe() const noexcept {
            if(values.get_seconds() / 60 > 1) {
                return {values.get_seconds() / 60, labelminutes};
            } else if(values.get_seconds() > 1) {  // seconds
                return {values.get_seconds(), labelseconds};
            } else if(values.get_millis() > 1) {  // millis
                return {values.get_millis(), labelmillis};
            } else if(values.get_micro() > 1) {  // micros
                return {values.get_micro(), labelmicro};
            } else {  // nanos
                return {values.get_nano(), labelnano};
            }
        }

    private:
        // Campi della classe
        TimeValues values;
        std::string_view labelminutes{"m"};
        std::string_view labelseconds{"s"};
        std::string_view labelmillis{"ms"};
        std::string_view labelmicro{"us"};
        std::string_view labelnano{"ns"};
    };
    DISABLE_WARNINGS_POP()
}  // namespace vnd

/**
 * This function is a formatter for CodeSourceLocation using fmt.
 * \cond
 */

// NOLINTNEXTLINE
template <> struct fmt::formatter<vnd::ValueLable> : fmt::formatter<std::string_view> {
    auto format(vnd::ValueLable val, format_context &ctx) const -> format_context::iterator {
        return fmt::formatter<std::string_view>::format(val.toString(), ctx);
    }
};

template <> struct std::formatter<vnd::ValueLable, char> {
    template <class ParseContext> constexpr auto parse(ParseContext &&ctx) -> decltype(ctx.begin()) { return ctx.begin(); }

    template <typename FormatContext> auto format(const vnd::ValueLable &valueLabel, FormatContext &ctx) const -> decltype(ctx.out()) {
        return std::format_to(ctx.out(), "{}", valueLabel.toStringSTD());
    }
};
/** \endcond */
// NOLINTEND(*-easily-swappable-parameters)