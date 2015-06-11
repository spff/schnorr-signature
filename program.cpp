#include <iostream>
#include <stdio.h>
#include <gmp.h>
#include <stdarg.h>
using namespace std;

#define QLength 160
#define PLength 1024

#define BUFFER_SIZE BITLENTH/8

class BigInt{
    public:
        mpz_t value;

        BigInt(){
            mpz_init(value);
        }

        ~BigInt(){
            mpz_clear(value);
        }
};

BigInt GeneratePrime(int BITLENTH){

    BigInt bi;
    char buf[BUFFER_SIZE];
    int i;
    mpz_t tmp1; mpz_init(tmp1);

    srand(time(NULL));


    // Set the bits of tmp randomly
    for(i = 0; i < BUFFER_SIZE; i++)
        buf[i] = rand() % 0xFF;
    // Set the top two bits to 1 to ensure int(tmp) is relatively large
    buf[0] = 0xC0;
    // Set the bottom bit to 1 to ensure int(tmp) is odd (better for finding primes)
    buf[BUFFER_SIZE - 1] |= 0x01;
    // Interpret this char buffer as an int
    mpz_import(tmp1, BUFFER_SIZE, 1, sizeof(buf[0]), 0, 0, buf);
    // Pick the next prime starting from that random number
    mpz_nextprime(bi.value, tmp1);

    return bi;
}

int main(){
    BigInt q = GeneratePrime(QLength);
    gmp_printf ("%Zd\n\n", q.value);

    BigInt p = GeneratePrime(PLength);
    while ( (p-1)%q!=0 ) {
        p = GeneratePrime(PLength);
    }
    
    gmp_printf ("%Zd\n", p.value);

    return 0;
}
