#include <iostream>
//#include <bitset>
//#include <cmath>
#include <cassert>
#include <climits>
#include <tuple>
#include <string>

#include "bigints.hpp"

#define ABS_M(a) (a < 0 ? -a : a)



namespace BigInts
{
    BigInt::BigInt(digit_t *list, len_t len) // Make a BigInt from a list
    {
        m_len = len; // Set the length of the integer
        m_digits = new digit_t[ABS_M(len)]; // Assign memory for the digits

        for (len_t i{ 0 }; i < ABS_M(len); ++i) // for every number in the list
            m_digits[i] = list[i]; // copy it over to the list of digits

        delete[] list;
    }
    BigInt::BigInt() // 0
    { m_digits = nullptr; m_len = 0; }
    BigInt::BigInt(const BigInt& other) // copy another BigInt
    {
        m_len = other.m_len;
        m_digits = new digit_t[ABS_M(m_len)];

        for (len_t i{ 0 }; i < ABS_M(m_len); ++i)
            m_digits[i] = other.m_digits[i];
    }
    BigInt& BigInt::operator=(const BigInt& other) // copy asygnment
    {
        delete[] m_digits;
        m_len = other.m_len;
        m_digits = new digit_t[ABS_M(m_len)];
        for (len_t i{ 0 }; i < ABS_M(m_len); ++i)
            m_digits[i] = other.m_digits[i];
        return *this;
    }
    BigInt::BigInt(int64 i) // make a BigInt form an integer
    {
        if (i == 0)
        { m_digits = nullptr; m_len = 0; } // 0
        else if (-DIGIT_MAX_G < i && i < DIGIT_MAX_G) // The number has one "digit"
        {
            m_digits = new digit_t[1]; // one digit
            m_len = (i < 0 ? -1 : 1); // a negative length means a negative number
            m_digits[0] = ABS_M(i); // assign the digit
        }
        else // more than one digit
        {
            bool neg = i < 0; // Is the number negative?
            i = ABS_M(i); // It is easier to work with positive numbers

            len_t len = 0; // The amount of digits in the number
            int64 tmp = i;
            while (tmp > 0)
            { ++len; tmp >>= DIGIT_BITS_G; } // calculate the # of digits needed

            m_len = (neg ? -len : len); // a negative length means a negative number
            m_digits = new digit_t[len]; // allocate memory for the digits

            for (len_t j{ 0 }; j < len; ++j) // assign the digits
            {
                m_digits[j] = i%DIGIT_MAX_G;
                i >>= DIGIT_BITS_G;
            }

            assert (i == 0 && "There should be no remainder!");
        }
    }
    BigInt::~BigInt()
    {
        if (m_digits != nullptr) delete[] m_digits; // de-allocate memory
    }

    int64 BigInt::toInt64() const
    {
        assert (ABS_M(m_len) <= 3 && "Can only convert BigInts with Max size of 64 bits to int64");
        if (ABS_M(m_len) == 3) assert (!(m_digits[0] == DIGIT_MAX_G && m_digits[1] == DIGIT_MAX_G
                                         && m_digits[2] >= (LLONG_MAX >> (DIGIT_BITS_G*2)))
                                       && "Can't convert an BigInt larger than 2^63-1 to a int64");

        if (m_len == 0) return 0;
        else
        {
            len_t len = ABS_M(m_len);
            int64 r = 0;
            for (int i{ 0 }; i < len; i++) r += m_digits[i]*(int64)pow(DIGIT_MAX_G, i);
            return (m_len < 0 ? -r : r);
        }
    }
    bool BigInt::toBool() const
    {
        return m_len != 0;
    }

    BigInt BigInt::operator-() const
    {
        BigInt r{ *this }; // Make a copy
        r.m_len = -r.m_len; // negate the length
        return r;
    }
    BigInt& BigInt::operator++()
    {
        (*this) = (*this) + (BigInt)1; // add one to this
        return *this; // return this
    }

