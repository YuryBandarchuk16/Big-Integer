//
//  big_integer.h
//  BigInteger
//
//  Created by Юрий Бондарчук on 6/3/17.
//  Copyright © 2017 Yury Bandarchuk. All rights reserved.
//

#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <vector>

struct big_integer
{
public:
    
    typedef long long ll;
    typedef unsigned int uint;
    
    static const ll BASE = (1LL << 31LL);
    static const ll LEFT_BORDER = -BASE;
    static const ll RIGHT_BORDER = BASE - 1LL;
    
    big_integer(); // done
    big_integer(big_integer const& other); // done
    big_integer(int a); // done
    explicit big_integer(std::string const& str); // done
    ~big_integer(); // done
    
    big_integer& operator=(big_integer const& other); // done
    
    big_integer& operator+=(big_integer const& rhs); // done
    big_integer& operator-=(big_integer const& rhs); // done
    big_integer& operator*=(big_integer const& rhs); // done
    big_integer& operator/=(big_integer const& rhs); // done
    big_integer& operator%=(big_integer const& rhs); // done
    
    big_integer& operator&=(big_integer const& rhs); // done
    big_integer& operator|=(big_integer const& rhs); // done
    big_integer& operator^=(big_integer const& rhs); // done
    
    big_integer& operator<<=(int rhs); // done
    big_integer& operator>>=(int rhs); // done
    
    big_integer operator+() const; // done
    big_integer operator-() const; // done
    big_integer operator~() const; // done
    
    big_integer& operator++(); // done
    big_integer operator++(int); // done
    
    big_integer& operator--(); // done
    big_integer operator--(int); // done
    
    friend bool operator==(big_integer const& a, big_integer const& b); // done
    friend bool operator!=(big_integer const& a, big_integer const& b); // done
    friend bool operator<(big_integer const& a, big_integer const& b);  // done
    friend bool operator>(big_integer const& a, big_integer const& b);  // done
    friend bool operator<=(big_integer const& a, big_integer const& b); // done
    friend bool operator>=(big_integer const& a, big_integer const& b); // done
    
    friend std::string to_string(big_integer const& a); // done
    
    // a < b : -1; a > b: +1, a == b: 0
    friend int compare_absolute_value(big_integer const& a, big_integer const& b); // done
    friend int compare(big_integer const& a, big_integer const& b); // done
    
private:
    void copy_on_write(); // done
    void turn_big_mode(); // done
    void resize(int new_size); // done
    void ensure_capacity(int size); // done
    void swap(big_integer& copy); // done
    void shift_left(int k); // done
    void shift_right(int k); // done
    void remove_zeroes(); // done
    void check_sign(); // done
    void make_correct(); // done
    big_integer& add(big_integer const& rhs); // done
    big_integer& sub(big_integer const& rhs); // done
    big_integer& add_small(big_integer::ll value); // done
    big_integer& sub_small(big_integer::ll value); // done
    big_integer& mul_small(big_integer::ll value); // done
    big_integer& div_small(big_integer::ll value); // done
    
    int size, capacity;
    union {
        uint *elements;
        long long small;
    };
    int sign;
    
    static const int POWER = 31;
};

big_integer operator+(big_integer a, big_integer const& b); // done
big_integer operator-(big_integer a, big_integer const& b); // done
big_integer operator*(big_integer a, big_integer const& b); // done
big_integer operator/(big_integer a, big_integer const& b); // done
big_integer operator%(big_integer a, big_integer const& b); // done

big_integer operator&(big_integer a, big_integer const& b); // done
big_integer operator|(big_integer a, big_integer const& b); // done
big_integer operator^(big_integer a, big_integer const& b); // done

big_integer operator<<(big_integer a, int b); // done
big_integer operator>>(big_integer a, int b); // done

bool operator==(big_integer const& a, big_integer const& b); // done
bool operator!=(big_integer const& a, big_integer const& b); // done
bool operator<(big_integer const& a, big_integer const& b); // done
bool operator>(big_integer const& a, big_integer const& b); // done
bool operator<=(big_integer const& a, big_integer const& b); // done
bool operator>=(big_integer const& a, big_integer const& b); // done

std::string to_string(big_integer const& a); // done
std::ostream& operator<<(std::ostream& s, big_integer const& a); // done

#endif // BIG_INTEGER_H
