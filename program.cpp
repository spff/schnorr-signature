/* install openssl, have gmp ./configure --enable-cxx
 *should create etc/ld.so.conf.d/gmp.conf  write "/usr/local/lib" then, run ldconfig
 * -lcrypto -lgmpxx -lgmp
 *
 *  todo    :make it work as batch
 */
#include <iostream>
#include <stdio.h>
#include <gmpxx.h>
#include <stdarg.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <string.h>
#include <ostream>
#include <thread>

#include <openssl/sha.h>

#define QLength 160
#define PLength 2014
#define BUFFER_SIZE BITLENTH/8
#define MAXSAVEDKEY 1
using namespace std;



class BigInt{
    public:
        mpz_t value;

        BigInt(){
            mpz_init(value);
        }

        ~BigInt(){
            mpz_clear(value);
        }
        string Outbase(int base){
            string Str{mpz_get_str(NULL, base, value)};
            return Str;
        }
        string Out(){
            string Str{mpz_get_str(NULL, 10, value)};
            return Str;
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



class KeyGenerator{
public:
    void start(){


    }
private:
    void GenerateKey(){


        mpz_class b;
        b = 456;
        cout << b;
        BigInt q = GeneratePrime(QLength);
        cout << "q = " << q.Out() << endl << endl;

        BigInt times, p, temp = GenerateNBitIOOOOOI(PLength);
        mpz_cdiv_q(times.value, temp.value, q.value);
        while(true){
            mpz_mul(p.value, q.value, times.value);
            mpz_add_ui(p.value, p.value, 1);
            if(mpz_probab_prime_p(p.value, 25) > 0)
                break;
            mpz_add_ui(times.value, times.value, 1);
        }
        cout << "times = " << times.Out() << endl << endl;
        cout << "p = " << p.Out() << endl << endl;

        BigInt a;
        mpz_set_ui(temp.value, 3);

        while(true){
            mpz_powm(a.value, temp.value, times.value, p.value);
            if(mpz_cmp_si(a.value, 1) > 0)
                break;
            mpz_add_ui(temp.value, temp.value, 1);
        }
        cout << "a = " << a.Out() << endl << endl;
        cout << "temp = " << temp.Out() << endl << endl;

    //for verification
    mpz_powm(temp.value, a.value, q.value, p.value);
    cout << "should be one = " << temp.Out() << endl << endl;


        int SecretKeyLength = rand() % (QLength - 2) +1;
        BigInt SecretKey = GeneratePrime(SecretKeyLength);
        cout << "SecretKey = " << SecretKey.Out() << endl << endl;

        BigInt V;
        mpz_powm(V.value, a.value, SecretKey.value, p.value);
        cout << "V = " << V.Out() << endl << endl;

    }

};

class Mission{
    private:
        int argc;
        char** argv;
    public:
        Mission(int argc, char* argv[]){
            this->argc = argc;
            this->argv = argv;
        }

        void start(){
            switch(Interact(argc, argv)){
                case 1:
                    Sign();
                    break;
                case 2:
                    Verify();
                    break;
                default:
                    return;
            }
        }
    private:

        int Interact(int argc, char* argv[]){

            string cmd;

            if(argc == 1){
                int act;
                while(true){
                    cout << "1)sign  2)verify 3)exit :";
                    getline(cin, cmd);
                    stringstream tempstream{cmd};
                    tempstream >> act;
                    if(act == 3)
                        return 0;
                    if((act == 1) || (act == 2))
                        break;
                }
            }

            else{
                while(true){
                    char split_char = ' ';
                    getline(cin, cmd);
                    istringstream split(cmd);
                    vector<string> tokens;
                    for (string each; getline(split, each, split_char); tokens.push_back(each));

                    if(tokens.size() !=3){
                        cerr << "invalid command.Type \"connect <host> <port>\"" << endl;
                        continue;
                    }

                    cmd = tokens[0];
                    if(cmd != "connect"){
                        cerr << "invalid command.Type \"connect <host> <port>\"" << endl;
                        continue;
                    }


                }
            }
        }

        void Sign(){

            char * tmp = mpz_get_str(NULL,10,V.value);
            string Str = tmp;


string M = "I AM A SECRET FILE";
cout << "M = " << M << endl << endl;

            string e = sha256(M + Str);
            cout << "e = " << e << endl;

            int RLength = rand() % (QLength - 2) +1;
            BigInt R = GeneratePrime(RLength);
            cout << "R = " << R.Out() << endl << endl;

            BigInt X;
            mpz_powm(X.value, a.value, R.value, p.value);
            cout << "X = " << X.Out() << endl << endl;

            BigInt Y, hihi;

            char input[e.length()];
            int tt;
            for(int i = 0;i < e.length();i++){
                mpz_mul_ui (hihi.value, hihi.value, 16);
                if(e[i] < 60)
                    tt = e[i] - 48;
                else
                    tt = e[i] - 87;

                mpz_add_ui(hihi.value, hihi.value, tt);
            }
        cout << e.length();


        cout << "e = hihi = " << hihi.Out() << endl << endl;

            mpz_mul (temp.value, SecretKey.value, hihi.value);
            mpz_sub (temp.value, R.value, temp.value);
            mpz_mod (Y.value, temp.value, q.value);

        }
        void Verify(){



        //(A * B) mod C = (A mod C * B mod C) mod C
            BigInt Xp, temp2;
            mpz_powm(temp.value, a.value, Y.value, p.value);
            mpz_powm(temp2.value, V.value, hihi.value, p.value);
            mpz_mul (temp.value, temp.value, temp2.value);
            mpz_mod (Xp.value, temp.value, p.value);
            string ep = sha256(M + Xp.Out());
            cout << "ep = " << ep << endl << endl << " = e = " << e;

        }
        string sha256(const string str){
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
};

int main(int argc, char* argv[]){

    KeyGenerator keygenerator;
    Mission mission{argc, argv};

    thread{&KeyGenerator::start, keygenerator}.detach();
    thread{&Mission::start, mission}.detach();

    cout << "exit" << endl;
    return 0;
}
