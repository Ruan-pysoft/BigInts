#include <tuple>
#include <cmath>
#include <string>

#ifndef RUAN_BIGINTS_HPP
#define RUAN_BIGINTS_HPP

namespace BigInts
{
    using int64 = long long;

    using digit_t = int32_t;
    using len_t = int;
    constexpr int DIGIT_BITS_G = 30;
    constexpr int64 DIGIT_MAX_G = (int64)powl(2,DIGIT_BITS_G);

    class BigInt
    {
        digit_t *m_digits = nullptr;
        len_t m_len;

        BigInt(digit_t *list, len_t len); // Make a BigInt from a list
    public:
        BigInt(); // 0
        BigInt(const BigInt& other); // copy another BigInt
        BigInt& operator=(const BigInt& other); // copy asygnment
        BigInt(int64 i); // make a BigInt form an integer
        ~BigInt();

        int64 toInt64() const;
        bool toBool() const;
        std::string toStr() const;

        BigInt operator-() const;
        BigInt& operator++();

        //friend std::ostream& operator<<(std::ostream& out, const BigInt& i);
        friend BigInt operator+(const BigInt& a, const BigInt& b);
        friend BigInt operator*(BigInt a, BigInt b);
        friend std::tuple<BigInt, BigInt> divMod(BigInt a, BigInt b);
        friend bool operator==(const BigInt& a, const BigInt& b);
        friend bool operator<(const BigInt& a, const BigInt& b);
        friend bool operator>(const BigInt& a, const BigInt& b);
        friend BigInt operator&(BigInt a, const BigInt& b);
        friend BigInt operator>>(BigInt a, BigInt b);
        friend void additionTest();
        friend void multiplicationTest();
    };

    std::ostream& operator<<(std::ostream& out, const BigInt& i);
    std::tuple<BigInt, BigInt> divMod(BigInt a, BigInt b);

    BigInt operator/(const BigInt& a, const BigInt& b);
    BigInt operator%(const BigInt& a, const BigInt& b);
    BigInt& operator+=(BigInt& a, const BigInt& b);
    BigInt& operator>>=(BigInt& a, const BigInt& b);
    bool operator!=(const BigInt& a, const BigInt& b);
    BigInt operator-(const BigInt& a, const BigInt& b);
}
#endif
