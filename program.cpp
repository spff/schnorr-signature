#include <iostream>
#include <stdio.h>
#include <gmp.h>
#include <stdarg.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string.h>
#include <ostream>

using namespace std;

#include <openssl/sha.h>

#define QLength 160
#define PLength 2014

#define BUFFER_SIZE BITLENTH/8

string sha256(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

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

        mpz_powm(a.value, temp.value, times.value, p.value);
        if(mpz_cmp_si(a.value, 1) > 0)
            break;
        mpz_add_ui(temp.value, temp.value, 1);
    }

    gmp_printf("a = % Zd\n\nt = %Zd\n\n", a.value, temp.value);

//for verification
mpz_powm(temp.value, a.value, q.value, p.value);
gmp_printf("should be one = % Zd\n\n", temp.value);



    int SecretKeyLength = rand() % (QLength - 2) +1;
    BigInt SecretKey = GeneratePrime(SecretKeyLength);
    gmp_printf("SecretKey = % Zd\n\n", SecretKey.value);


    BigInt V;
    mpz_powm(V.value, a.value, SecretKey.value, p.value);
    gmp_printf("V = % Zd\n\n", V.value);

    char * tmp = mpz_get_str(NULL,10,V.value);
    string Str = tmp;


string M = "I AM A SECRET FILE";

    string e = sha256(M + Str);
    cout << "e = " << e << endl;

    int RLength = rand() % (QLength - 2) +1;
    BigInt R = GeneratePrime(RLength);
    gmp_printf("SecretKey = % Zd\n\n", R.value);

    BigInt X;
    mpz_powm(X.value, a.value, R.value, p.value);
    gmp_printf("X = % Zd\n\n", X.value);


    BigInt Y, hihi;

    uint8_t input[e.length()];

    for(int i = 0;i < e.length();i+=2){

        input[i] = (unsigned int)e[i]*16 + (unsigned int)e[i+1];
    }
cout << e.length();
    mpz_import(hihi.value, e.length()/3, 1, sizeof(input[0]), 0, 0, input);

gmp_printf("e = hihi = % Zd\n\n", hihi.value);


    mpz_mul (temp.value, SecretKey.value, hihi.value);
    mpz_add (temp.value, R.value, temp.value);
    mpz_mod (Y.value, temp.value, q.value);



//(A * B) mod C = (A mod C * B mod C) mod C

    BigInt Xp, temp2;
    mpz_powm(temp.value, a.value, Y.value, p.value);
    mpz_powm(temp2.value, V.value, hihi.value, p.value);
    mpz_mul (temp.value, temp.value, temp2.value);
    mpz_mod (Xp.value, temp.value, p.value);

    tmp = mpz_get_str(NULL,10,Xp.value);
    Str = tmp;
    string ep = sha256(M + Str);
    cout << "ep = " << ep << endl << endl << " = e = " << e;

    return 0;
}
