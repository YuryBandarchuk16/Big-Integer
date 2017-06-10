//
//  big_integer.cpp
//  BigInteger
//
//  Created by Юрий Бондарчук on 6/3/17.
//  Copyright © 2017 Yury Bandarchuk. All rights reserved.
//


#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cassert>

#include "big_integer.h"

namespace ui {
    big_integer::uint * alloc(int sz, big_integer::uint rf) {
        big_integer::uint * ret = new big_integer::uint[sz + 1];
        ret[0] = rf;
        ++ret;
        return ret;
    }
    big_integer::uint * alloc_and_fill(int sz) {
        big_integer::uint * ret = new big_integer::uint[sz];
        for (int i = 0; i < sz; ++i) {
            ret[i] = 0;
        }
        return ret;
    }
    void dealloc(big_integer::uint * ptr) {
        --ptr;
        delete[] ptr;
    }
    void retain(big_integer::uint * ptr) {
        ++ptr[-1];
    }
    void release(big_integer::uint * ptr) {
        --ptr;
        ptr[0]--;
        if (ptr[0] == 0) {
            ++ptr;
            dealloc(ptr);
        }
    }
}

big_integer::big_integer() {
    capacity = 1;
    small = 0LL;
}

void big_integer::ensure_capacity(int size) {
    if (size > capacity) {
        resize(size + size);
    }
}

void big_integer::resize(int new_size) {
    uint * tmp = ui::alloc(new_size, 1);
    for (size_t i = 0; i < size; ++i) {
        tmp[i] = elements[i];
    }
    for (int i = size; i < new_size; ++i) {
        tmp[i] = 0;
    }
    capacity = new_size;
    ui::release(elements);
    elements = tmp;
}

big_integer::big_integer(std::string const& str) {
    size_t start_idx = 0;
    sign = 1;
    if (str[0] == '-') {
        start_idx = 1;
        sign = -1;
    }
    size_t its_length = start_idx;
    while (its_length < str.size() && str[its_length] == '0') ++its_length;
    int diff = (int)str.size() - (int)its_length;
    if (diff <= 10) {
        small = 0LL;
        for (size_t i = its_length; i < str.size(); ++i) {
            small = small * 10LL + (long long)str[i] - 48LL;
        }
        if (str[0] == '-') {
            small *= -1LL;
        }
        if (small >= LEFT_BORDER && small <= RIGHT_BORDER) {
            capacity = 1;
            return;
        }
    }
    std::vector<int> digits;
    size = 0;
    capacity = std::max((int)str.size(), 2);
    elements = ui::alloc(capacity, 1);
    for (size_t i = 0; i < capacity; ++i) {
        elements[i] = 0;
    }
    for (size_t i = start_idx; i < str.size(); ++i) {
        digits.push_back((int)str[i] - 48);
    }
    for (size_t iters = 0; iters < str.size(); ++iters) {
        bool fail = true;
        ll cur = 0LL;
        for (size_t i = 0; i < digits.size(); ++i) {
            cur = (cur * 10LL) + (ll)digits[i];
            digits[i] = (int)(cur / BASE);
            (digits[i] != 0 ? fail &= false : fail &= true);
            cur %= BASE;
        }
        elements[size++] = (uint)cur;
        if (fail) break;
    }
    make_correct();
    check_sign();
}


void negate_module(long long &x, long long &y) {
    y = 0;
    while(x < 0) {
        x += big_integer::BASE;
        ++y;
    }
}

int compare_absolute_value(big_integer const& a, big_integer const& b) {
    if (a.capacity == 1 && b.capacity == 1) {
        if (std::abs(a.small) < std::abs(b.small)) {
            return -1;
        } else if (std::abs(a.small) > std::abs(b.small)) {
            return 1;
        }
        return 0;
    }
    if (a.capacity == 1 && b.size <= 2) {
        big_integer::ll b_medium = 1LL * b.elements[0] + 1LL * big_integer::BASE * b.elements[1];
        if (std::abs(a.small) < std::abs(b_medium)) {
            return -1;
        } else if (std::abs(a.small) > std::abs(b.small)) {
            return 1;
        }
        return 0;
    }
    if (b.capacity == 1 && a.size <= 2) {
        big_integer::ll a_medium = 1LL * a.elements[0] + 1LL * big_integer::BASE * a.elements[1];
        if (std::abs(b.small) < std::abs(a_medium)) {
            return 1;
        } else if (std::abs(b.small) > std::abs(a_medium)) {
            return -1;
        }
        return 0;
    }
    if (a.capacity == 1) {
        return -1;
    }
    if (b.capacity == 1) {
        return 1;
    }
    if (a.size < b.size) {
        return -1;
    }
    if (a.size > b.size) {
        return 1;
    }
    for (int i = a.size - 1; /* empty */; --i) {
        if (a.elements[i] > b.elements[i]) {
            return 1;
        }
        if (a.elements[i] < b.elements[i]) {
            return -1;
        }
        if (i == 0) {
            break;
        }
    }
    return 0;
}

