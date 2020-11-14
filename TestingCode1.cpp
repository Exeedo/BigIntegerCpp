#include <iostream>
#include <stdio.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "BigInteger++.h"

unsignedBigInteger GCD(unsignedBigInteger x, unsignedBigInteger y) {
    // GCD(x, y) = GCD(y, x % y)
    if (x == 0)
        return y;
    if (y == 0)
        return x;
    while (y > 0) {
        x %= y;
        std::swap(x, y);
    }
    return x;
}

int main()
{
    printf("Compiled correctly!\n\n");

    printf("Checking variant operations:\n");
    unsignedBigInteger number1(1234);
    printf("number1 = ");
    number1.PrintAsDecimal();
    printf(" = ");
    number1.PrintAsHex('\n');

    unsignedBigInteger number2("5678");
    printf("number2 = ");
    number2.PrintAsDecimal();
    printf(" = ");
    number2.PrintAsHex('\n');

    unsignedBigInteger number3("0x1A3b", 16);
    printf("number3 = ");
    number3.PrintAsDecimal();
    printf(" = ");
    number3.PrintAsHex('\n');

    unsignedBigInteger number4(E18);
    number4 *= 22;
    printf("number4 = ");
    number4.PrintAsDecimal();
    printf(" = ");
    number4.PrintAsHex('\n');

    unsignedBigInteger number(HIGH_DWORD | LOW_DWORD), ONE(1);

    number <<= 64;
    number |= HIGH_DWORD | LOW_DWORD;

    number -= 4;

    ++number;
    number += 1;
    number += ONE;
    number = number + ONE;
    number = number + 1;

    printf("number5 = ");
    number.PrintAsDecimal();
    printf(" = ");
    number.PrintAsHex('\n');

    --number;
    printf("number6 = ");
    number.PrintAsDecimal();
    printf(" = ");
    number.PrintAsHex('\n');
    printf("------------------------\n\n");
    
    unsigned int exponent = 300;
    unsignedBigInteger power2[5] = { 1,1,1,1,1 };

    for (unsigned int i = 1; i <= exponent; i++) {
        power2[0] *= 2;
        power2[1] <<= 1;
        unsignedBigInteger Temp = power2[2];
        power2[2] += Temp;
    }

    power2[3] <<= exponent;
    power2[4] = 2;
    power2[4].FastPower(exponent);

    printf("Calculation of 2^%d:\n", exponent);

    printf("Using multiplication: ");
    power2[0].PrintAsDecimal('\n');

    printf("Using shifting      : ");
    power2[1].PrintAsDecimal('\n');

    printf("Using addition      : ");
    power2[2].PrintAsDecimal('\n');

    printf("Using shifting once : ");
    power2[3].PrintAsDecimal('\n');

    printf("Using power function: ");
    power2[4].PrintAsDecimal('\n');

    for (unsigned int i = 1; i <= exponent; i++) {
        power2[0] /= 2;
        power2[1] >>= 1;
    }
    power2[3] >>= exponent;

    if (power2[0] != 1) {
        printf("Division unsuccessful !\n");
        printf("Found : ");
        power2[0].PrintAsDecimal(' ');
        printf("instead of 1\n");
    }
    if (power2[1] != 1) {
        printf("Shifting unsuccessful !\n");
        printf("Found : ");
        power2[1].PrintAsDecimal(' ');
        printf("instead of 1\n");
    }
    if (power2[3] != 1) {
        printf("Shifting (once) unsuccessful !\n");
        printf("Found : ");
        power2[3].PrintAsDecimal(' ');
        printf("instead of 1\n");
    }
    printf("------------------------\n\n");
    
    printf("Printing factorials:\n");
    const unsigned int N_fact = 100;
    unsignedBigInteger fact[N_fact + 1];
    fact[0] = 1;
    for (int i = 1; i <= N_fact; i++) {
        fact[i] = fact[i - 1] * i;
        printf("%4d! = ", i);
        fact[i].PrintAsDecimal('\n');
    }
    printf("------------------------\n\n");

    printf("Printing the LCM of numbers from 1 to:\n");
    const unsigned int N_LCM = 1000;
    unsignedBigInteger LCM[N_LCM + 1];
    unsignedBigInteger g;
    LCM[1] = 1;
    for (int i = 2; i <= N_LCM; i++) {
        g = GCD(LCM[i - 1], unsignedBigInteger(i));
        LCM[i] = (LCM[i - 1] / g) * i;
        if (LCM[i] != LCM[i - 1]) {
            printf("%4d => ", i);
            LCM[i].PrintAsDecimal('\n');
        }
    }
    printf("------------------------\n\n");

    const unsigned int N_primes = 1000;
    bool isPrime[N_primes + 1];
    std::vector<unsigned int> primes;
    memset(isPrime, true, sizeof isPrime);
    isPrime[0] = false;
    isPrime[1] = false;
    for (unsigned long long i = 2; i <= N_primes; i++) {
        if (!isPrime[i]) continue;
        primes.push_back(i);
        for (unsigned long long j = i * i; j <= N_primes; j += i)
            isPrime[j] = false;
    }

    printf("Printing Primes upto %d:\n", N_primes);
    unsignedBigInteger product = 1;
    for (unsigned int p : primes) {
        printf("%d\n", p);
        product *= p;
    }
    printf("\nProduct of primes upto %d is:\n", N_primes);
    product.PrintAsDecimal('\n');
    printf("------------------------\n\n");

	printf("Calculating 2^(2^x):\n");
    unsignedBigInteger x(1), y;
    for (unsigned int i = 1, j = 0; j <= 12; i <<= 1, j++) {
        printf("x=%d, 2^%d =\n", j, i);
        y = x;
        y <<= i;
        y.PrintAsDecimal('\n');
        printf("\n");
    }
    return 0;
}
