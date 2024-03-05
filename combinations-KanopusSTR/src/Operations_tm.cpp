#include <ctime>
#include <vector>
int compare_tm(const std::tm tm1, const std::tm tm2)
{
    if (tm1.tm_year > tm2.tm_year) {
        return -1;
    }
    if (tm1.tm_year < tm2.tm_year) {
        return 1;
    }
    if (tm1.tm_mon > tm2.tm_mon) {
        return -1;
    }
    if (tm1.tm_mon < tm2.tm_mon) {
        return 1;
    }
    if (tm1.tm_mday > tm2.tm_mday) {
        return -1;
    }
    if (tm1.tm_mday < tm2.tm_mday) {
        return 1;
    }
    return 0;
}

bool operator==(const std::tm tm1, const std::tm tm2)
{
    if (tm1.tm_year != tm2.tm_year) {
        return false;
    }
    if (tm1.tm_mon != tm2.tm_mon) {
        return false;
    }
    if (tm1.tm_mday != tm2.tm_mday) {
        return false;
    }
    return true;
}

bool operator!=(const std::tm tm1, const std::tm tm2)
{
    return !(tm1 == tm2);
}

int tm_diff_day(const std::tm tm1, const std::tm tm2)
{
    std::vector days_in_mon = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    std::tm tm_b;
    std::tm tm_s;
    int sign;
    int days_count = 0;
    if (compare_tm(tm1, tm2) == -1) {
        tm_b = tm1;
        tm_s = tm2;
        sign = 1;
    }
    else {
        tm_b = tm2;
        tm_s = tm1;
        sign = -1;
    }
    for (int i = tm_s.tm_year; i < tm_b.tm_year; ++i) {
        if (i % 4 != 0) {
            days_count += 365;
        }
        else {
            days_count += 366;
        }
    }
    for (int i = 0; i < tm_s.tm_mon; ++i) {
        if (i == 1) {
            if (tm_s.tm_year % 4 != 0) {
                days_count -= 28;
            }
            else {
                days_count -= 29;
            }
        }
        else {
            days_count -= days_in_mon[i];
        }
    }
    for (int i = 0; i < tm_b.tm_mon; ++i) {
        if (i == 1) {
            if (tm_b.tm_year % 4 != 0) {
                days_count += 28;
            }
            else {
                days_count += 29;
            }
        }
        else {
            days_count += days_in_mon[i];
        }
    }
    days_count += tm_b.tm_mday - tm_s.tm_mday;
    return days_count * sign;
}