    std::string BigInt::toStr() const
    {
        BigInt i{ *this };
        std::string s = "";

        if (!i.toBool()) s = "0"; // if i == 0
        if (i < 0)
        {
            s = "-";
            i = -i;
        }
        while (i > 0)
        {
            s.insert(0, 1, '0' + (char)((i % 10).toInt64()));
            i = i/10;
        }

        return s;
    }

    std::ostream& operator<<(std::ostream& out, const BigInt& i)
    {
        //for (int j{ 0 }; j < ABS_M(i.m_len); ++j) out << i.m_digits[j] << ' ';
        //out << '{' << i.m_len << '}';
        /*if (!i.toBool()) out << '0'; // if i == 0
        if (i < 0)
        {
            out << '-';
            i = -i;
        }
        while (i > 0)
        {
            out << (i % 10).toInt64();
            i = i/10;
        }*/
        out << i.toStr();
        return out;
    }

    BigInt operator+(const BigInt& a, const BigInt& b)
    {
        if (a.m_len < b.m_len) return b + a; // If a is garenteed to be > b, it simplifies the logic
        if (a == -b) return 0; // x-x = 0
        if (a.m_len == 0) return b; if (b.m_len == 0) return a; // x + 0 = x

        len_t ma = std::max(ABS_M(a.m_len), ABS_M(b.m_len));
        len_t mi = std::min(ABS_M(a.m_len), ABS_M(b.m_len));

        if (a.m_len < 0)
        return -(-a + -b); // -x + -y = -(x + y)
        else if (b.m_len > 0) // if both numbers are positive
        {
            len_t len = ma+1; // 9 + 9 = 18, but no x-digit numbers added together
            // produces an x+2-digit number
            digit_t *tmp = new digit_t[len]; // allocate memory for the new integer
            for (len_t i{ 0 }; i < len; ++i) tmp[i] = 0; // NB: clear the memory

            len_t i{ 0 };
            for (; i < mi; ++i) // for every digit in both numbers
            {
                tmp[i] += a.m_digits[i] + b.m_digits[i]; // add the sum of the digits to tmp
                if (tmp[i] >= DIGIT_MAX_G) // if the result is larger than the largest possible digit
                {
                    tmp[i] %= DIGIT_MAX_G; // make it small enough to fit
                    tmp[i+1] = 1; // & overflow
                }
            }
            for (; i < ma; ++i) // for the rest of the digits of the larger number
            {
                digit_t *digits = (ABS_M(a.m_len) > ABS_M(b.m_len) ? a.m_digits : b.m_digits);
                tmp[i] += digits[i]; // add the digit to tmp
                if (tmp[i] >= DIGIT_MAX_G) // if the result is larger than the largest possible digit
                {
                    tmp[i] %= DIGIT_MAX_G; // make it small enough to fit
                    tmp[i+1] = 1; // & overflow
                }
            }

            while (len != 0 && tmp[ABS_M(len)-1] == 0) // avoid leading zeroes
            {
                if (len > 0) --len;
                if (len < 0) ++len;
            }

            return { tmp, len }; // return the integer
        }
        else if (a > -b) // abs(a) > abs(b); result > 0
        {
            len_t len = ma;
            digit_t *tmp = new digit_t[len];
            for (len_t i{ 0 }; i < len; ++i) tmp[i] = 0;

            len_t i{ 0 };
            for (; i < mi; ++i)
            {
                tmp[i] += a.m_digits[i] - b.m_digits[i];
                if (tmp[i] < 0)
                {
                    tmp[i] += DIGIT_MAX_G;
                    tmp[i+1] = -1;
                }
            }
            for (; i < ma; ++i)
            {
                tmp[i] += a.m_digits[i];
                if (tmp[i] < 0)
                {
                    tmp[i] += DIGIT_MAX_G;
                    tmp[i+1] = -1;
                }
            }

            while (len != 0 && tmp[ABS_M(len)-1] == 0) // avoid leading zeroes
            {
                if (len > 0) --len;
                if (len < 0) ++len;
            }

            return { tmp, len };
        }
        else // abs(a) < abs(b); result < 0
        {
            return -(-a + -b); // a - b = -(b - a)
        }
    }