int compare(big_integer const& a, big_integer const& b) {
    if (a.capacity == 1 && b.capacity == 1) {
        if (a.small < b.small) {
            return -1;
        }
        if (a.small > b.small) {
            return 1;
        }
        return 0;
    }
    if (a.capacity == 1 && b.size <= 2) {
        big_integer::ll b_medium = 1LL * b.elements[0] + 1LL * big_integer::BASE * b.elements[1];
        if (b.sign == -1) b_medium *= -1LL;
        if (a.small < b_medium) {
            return -1;
        } else if (a.small > b.small) {
            return 1;
        }
        return 0;
    }
    if (b.capacity == 1 && a.size <= 2) {
        big_integer::ll a_medium = 1LL * a.elements[0] + 1LL * big_integer::BASE * a.elements[1];
        if (a.sign == -1) a_medium *= -1;
        if (b.small < a_medium) {
            return 1;
        } else if (b.small > a_medium) {
            return -1;
        }
        return 0;
    }
    if (a.capacity == 1) {
        if (a.small < 0 && b.sign == 1) {
            return -1;
        }
        if (a.small > 0 && b.sign == -1) {
            return 1;
        }
        return -1;
    }
    if (b.capacity == 1) {
        if (b.small < 0 && a.sign == 1) {
            return 1;
        }
        if (b.small > 0 && a.sign == -1) {
            return -1;
        }
        return 1;
    }
    if (a.sign < b.sign) {
        return -1;
    }
    if (a.sign > b.sign) {
        return 1;
    }
    if (a.sign == -1) {
        if (a.size < b.size) {
            return 1;
        }
        if (a.size > b.size) {
            return -1;
        }
        for (int i = a.size - 1; /* empty */; --i) {
            if (a.elements[i] > b.elements[i]) {
                return -1;
            }
            if (a.elements[i] < b.elements[i]) {
                return 1;
            }
            if (i == 0) break;
        }
        return 0;
    } else {
        return compare_absolute_value(a, b);
    }
}

bool operator==(big_integer const& a, big_integer const& b) {
    return compare(a, b) == 0;
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return compare(a, b) != 0;
}

bool operator<(big_integer const& a, big_integer const& b) {
    return compare(a, b) == -1;
}

bool operator>(big_integer const& a, big_integer const& b) {
    return compare(a, b) == 1;
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return compare(a, b) != 1;
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return compare(a, b) != -1;
}

big_integer::big_integer(big_integer const &other) {
    sign = other.sign;
    size = other.size;
    capacity = other.capacity;
    if (other.capacity == 1) {
        small = other.small;
    } else {
        elements = other.elements;
        ui::retain(elements);
    }
}

big_integer::~big_integer() {
    if (capacity == 1) return;
    ui::release(elements);
}

big_integer::big_integer(int x) {
    capacity = 1;
    small = 1LL * x;
}

big_integer &big_integer::operator=(big_integer const &other) {
    big_integer copy = big_integer(other);
    swap(copy);
    return *this;
}

void big_integer::swap(big_integer & copy)  {
    std::swap(sign, copy.sign);
    std::swap(elements, copy.elements);
    std::swap(size, copy.size);
    std::swap(capacity, copy.capacity);
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
    if (capacity == 1 && rhs.capacity == 1) {
        long long new_small = small + rhs.small;
        if (new_small >= LEFT_BORDER && new_small <= RIGHT_BORDER) {
            small = new_small;
            return *this;
        }
    }
    if (capacity == 1) {
        turn_big_mode();
    }
    copy_on_write();
    if (rhs.capacity == 1) {
        if (rhs.small > 0) {
            return add_small(rhs.small);
        } else {
            return sub_small(-rhs.small);
        }
    }
    if (sign == rhs.sign) {
        add(rhs);
        return *this;
    }
    if (compare_absolute_value(*this, rhs) < 0) {
        big_integer copy = rhs;
        copy.copy_on_write();
        copy.sub(*this);
        swap(copy);
    } else {
        sub(rhs);
    }
    return *this;
}

