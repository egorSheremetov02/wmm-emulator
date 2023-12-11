#ifndef ACCESS_MODE_H
#define ACCESS_MODE_H

#include <cassert>

enum class AccessMode {
    RLX, REL, ACQ, REL_ACQ, SEQ_CST
};

inline constexpr int GetCmpClass(AccessMode mode) {
    switch (mode) {
        case AccessMode::RLX:
            return 0;
        case AccessMode::REL:
            return 1;
        case AccessMode::ACQ:
            return 1;
        case AccessMode::REL_ACQ:
            return 2;
        case AccessMode::SEQ_CST:
            return 3;
        default:
            assert(false);
    }
}

inline constexpr bool operator<(AccessMode lhs, AccessMode rhs) {
    return GetCmpClass(lhs) < GetCmpClass(rhs);
}

inline constexpr bool operator>(AccessMode lhs, AccessMode rhs) {
    return GetCmpClass(lhs) > GetCmpClass(rhs);
}

inline constexpr bool operator<=(AccessMode lhs, AccessMode rhs) {
    return GetCmpClass(lhs) <= GetCmpClass(rhs);
}

inline constexpr bool operator>=(AccessMode lhs, AccessMode rhs) {
    return GetCmpClass(lhs) >= GetCmpClass(rhs);
}

#endif //ACCESS_MODE_H