    BigInt operator*(BigInt a, BigInt b)
    {
        if (a == 0 || b == 0) return { 0 };
        if (a < 0)
        {
            if (b < 0) return (-a) * (-b);
            return -((-a) * b);
        }
        if (b < 0) return -(a * (-b));
        //std::cout << '*' << a << ' ' << b;
        BigInt result{ 0 };

        //int i{ 0 };
        while (b.toBool())
        {
            //std::cout << a << ' ' << b << ' ' << result << ' ' << (b&1) << '\n';
            if ((b & 1).toBool()) result += a;
            //std::cout << " | " << (b&1) << ' ' << result;
            a += a;
            b >>= 1;
            //++i;
            //if (i >= 10) break;
            //std::cout << ' ' << a << ' ' << b;
        }

        //std::cout << '*';
        return result;
    }

    std::tuple<BigInt, BigInt> divMod(BigInt a, BigInt b)
    {
        assert (b.toBool() && "Can't devide by zero!");
        if (a.m_len < 0 && b.m_len < 0)
        { std::tuple<BigInt, BigInt> r = divMod(-a, -b); return std::make_tuple(std::get<0>(r), -std::get<1>(r)); }
        if (a.m_len < 0)
        { std::tuple<BigInt, BigInt> r = divMod(-a, -b); return std::make_tuple(-std::get<0>(r), std::get<1>(r)); }
        if (b.m_len < 0)
        { std::tuple<BigInt, BigInt> r = divMod(-a, -b); return std::make_tuple(-std::get<0>(r), -std::get<1>(r)); }
        if (b == 1) return std::make_tuple(a, BigInt{});
        if (a == b) return std::make_tuple(BigInt{ 1 }, BigInt{});
        if (a < b) return std::make_tuple(BigInt{}, a);

        BigInt n{ 0 };

        while (b < a)
        {
            ++n;
            a = a-b;
        }

        return { n, a };
    }

    bool operator==(const BigInt& a, const BigInt& b)
    {
        if (a.m_len != b.m_len) return false; // compare the length of the integers

        for (len_t i{ 0 }; i < a.m_len; ++i)
        if (a.m_digits[i] != b.m_digits[i]) return false; // compare the digits

        return true;
    }
    bool operator<(const BigInt& a, const BigInt& b)
    {
        if (a.m_len < b.m_len) return true; // comepare the lengths
        if (a.m_len > b.m_len) return false;

        for (len_t i{ ABS_M(a.m_len)-1 }; i >= 0; --i) // compare the digits
        {
            if (a.m_digits[i] < b.m_digits[i]) return true;
            if (a.m_digits[i] > b.m_digits[i]) return false;
        }

        return false;
    }
    bool operator>(const BigInt& a, const BigInt& b)
    {
        if (a.m_len < b.m_len) return false;
        if (a.m_len > b.m_len) return true;

        for (len_t i{ ABS_M(a.m_len)-1 }; i >= 0; --i)
        {
            if (a.m_digits[i] < b.m_digits[i]) return false;
            if (a.m_digits[i] > b.m_digits[i]) return true;
        }

        return false;
    }

    BigInt operator&(BigInt a, const BigInt& b)
    {
        if (a.m_len >= 0 && b.m_len >= 0)
        {
            for (len_t i{ 0 }; i < std::min(a.m_len, b.m_len); ++i)
            a.m_digits[i] &= b.m_digits[i];
            for (len_t i{ std::min(a.m_len, b.m_len) }; i < a.m_len; ++i)
            a.m_digits[i] = 0;
        }

        while (a.m_len != 0 && a.m_digits[ABS_M(a.m_len)-1] == 0) // avoid leading zeroes
        {
            if (a.m_len > 0) --a.m_len;
            if (a.m_len < 0) ++a.m_len;
        }

        return a;
    }