big_integer &big_integer::add_small(big_integer::ll value) {
    big_integer::ll carry = 0;
    for (int i = 0; i < size; i++) {
        carry += (1LL * elements[i]);
        elements[i] = (uint)(carry % BASE);
        carry /= BASE;
        if (carry == 0) {
            break;
        }
    }
    if (carry != 0) {
        elements[size++] += (uint)carry;
    }
    make_correct();
    check_sign();
    return *this;
}

big_integer &big_integer::add(big_integer const& rhs) {
    assert(capacity != 1);
    if (rhs.capacity == 1) {
        ensure_capacity(size + 1);
        return add_small(rhs.small);
    }
    ensure_capacity(std::max(size, rhs.size) + 3);
    ll cur = 0LL;
    int max_size = std::max(size, rhs.size) + 2;
    for (int i = 0; i < max_size; ++i) {
        if (i < size) cur += 1LL * elements[i];
        if (i < rhs.size) cur += (1LL * rhs.elements[i]);
        elements[i] = (uint)(cur % BASE);
        cur /= BASE;
    }
    make_correct();
    check_sign();
    return *this;
}

big_integer &big_integer::sub_small(big_integer::ll value) {
    big_integer::ll decrement = value;
    for (int i = 0; i < size; i++) {
        big_integer::ll cur = elements[i] - decrement;
        if (cur < 0LL) {
            elements[i] = (uint)(cur + BASE);
            decrement = 1LL;
        } else {
            elements[i] = (uint)(cur);
            decrement = 0LL;
        }
        if (decrement == 0LL) {
            break;
        }
    }
    make_correct();
    check_sign();
    return *this;
}

big_integer &big_integer::sub(big_integer const& rhs) {
    assert(capacity != 1);
    if (rhs.capacity == 1) {
        return sub_small(rhs.small);
    }
    ensure_capacity(std::max(size, rhs.size) + 2);
    bool decrement = false;
    for (size_t i = 0; i < size; ++i) {
        if (i >= rhs.size && !decrement) break;
        if (i >= rhs.size && decrement) {
            if (elements[i] == 0) {
                elements[i] = (uint)(BASE - 1LL);
            } else {
                --elements[i];
                decrement = false;
            }
        } else {
            ll cur = 1LL * elements[i] - 1LL * rhs.elements[i] - 1LL * decrement;
            if (cur < 0) cur += BASE, decrement = true; else decrement = false;
            elements[i] = ((uint)(cur));
        }
    }
    make_correct();
    check_sign();
    return *this;
}

void big_integer::remove_zeroes() {
    if (capacity == 1) return;
    while (size > 1 && elements[size - 1] == 0) {
        --size;
    }
}

inline void big_integer::check_sign() {
    if (capacity == 1) return;
    if (size == 1 && elements[0] == 0) {
        sign = 1;
    }
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (capacity == 1 && rhs.capacity == 1) {
        big_integer::ll new_small = small - rhs.small;
        if (new_small >= LEFT_BORDER && new_small <= RIGHT_BORDER) {
            small = new_small;
            return *this;
        }
    }
    if (capacity == 1) {
        turn_big_mode();
    }
    copy_on_write();
    if (rhs.capacity == 1) {
        if (rhs.small < 0) {
            return add_small(-rhs.small);
        } else {
            return sub_small(rhs.small);
        }
    }
    if (sign != rhs.sign) {
        add(rhs);
        return *this;
    }
    if (compare_absolute_value(*this, rhs) < 0) {
        big_integer copy = rhs;
        copy.copy_on_write();
        copy.sub(*this);
        *this = copy;
        sign *= -1;
    } else {
        sub(rhs);
    }
    return *this;
}

big_integer big_integer::operator+() const {
    big_integer tmp = *this;
    return tmp;
}

big_integer big_integer::operator-() const {
    big_integer tmp = *this;
    if (capacity == 1) {
        tmp.small *= -1;
        if (tmp.small >= LEFT_BORDER && tmp.small <= RIGHT_BORDER) {
            return tmp;
        }
        tmp.turn_big_mode();
        return tmp;
    }
    if (size == 1 && elements[0] == 0) return tmp;
    tmp.copy_on_write();
    tmp.sign *= -1;
    return tmp;
}

