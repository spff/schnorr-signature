#include <iostream>
#include <stdio.h>
#include <gmp.h>
#include <stdarg.h>
using namespace std;

#define QLength 160
#define PLength 2014

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

BigInt GenerateNBitIOOOOOI(int BITLENTH){

    BigInt bi;
    char buf[BUFFER_SIZE];
    int i;
    mpz_t tmp1; mpz_init(tmp1);

    // Set the bits of tmp to 0
    for(i = 0; i < BUFFER_SIZE; i++)
        buf[i] = 0x00;
    // Set the top bit to 1
    buf[0] = 0x80;
    // Set the bottom bit to 1 to ensure int(tmp) is odd (better for finding primes)
    buf[BUFFER_SIZE - 1] |= 0x01;
    // Interpret this char buffer as an int
    mpz_import(bi.value, BUFFER_SIZE, 1, sizeof(buf[0]), 0, 0, buf);

    return bi;
}

int main(){
    BigInt q = GeneratePrime(QLength);
    gmp_printf("q = %Zd\n\n", q.value);

    BigInt temp = GenerateNBitIOOOOOI(PLength), times, p;
    mpz_cdiv_q(times.value, temp.value, q.value);
    while(true){
        mpz_mul(p.value, q.value, times.value);
        mpz_add_ui(p.value, p.value, 1);
        if(mpz_probab_prime_p(p.value, 25) > 0)
            break;
        mpz_add_ui(times.value, times.value, 1);
    }

    gmp_printf("times = % Zd\n\np = %Zd\n\n", times.value, p.value);

    BigInt a;
    mpz_set_ui(temp.value, 3);
    while(true){
        /*int TLength = rand() % (PLength - 2) +1;
        BigInt T = GeneratePrime(TLength);
        */
        mpz_powm(a.value, temp.value, times.value, p.value);
        if(mpz_cmp_si(a.value, 1) > 0)
            break;
        mpz_add_ui(temp.value, temp.value, 1);
    }

    gmp_printf("a = % Zd\n\nt = %Zd\n\n", a.value, temp.value);

mpz_powm(temp.value, a.value, q.value, p.value);
gmp_printf("should be one = % Zd\n\n", temp.value);


    return 0;
}