    BigInt operator>>(BigInt a, BigInt b)
    {
        assert ((b > 0 || b == 0) && "Cannot shift by a negative amount!");
        if (a < b) return -((-a) >> b);
        //std::cout << ".." << a << ' ' << b;

        int big_shifts = 0;
        while (b > DIGIT_BITS_G || b == DIGIT_BITS_G)
        {
            ++big_shifts; b = b - (BigInt)DIGIT_BITS_G;
            if (big_shifts >= a.m_len)
            {
                return {};
            }
        }
        len_t len = a.m_len - big_shifts;
        digit_t *tmp = new digit_t[len];
        for (len_t i{ big_shifts }; i < a.m_len; ++i)
        tmp[i-big_shifts] = a.m_digits[i];

        //std::cout << ' ' << len << ' ' << big_shifts << '\n';

        digit_t tmp_{ 0 };
        for (len_t i{ len-1 }; i >= 0; --i)
        {
            digit_t tmp__ = tmp[i] & ((digit_t)pow(2, b.toInt64()+1) - 1);
            tmp[i] >>= b.toInt64();
            tmp[i] |= (tmp_ << (DIGIT_BITS_G-b.toInt64()));
            tmp_ = tmp__;
            //tmp[i-1] = tmp[i] & ((digit_t)pow(2, b.toInt64()+1) - 1);
            //tmp[i] >>= (digit_t)b;
        }

        while (len != 0 && tmp[ABS_M(len)-1] == 0)
        {
            if (len > 0) --len;
            if (len < 0) ++len;
        }

        return { tmp, len };
    }



    BigInt operator/(const BigInt& a, const BigInt& b)
    {
        return std::get<0>(divMod(a, b));
    }
    BigInt operator%(const BigInt& a, const BigInt& b)
    {
        return std::get<1>(divMod(a, b));
    }

    BigInt& operator+=(BigInt& a, const BigInt& b)
    {
        return (a = a+b);
    }
    BigInt& operator>>=(BigInt& a, const BigInt& b)
    {
        return (a = a>>b);
    }
    bool operator!=(const BigInt& a, const BigInt& b)
    {
        return !(a == b);
    }
    BigInt operator-(const BigInt& a, const BigInt& b)
    { return a + -b; } // a - b = a + -b