void big_integer::make_correct() {
    remove_zeroes();
    ensure_capacity(size);
}

big_integer big_integer::operator~() const {
    big_integer tmp = *this;
    if (tmp.capacity == 1) {
        ++tmp.small;
        tmp.small = -tmp.small;
        if (tmp.small >= LEFT_BORDER && tmp.small <= RIGHT_BORDER) {
            return tmp;
        }
        tmp.turn_big_mode();
        return tmp;
    }
    tmp.copy_on_write();
    ++tmp;
    tmp.sign *= -1;
    tmp.make_correct();
    return tmp;
}

big_integer &big_integer::operator++() {
    if (capacity == 1) {
        ++small;
        if (small >= LEFT_BORDER && small <= RIGHT_BORDER) {
            return *this;
        }
        turn_big_mode();
        return *this;
    }
    (*this) += 1;
    return *this;
}

big_integer big_integer::operator++(int) {
    big_integer tmp = *this;
    (*this) += 1;
    return tmp;
}

big_integer &big_integer::operator--() {
    (*this) -= 1;
    return *this;
}

big_integer big_integer::operator--(int) {
    big_integer tmp = *this;
    (*this) -= 1;
    return tmp;
}

void big_integer::shift_left(int k) {
    if (capacity == 1) {
        if (small == 0LL) {
            return;
        }
        if (k <= 31) {
            small <<= (1LL * k);
            if (small >= LEFT_BORDER && small <= RIGHT_BORDER) {
                return;
            }
            turn_big_mode();
            return;
        } else {
            turn_big_mode();
        }
    }
    copy_on_write();
    int blocks = k / POWER;
    k %= POWER;
    ensure_capacity(size + blocks + 1);
    uint tmp = 0, shift = (uint)POWER - (uint)k;
    uint mask = (1U << shift) - 1;
    for (int i = size - 1; i >= 0; --i) {
        tmp = elements[i];
        elements[i] = 0;
        elements[i + blocks + 1] |= (tmp >> shift);
        elements[i + blocks] |= ((tmp & mask) << k);
    }
    size += blocks + 1;
    make_correct();
}

void big_integer::shift_right(int k) {
    if (capacity == 1) {
        small >>= (1LL * k);
        return;
    }
    copy_on_write();
    int blocks = k / POWER;
    k %= POWER;
    bool dec_one = ((elements[0] & ((1U << (uint)k) - 1)) != 0);
    elements[0] >>= k;
    for (int i = 1; i < size; ++i) {
        elements[i - 1] |= (elements[i] & ((1U << (uint)k) - 1)) << (POWER - k);
    }
    int max_idx_possible = std::min(blocks, size);
    for (int i = 0; i < size; ++i) {
        if (i < max_idx_possible && elements[i] != 0) {
            dec_one = true;
        }
        elements[i] = (i + blocks < size ? elements[i + blocks] : 0);
    }
    if ((sign < 0) && (dec_one)) {
        --(*this);
    }
    make_correct();
}

std::string to_string(big_integer const& a) { // TODO
    if (a.capacity == 1) {
        return std::to_string(a.small);
    }
    int idx = a.size - 1;
    big_integer::uint * tmp = new big_integer::uint[idx + 1];
    for (int i = 0; i <= idx; ++i) {
        tmp[i] = a.elements[i];
    }
    std::string result = "";
    while (true) {
        bool fail = true;
        big_integer::ll cur = 0LL;
        for (int i = idx; /*empty */; --i) {
            cur = cur * big_integer::BASE + 1LL * tmp[i];
            tmp[i] = (big_integer::uint)(cur / 10LL);
            cur %= 10LL;
            (tmp[i] != 0LL ? fail &= false : fail &= true);
            if (i == 0) break;
        }
        result += (char)(cur + 48);
        if (fail) break;
        while (idx > 1 && tmp[idx] == 0) --idx;
    }
    result += (a.sign == -1 ? "-" : "");
    std::reverse(result.begin(), result.end());
    delete[] tmp;
    return result;
}

