#include <iostream>
#include <tuple>

#include "bigints.hpp"

int main()
{
    //additionTest();
    //multiplicationTest();

//    const BigInt N_TWO{ -2 };
//    const BigInt N_THREE{ -3 };
//    const BigInt SIX{ 6 };
//
//    std::cout << N_THREE << ' ' << N_TWO << ' ' << SIX << ' ' << (N_THREE*N_TWO) << ' ' << (N_THREE * N_TWO == SIX) << '\n';

    std::tuple<BigInts::BigInt, BigInts::BigInt> x = BigInts::divMod(329, 87);
    std::cout << std::get<0>(x) << ' ' << std::get<1>(x) << '\n';

	std::cout << BigInts::BigInt{ 105 } << '\n';

    return 0;
}
