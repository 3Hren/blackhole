#pragma once

#include <ctime>

namespace blackhole {
inline namespace v1 {

static auto gmtime(const time_t* t, struct tm* tp) noexcept -> struct tm* {
    int yday;
    uintptr_t n, sec, min, hour, mday, mon, year, wday, days, leap;

    // The calculation is valid for positive time_t only.
    n = static_cast<uintptr_t>(*t);
    days = n / 86400;

    // Jaunary 1, 1970 was Thursday
    wday = (4 + days) % 7;
    n %= 86400;
    hour = n / 3600;
    n %= 3600;
    min = n / 60;
    sec = n % 60;

    // The algorithm based on Gauss's formula, see src/http/ngx_http_parse_time.c.
    // Days since March 1, 1 BC.
    days = days - (31 + 28) + 719527;

    // The "days" should be adjusted to 1 only, however, some March 1st's go to previous year, so
    // we adjust them to 2.  This causes also shift of the last Feburary days to next year, but we
    // catch the case when "yday" becomes negative.
    year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);
    yday = static_cast<int>(days - (365 * year + year / 4 - year / 100 + year / 400));

    leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));

    if (yday < 0) {
        yday = static_cast<int>(365 + leap + static_cast<unsigned long>(yday));
        year--;
    }

    // The empirical formula that maps "yday" to month. There are at least 10 variants, some of
    // them are:
    // mon = (yday + 31) * 15 / 459
    // mon = (yday + 31) * 17 / 520
    // mon = (yday + 31) * 20 / 612
    mon = static_cast<uintptr_t>((yday + 31) * 10 / 306);

    // The Gauss's formula that evaluates days before the month.
    mday = static_cast<unsigned long>(yday)- (367 * mon / 12 - 30) + 1;

    if (yday >= 306) {
        year++;
        mon -= 10;
        yday -= 306;
    } else {
        mon += 2;
        yday += 31 + 28 + static_cast<int>(leap);
    }

    tp->tm_sec = static_cast<int>(sec);
    tp->tm_min = static_cast<int>(min);
    tp->tm_hour = static_cast<int>(hour);
    tp->tm_mday = static_cast<int>(mday);
    tp->tm_mon = static_cast<int>(mon - 1);
    tp->tm_year = static_cast<int>(year - 1900);
    tp->tm_yday = yday;
    tp->tm_wday = static_cast<int>(wday);
    tp->tm_isdst = 0;

    return tp;
}

class tzinit_t {
public:
    tzinit_t() { tzset(); }
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"

static const tzinit_t tz;

#pragma clang diagnostic pop

static auto localtime(const time_t* t, struct tm* tp) noexcept -> struct tm* {
    time_t time = *t - timezone;
    gmtime_r(&time, tp);
    tp->tm_gmtoff = timezone;
    tp->tm_zone = *tzname;

    return tp;
}

} // namespace v1
} // namespace blackhole