big_integer operator+(big_integer a, big_integer const& b) {
    a += b;
    return a;
}
big_integer operator-(big_integer a, big_integer const& b) {
    a -= b;
    return a;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    if (capacity == 1 && rhs.capacity == 1) {
        big_integer::ll new_small = small * rhs.small;
        small = new_small;
        if (new_small >= LEFT_BORDER && new_small <= RIGHT_BORDER) {
            return *this;
        } else {
            turn_big_mode();
            return *this;
        }
    }
    if (capacity == 1) {
        turn_big_mode();
    }
    if (rhs.capacity == 1) {
        ensure_capacity(size + 2);
        copy_on_write();
        return mul_small(rhs.small);
    }
    sign *= rhs.sign;
    uint * tmp = ui::alloc(size + rhs.size + 5, 1);
    for (int i = 0; i < size + rhs.size + 5; ++i) {
        tmp[i] = 0;
    }
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < rhs.size; ++j) {
            ll val = (1LL * elements[i] * rhs.elements[j]) + 1LL * tmp[i + j];
            size_t idx = i + j;
            tmp[idx] = ((uint)(val % BASE));
            val /= BASE;
            ++idx;
            tmp[idx] += ((uint)(val));
            while (1LL * tmp[idx] >= BASE) {
                uint cnt = 0;
                while (1LL * tmp[idx] >= BASE) {
                    ++cnt;
                    tmp[idx] -= BASE;
                }
                if (cnt == 0) break;
                ++idx;
                tmp[idx] += cnt;
            }
        }
    }
    ui::release(elements);
    elements = tmp;
    size += rhs.size;
    capacity = size + 1;
    make_correct();
    check_sign();
    return *this;
}

void inverse_all(big_integer::uint * a, int sz, bool do_i_really_need_this_shit) {
    if (!do_i_really_need_this_shit) return;
    for (int i = 0; i < sz; ++i) {
        a[i] ^= (big_integer::uint)(big_integer::BASE - 1);
    }
    big_integer::uint add = 1;
    for (int i = 0; i < sz && add > 0; ++i) {
        a[i] += add;
        if (1LL * a[i] >= big_integer::BASE) {
            a[i] -= (big_integer::uint)(big_integer::BASE);
            add = 1;
        } else {
            add = 0;
        }
    }
}

void subtract_division_result(big_integer::uint * a, big_integer::uint * b, int from, int to, big_integer::ll &carry, big_integer::ll denum,
                              big_integer::ll &got) {
    carry = 0LL;
    long long dec;
    for (int j = from; j <= to; j++) {
        carry += (big_integer::ll)a[j] - b[j] * got;
        denum = carry % big_integer::BASE;
        carry /= big_integer::BASE;
        negate_module(denum, dec);
        carry -= dec;
        a[j] = (uint)denum;
    }
    if (carry != 0) {
        a[++to] += carry;
        big_integer::uint cnt = 0;
        while (1LL * a[to] >= big_integer::BASE) {
            a[to] -= (big_integer::uint)(big_integer::BASE);
            ++cnt;
        }
        if (cnt > 0) {
            a[++to] += cnt;
        }
    }
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    if (rhs.capacity == 1 && rhs.small == 0) {
        throw std::runtime_error("oops, division by zero :(");
    }
    if (capacity == 1 && rhs.capacity == 1) {
        small /= rhs.small;
        if (LEFT_BORDER <= small && small <= RIGHT_BORDER) {
            return *this;
        } else {
            turn_big_mode();
            return *this;
        }
    }
    if (capacity == 1) {
        turn_big_mode();
    }
    copy_on_write();
    if (rhs.capacity == 1) {
        return div_small(rhs.small);
    }
    if (rhs == 0) {
        throw std::runtime_error("oops, division by zero :(");
    }
    if (compare_absolute_value(*this, rhs) < 0) {
        *this = 0;
        return *this;
    }
    if (size == 1 && elements[0] == 0) {
        return *this;
    }
    big_integer copy = rhs;
    if (rhs.capacity == 1) {
        copy.turn_big_mode();
    }
    int new_sign = sign * copy.sign;
    int sz_before = copy.size;
    copy.copy_on_write();
    int tmp_size = size - copy.size + 3;
    uint *tmp = ui::alloc(tmp_size, 1);
    for (int i = 0; i < tmp_size; ++i) {
        tmp[i] = 0;
    }
    long double numerator;
    big_integer::ll denumerator;
    big_integer::ll carry, res;
    sign = copy.sign = 1;
    int was_sz = size;
    copy <<= (POWER * (size - copy.size));
    copy_on_write();
    elements[size] = 0;
    int max_size_allowed = std::max(sz_before - 1, 0);
    for (int i = size - 1; i >= max_size_allowed; --i) {
        int idx = i - sz_before + 1;
        numerator = (i == 0 ? 0 : 1.0L * elements[i - 1]);
        denumerator = (i == 0 ? 0 : 1LL * copy.elements[i - 1]);
        numerator += (1.0L * BASE * BASE * elements[i + 1]);
        numerator += (1LL * BASE * elements[i]);
        denumerator += (1LL * BASE * copy.elements[i]) + 1;
        tmp[idx] = (uint)((big_integer::ll)((numerator / denumerator)));
        res = (big_integer::ll)tmp[idx];
        subtract_division_result(elements, copy.elements, idx, i, carry, denumerator, res);
        while ((size > 1) && (elements[size - 1] == 0)) {
            --size;
        }
        res = 1LL;
        if(*this >= copy) {
            ++tmp[idx];
            subtract_division_result(elements, copy.elements, idx, i, carry, denumerator, res);
        }
        for (int j = std::max(i - copy.size, 0); j < i; j++) {
            copy.elements[j] = copy.elements[j + 1];
        }
        copy.elements[i] = 0;
        copy.size--;
    }
    ui::release(elements);
    sign = new_sign;
    size = was_sz - sz_before + 1;
    elements = tmp;
    capacity = tmp_size;
    make_correct();
    check_sign();
    return *this;
}