    void additionTest()
    {
        const BigInt ZERO{ 0 };
        const BigInt ONE{ 1 };
        const BigInt TWO{ 2 };
        const BigInt THREE{ 3 };
        const BigInt NINE{ DIGIT_MAX_G-1 };
        const BigInt TEN{ DIGIT_MAX_G };
        const BigInt ELEVEN{ DIGIT_MAX_G+1 };
        const BigInt EIGHTEEN{ DIGIT_MAX_G*2 - 2 };
        digit_t *tmp = new digit_t[2];
        tmp[0] = DIGIT_MAX_G-1; tmp[1] = DIGIT_MAX_G-1;
        const BigInt NINETY_NINE{ tmp, 2 };
        tmp = new digit_t[3];
        tmp[0] = 0; tmp[1] = 0; tmp[2] = 1;
        const BigInt HUNDRED{ tmp, 3 };

        const BigInt N_ONE{ -1 };
        const BigInt N_TWO{ -2 };
        const BigInt N_THREE{ -3 };
        const BigInt N_NINE{ -DIGIT_MAX_G+1 };
        const BigInt N_TEN{ -DIGIT_MAX_G };
        const BigInt N_ELEVEN{ -DIGIT_MAX_G-1 };
        const BigInt N_EIGHTEEN{ -DIGIT_MAX_G*2 + 2 };
        tmp = new digit_t[2];
        tmp[0] = DIGIT_MAX_G-1; tmp[1] = DIGIT_MAX_G-1;
        const BigInt N_NINETY_NINE{ tmp, -2 };
        tmp = new digit_t[3];
        tmp[0] = 0; tmp[1] = 0; tmp[2] = 1;
        const BigInt N_HUNDRED{ tmp, -3 };

        std::cout << std::boolalpha;
        // a > 0, b > 0
        // len(a) > len(b)
        std::cout << "10 + 1 == 11     : " << (TEN + ONE == ELEVEN) << '\n'; // len(a + b) == len(a)
        std::cout << "99 + 1 == 100    : " << (NINETY_NINE + ONE == HUNDRED) << '\n'; // len(a + b) == len(a)+1
        // len(a) == len(b)
        // a > b
        std::cout << "2 + 1 == 3       : " << (TWO + ONE == THREE) << '\n'; // len(a + b) == len(a)
        std::cout << "9 + 1 == 10      : " << (NINE + ONE == TEN) << '\n'; // len(a + b) == len(a)+1
        // a == b
        std::cout << "1 + 1 == 2       : " << (ONE + ONE == TWO) << '\n'; // len(a + b) == len(a)
        std::cout << "9 + 9 == 18      : " << (NINE + NINE == EIGHTEEN) << '\n'; // len(a + b) == len(a)+1
        // a < b
        std::cout << "1 + 2 == 3       : " << (ONE + TWO == THREE) << '\n'; // len(a + b) == len(a)
        std::cout << "1 + 9 == 10      : " << (ONE + NINE == TEN) << '\n'; // len(a + b) == len(a)+1
        // len(a) < len(b)
        std::cout << "1 + 10 == 11     : " << (ONE + TEN == ELEVEN) << '\n'; // len(a + b) == len(b)
        std::cout << "1 + 99 == 100    : " << (ONE + NINETY_NINE == HUNDRED) << '\n'; // len(a + b) == len(b)+1
        std::cout << "1 + 0 == 1       : " << (ONE + ZERO == ONE) << '\n'; // a > 0, b == 0
        // a > 0, b < 0
        // len(a) > len(ABS(b))
        std::cout << "11 + -1 == 10    : " << (ELEVEN + N_ONE == TEN) << '\n'; // len(a + b) == len(a)
        std::cout << "100 + -1 == 99   : " << (HUNDRED + N_ONE == NINETY_NINE) << '\n'; // len(a + b) < len(a)
        // len(a) == len(ABS(b))
        // a > ABS(b)
        std::cout << "3 + -1 == 2      : " << (THREE + N_ONE == TWO) << '\n'; // len(a + b) == len(a)
        std::cout << "11 + -10 == 1    : " << (ELEVEN + N_TEN == ONE) << '\n'; // len(a + b) < len(a)
        std::cout << "1 + -1 == 0      : " << (ONE + N_ONE == ZERO) << '\n'; // a == ABS(b)
        // a < ABS(b)
        std::cout << "1 + -3 == -2     : " << (ONE + N_THREE == N_TWO) << '\n'; // len(a + b) == len(a)
        std::cout << "1 + -10 == -9    : " << (ONE + N_TEN == N_NINE) << '\n'; // len(a + b) < len(a)
        // len(a) < len(ABS(b))
        std::cout << "1 + -11 == -10   : " << (ONE + N_ELEVEN == N_TEN) << '\n'; // len(a + b) == len(b)
        std::cout << "1 + -100 == -99  : " << (ONE + N_HUNDRED == N_NINETY_NINE) << '\n'; // len(a + b) < len(b)
        std::cout << "0 + 1 == 1       : " << (ZERO + ONE == ONE) << '\n'; // a == 0, b > 0
        std::cout << "0 + 0 == 0       : " << (ZERO + ZERO == ZERO) << '\n'; // a == 0, b == 0
        std::cout << "0 + -1 == -1     : " << (ZERO + N_ONE == N_ONE) << '\n'; // a == 0, b < 0
        // a < 0, b > 0
        // len(ABS(a)) > len(b)
        std::cout << "-11 + 1 == -10   : " << (N_ELEVEN + ONE == N_TEN) << '\n'; // len(a + b) == len(a)
        std::cout << "-100 + 1 == -99  : " << (N_HUNDRED + ONE == N_NINETY_NINE) << '\n'; // len(a + b) < len(a)
        // len(ABS(a)) == len(b)
        // ABS(a) > b
        std::cout << "-3 + 1 == -2     : " << (N_THREE + ONE == N_TWO) << '\n'; // len(a + b) == len(a)
        std::cout << "-10 + 1 == -9    : " << (N_TEN + ONE == N_NINE) << '\n'; // len(a + b) == len(a)+1
        std::cout << "-1 + 1 == 0      : " << (N_ONE + ONE == ZERO) << '\n'; // ABS(a) == b
        // ABS(a) < b
        std::cout << "-1 + 3 == 2      : " << (N_ONE + THREE == TWO) << '\n'; // len(a + b) == len(ABS(a))
        std::cout << "-1 + 10 == 9     : " << (N_ONE + TEN == NINE) << '\n'; // len(a + b) == len(ABS(a))+1
        // len(ABS(a)) < len(b)
        std::cout << "-1 + 11 == 10    : " << (N_ONE + ELEVEN == TEN) << '\n'; // len(a + b) == len(b)
        std::cout << "-1 + 100 == 99   : " << (N_ONE + HUNDRED == NINETY_NINE) << '\n'; // len(a + b) < len(b)
        std::cout << "-1 + 0 == -1     : " << (N_ONE + ZERO == N_ONE) << '\n'; // a < 0, b == 0
        // a < 0, b < 0
        // len(a) > len(b)
        std::cout << "-10 + -1 == -11  : " << (N_TEN + N_ONE == N_ELEVEN) << '\n'; // len(a + b) == len(a)
        std::cout << "-99 + -1 == -100 : " << (N_NINETY_NINE + N_ONE == N_HUNDRED) << '\n'; // len(a + b) < len(a)
        // len(a) == len(b)
        // ABS(a) > ABS(b)
        std::cout << "-2 + -1 == -3    : " << (N_TWO + N_ONE == N_THREE) << '\n'; // len(a + b) == len(a)
        std::cout << "-9 + -1 == -10   : " << (N_NINE + N_ONE == N_TEN) << '\n'; // len(a + b) < len(a)
        // a == b
        std::cout << "-1 + -1 == -2    : " << (N_ONE + N_ONE == N_TWO) << '\n'; // len(a + b) == len(a)
        std::cout << "-9 + -9 == -18   : " << (N_NINE + N_NINE == N_EIGHTEEN) << '\n'; // len(a + b) < len(a)
        // ABS(a) < ABS(b)
        std::cout << "-1 + -2 == -3    : " << (N_ONE + N_TWO == N_THREE) << '\n'; // len(a + b) == len(a)
        std::cout << "-1 + -9 == -10   : " << (N_ONE + N_NINE == N_TEN) << '\n'; // len(a + b) < len(a)
        // len(a) < len(b)
        std::cout << "-1 + -10 == -11  : " << (N_ONE + N_TEN == N_ELEVEN) << '\n'; // len(a + b) == len(b)
        std::cout << "-1 + -99 == -100 : " << (N_ONE + N_NINETY_NINE == N_HUNDRED) << '\n'; // len(a + b) < len(b)
    }