big_integer &big_integer::operator<<=(int rhs) {
    if (rhs < 0) {
        shift_right(-rhs);
    } else {
        shift_left(rhs);
    }
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs < 0) {
        shift_left(-rhs);
    } else {
        shift_right(rhs);
    }
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    big_integer ths = *this;
    big_integer tmp = ths / rhs;
    big_integer mult = tmp * rhs;
    tmp = ths - mult;
    *this = tmp;
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    if (capacity == 1 && rhs.capacity == 1) {
        small &= rhs.small;
        if (LEFT_BORDER <= small && small <= RIGHT_BORDER) {
            return *this;
        } else {
            turn_big_mode();
            return *this;
        }
    }
    if (capacity == 1) {
        turn_big_mode();
    }
    if (rhs.capacity == 1) {
        copy_on_write();
        big_integer::ll abs_taken = std::abs(rhs.small);
        big_integer::ll first = (abs_taken % BASE), second = (abs_taken) / BASE;
        if (rhs.small < 0LL) {
            first ^= (BASE - 1LL);
            second ^= (BASE - 1LL);
        }
        elements[0] &= first;
        elements[1] &= second;
        for (int i = 2; i < size; i++) {
            elements[i] = 0;
        }
        make_correct();
        check_sign();
        return *this;
    }
    if (size < rhs.size) {
        ensure_capacity(rhs.size);
    }
    copy_on_write();
    big_integer tmp = big_integer(rhs);
    inverse_all(elements, size, sign == -1);
    inverse_all(tmp.elements, tmp.size, tmp.sign == -1);
    for (int i = 0; i < size; i++) {
        uint x = elements[i], y = 0;
        if (i < tmp.size) {
            y = tmp.elements[i];
        }
        elements[i] = (x & y);
    }
    if (sign != tmp.sign || sign != -1) {
        sign = 1;
    }
    inverse_all(elements, size, sign == -1);
    make_correct();
    check_sign();
    return *this;
}

big_integer &big_integer::operator|=(const big_integer &rhs) {
    if (capacity == 1 && rhs.capacity == 1) {
        small |= rhs.small;
        if (LEFT_BORDER <= small && small <= RIGHT_BORDER) {
            return *this;
        } else {
            turn_big_mode();
            return *this;
        }
    }
    if (capacity == 1) {
        turn_big_mode();
    }
    if (rhs.capacity == 1) {
        copy_on_write();
        big_integer::ll abs_taken = std::abs(rhs.small);
        big_integer::ll first = (abs_taken % BASE), second = (abs_taken) / BASE;
        if (rhs.small < 0LL) {
            first ^= (BASE - 1LL);
            second ^= (BASE - 1LL);
        }
        elements[0] |= first;
        elements[1] |= second;
        make_correct();
        check_sign();
        return *this;
    }
    if (size < rhs.size) {
        ensure_capacity(rhs.size);
    }
    big_integer tmp = big_integer(rhs);
    inverse_all(elements, size, sign == -1);
    inverse_all(tmp.elements, tmp.size, tmp.sign == -1);
    for (int i = 0; i < size; i++) {
        uint x = elements[i], y = 0;
        if (i < tmp.size) {
            y = tmp.elements[i];
        }
        elements[i] = (x | y);
    }
    if (sign == -1 || tmp.sign == -1) {
        sign = -1;
    }
    inverse_all(elements, size, sign == -1);
    make_correct();
    check_sign();
    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    if (capacity == 1 && rhs.capacity == 1) {
        small ^= rhs.small;
        if (LEFT_BORDER <= small && small <= RIGHT_BORDER) {
            return *this;
        } else {
            turn_big_mode();
            return *this;
        }
    }
    if (capacity == 1) {
        turn_big_mode();
    }
    if (rhs.capacity == 1) {
        copy_on_write();
        big_integer::ll abs_taken = std::abs(rhs.small);
        big_integer::ll first = (abs_taken % BASE), second = (abs_taken) / BASE;
        if (rhs.small < 0LL) {
            first ^= (BASE - 1LL);
            second ^= (BASE - 1LL);
        }
        elements[0] ^= first;
        elements[1] ^= second;
        make_correct();
        check_sign();
        return *this;
    }
    if (size < rhs.size) {
        ensure_capacity(rhs.size);
    }
    big_integer tmp = big_integer(rhs);
    inverse_all(elements, size, sign == -1);
    inverse_all(tmp.elements, tmp.size, tmp.sign == -1);
    for (int i = 0; i < size; i++) {
        uint x = elements[i], y = 0;
        if (i < tmp.size) {
            y = tmp.elements[i];
        }
        elements[i] = (x ^ y);
    }
    if (sign != tmp.sign) {
        sign = -1;
    }
    inverse_all(elements, size, sign == -1);
    make_correct();
    check_sign();
    return *this;
}

void big_integer::turn_big_mode() {
    big_integer::ll saved_small = small;
    elements = ui::alloc(3, 1);
    if (saved_small < 0) {
        sign = -1;
        saved_small = -saved_small;
    } else {
        sign = 1;
    }
    elements[0] = (uint)(saved_small % BASE);
    elements[1] = (uint)(saved_small / BASE);
    elements[2] = 0;
    size = 2;
    if (elements[1] == 0) {
        size = 1;
    }
    capacity = 3;
}

void big_integer::copy_on_write() {
    if (capacity == 1) {
        return;
    }
    if (elements[-1] == 1) return;
    uint *tmp = ui::alloc(capacity, 1);
    for (int i = 0; i < capacity; ++i) {
        if (i < size) {
            tmp[i] = elements[i];
        } else {
            tmp[i] = 0;
        }
    }
    ui::release(elements);
    elements = tmp;
}

big_integer &big_integer::mul_small(big_integer::ll value) {
    if (value < 0) {
        sign *= -1;
        value = std::abs(value);
    }
    big_integer::ll carry = 0LL;
    for (int i = 0; i < size; i++) {
        big_integer::ll cur = 1LL * elements[i] * value + carry;
        elements[i] = (uint)(cur % BASE);
        carry = cur / BASE;
    }
    if (carry > 0) {
        elements[size++] = (uint)carry;
    }
    return *this;
}

big_integer &big_integer::div_small(big_integer::ll value) {
    if (value < 0) {
        sign *= -1;
        value = std::abs(value);
    }
    big_integer::ll carry = 0LL;
    for (int i = size - 1; i >= 0; --i) {
        big_integer::ll cur = 1LL * elements[i] + carry * big_integer::BASE;
        elements[i] = (uint)(cur / value);
        carry = cur % value;
    }
    make_correct();
    check_sign();
    return *this;
}

big_integer operator*(big_integer a, big_integer const& b) {
    a *= b;
    return a;
}


big_integer operator/(big_integer a, big_integer const& b) {
    a /= b;
    return a;
}
big_integer operator%(big_integer a, big_integer const& b) {
    a %= b;
    return a;
}

big_integer operator<<(big_integer a, int b) {
    a <<= b;
    return a;
}
big_integer operator>>(big_integer a, int b) {
    a >>= b;
    return a;
}

std::ostream& operator<<(std::ostream& out, big_integer const& c) {
    out << to_string(c);
    return out;
}

big_integer operator&(big_integer a, big_integer const& b) {
    a &= b;
    return a;
}

big_integer operator|(big_integer a, big_integer const& b) {
    a |= b;
    return a;
}

big_integer operator^(big_integer a, big_integer const& b) {
    a ^= b;
    return a;
}