    void multiplicationTest()
    {
        const BigInt ZERO{ 0 };
        const BigInt ONE{ 1 };
        const BigInt TWO{ 2 };
        const BigInt THREE{ 3 };
        const BigInt FOUR{ 4 };
        const BigInt SIX{ 6 };
        const BigInt FIVE{ DIGIT_MAX_G >> 1 };
        const BigInt TEN{ DIGIT_MAX_G };
        const BigInt TWENTY{ DIGIT_MAX_G*2 };
        digit_t *tmp = new digit_t[2];
        tmp[0] = 0; tmp[1] = DIGIT_MAX_G >> 2;
        const BigInt TWENTY_FIVE{ tmp, 2 };
        tmp = new digit_t[2];
        tmp[0] = 0; tmp[1] = DIGIT_MAX_G>>1;
        const BigInt FIFTY{ tmp, 2 };
        tmp = new digit_t[3];
        tmp[0] = 0; tmp[1] = 0; tmp[2] = 1;
        const BigInt HUNDRED{ tmp, 3 };

        const BigInt N_ONE{ -1 };
        const BigInt N_TWO{ -2 };
        const BigInt N_THREE{ -3 };
        const BigInt N_FOUR{ -4 };
        const BigInt N_SIX{ -6 };
        const BigInt N_FIVE{ -(DIGIT_MAX_G >> 1) };
        const BigInt N_TEN{ -DIGIT_MAX_G };
        const BigInt N_TWENTY{ -DIGIT_MAX_G*2 };
        tmp = new digit_t[2];
        tmp[0] = 0; tmp[1] = DIGIT_MAX_G >> 2;
        const BigInt N_TWENTY_FIVE{ tmp, -2 };
        tmp = new digit_t[2];
        tmp[0] = 0; tmp[1] = DIGIT_MAX_G>>1;
        const BigInt N_FIFTY{ tmp, -2 };
        tmp = new digit_t[3];
        tmp[0] = 0; tmp[1] = 0; tmp[2] = 1;
        const BigInt N_HUNDRED{ tmp, -3 };

        std::cout << std::boolalpha;
        // a > 0, b > 0
        // len(a) > len(b)
        std::cout << "10 * 2 == 20     : " << (TEN * TWO == TWENTY) << '\n'; // len(a * b) == len(a)
        std::cout << "50 * 2 == 100    : " << (FIFTY * TWO == HUNDRED) << '\n'; // len(a * b) == len(a)+1
        // len(a) == len(b)
        // a > b
        std::cout << "3 * 2 == 6       : " << (THREE * TWO == SIX) << '\n'; // len(a * b) == len(a)
        std::cout << "5 * 2 == 10      : " << (FIVE * TWO == TEN) << '\n'; // len(a * b) == len(a)+1
        // a == b
        std::cout << "2 * 2 == 4       : " << (TWO * TWO == FOUR) << '\n'; // len(a * b) == len(a)
        std::cout << "5 * 5 == 25      : " << (FIVE * FIVE == TWENTY_FIVE) << '\n'; // len(a * b) == len(a)+1
        // a < b
        std::cout << "2 * 3 == 6       : " << (TWO * THREE == SIX) << '\n'; // len(a * b) == len(a)
        std::cout << "2 * 5 == 10      : " << (TWO * FIVE == TEN) << '\n'; // len(a * b) == len(a)+1
        // len(a) < len(b)
        std::cout << "2 * 10 == 20     : " << (TWO * TEN == TWENTY) << '\n'; // len(a * b) == len(b)
        std::cout << "2 * 50 == 100    : " << (TWO * FIFTY == HUNDRED) << '\n'; // len(a * b) == len(b)+1
        std::cout << "1 * 0 == 0       : " << (ONE * ZERO == ZERO) << '\n'; // a > 0, b == 0
        // a > 0, b < 0
        // len(a) > len(ABS(b))
        std::cout << "10 * -2 == -20   : " << (TEN * N_TWO == N_TWENTY) << '\n'; // len(a * b) == len(a)
        std::cout << "50 * -2 == -100  : " << (FIFTY * N_TWO == N_HUNDRED) << '\n'; // len(a * b) > len(a)
        // len(a) == len(ABS(b))
        // a > ABS(b)
        std::cout << "3 * -2 == -6     : " << (THREE * N_TWO == N_SIX) << '\n'; // len(a * b) == len(a)
        std::cout << "50 * -2 == -100  : " << (FIFTY * N_TWO == N_HUNDRED) << '\n'; // len(a * b) > len(a)
        // a == ABS(b)
        std::cout << "2 * -2 == -4     : " << (TWO * N_TWO == N_FOUR) << '\n'; // len(a * b) == len(a)
        std::cout << "5 * -5 == -25    : " << (FIVE * N_FIVE == N_TWENTY_FIVE) << '\n'; // len(a * b) > len(a)
        // a < ABS(b)
        std::cout << "2 * -3 == -6     : " << (TWO * N_THREE == N_SIX) << '\n'; // len(a * b) == len(a)
        std::cout << "2 * -5 == -10    : " << (TWO * N_FIVE == N_TEN) << '\n'; // len(a * b) < len(a)
        // len(a) < len(ABS(b))
        std::cout << "2 * -10 == -20   : " << (TWO * N_TEN == N_TWENTY) << '\n'; // len(a * b) == len(b)
        std::cout << "2 * -50 == -100  : " << (TWO * N_FIFTY == N_HUNDRED) << '\n'; // len(a * b) < len(b)
        std::cout << "0 * 1 == 0       : " << (ZERO * ONE == ZERO) << '\n'; // a == 0, b > 0
        std::cout << "0 * 0 == 0       : " << (ZERO * ZERO == ZERO) << '\n'; // a == 0, b == 0
        std::cout << "0 * -1 == 0      : " << (ZERO * N_ONE == ZERO) << '\n'; // a == 0, b < 0
        // a < 0, b > 0
        // len(ABS(a)) > len(b)
        std::cout << "-10 * 2 == -20   : " << (N_TEN * TWO == N_TWENTY) << '\n'; // len(a * b) == len(a)
        std::cout << "-50 * 2 == -100  : " << (N_FIFTY * TWO == N_HUNDRED) << '\n'; // len(a * b) > len(a)
        // len(ABS(a)) == len(b)
        // ABS(a) > b
        std::cout << "-3 * 2 == -6     : " << (N_THREE * TWO == N_SIX) << '\n'; // len(a * b) == len(a)
        std::cout << "-50 * 2 == -100  : " << (N_FIFTY * TWO == N_HUNDRED) << '\n'; // len(a * b) > len(a)
        // ABS(a) == b
        std::cout << "-2 * 2 == -4     : " << (N_TWO * TWO == N_FOUR) << '\n'; // len(a * b) == len(a)
        std::cout << "-5 * 5 == -25    : " << (N_FIVE * FIVE == N_TWENTY_FIVE) << '\n'; // len(a * b) > len(a)
        // ABS(a) < b
        std::cout << "-2 * 3 == -6     : " << (N_TWO * THREE == N_SIX) << '\n'; // len(a * b) == len(a)
        std::cout << "-2 * 5 == -10    : " << (N_TWO * FIVE == N_TEN) << '\n'; // len(a * b) < len(a)
        // len(ABS(a)) < len(b)
        std::cout << "-2 * 10 == -20   : " << (N_TWO * TEN == N_TWENTY) << '\n'; // len(a * b) == len(b)
        std::cout << "-2 * 50 == -100  : " << (N_TWO * FIFTY == N_HUNDRED) << '\n'; // len(a * b) < len(b)
        std::cout << "-1 * 0 == 0      : " << (N_ONE * ZERO == ZERO) << '\n'; // a < 0, b == 0
        // a < 0, b < 0
        // len(a) > len(b)
        std::cout << "-10 * -2 == 20   : " << (N_TEN * N_TWO == TWENTY) << '\n'; // len(a * b) == len(a)
        std::cout << "-50 * -2 == 100  : " << (N_FIFTY * N_TWO == HUNDRED) << '\n'; // len(a * b) < len(a)
        // len(a) == len(b)
        // ABS(a) > ABS(b)
        std::cout << "-3 * -2 == 6     : " << (N_THREE * N_TWO == SIX) << '\n'; // len(a * b) == len(a)
        std::cout << "-5 * -2 == 10    : " << (N_FIVE * N_TWO == TEN) << '\n'; // len(a * b) < len(a)
        // a == b
        std::cout << "-2 * -2 == 4     : " << (N_TWO * N_TWO == FOUR) << '\n'; // len(a * b) == len(a)
        std::cout << "-5 * -5 == 25    : " << (N_FIVE * N_FIVE == TWENTY_FIVE) << '\n'; // len(a * b) < len(a)
        // ABS(a) < ABS(b)
        std::cout << "-2 * -3 == 6     : " << (N_TWO * N_THREE == SIX) << '\n'; // len(a * b) == len(a)
        std::cout << "-2 * -5 == 10    : " << (N_TWO * N_FIVE == TEN) << '\n'; // len(a * b) < len(a)
        // len(a) < len(b)
        std::cout << "-2 * -10 == 20   : " << (N_TWO * N_TEN == TWENTY) << '\n'; // len(a * b) == len(b)
        std::cout << "-2 * -50 == 100  : " << (N_TWO * N_FIFTY == HUNDRED) << '\n'; // len(a * b) < len(b)
    }
}
