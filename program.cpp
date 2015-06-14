/* install openssl, have gmp ./configure --enable-cxx
 *should create etc/ld.so.conf.d/gmp.conf  write "/usr/local/lib" then, run ldconfig
 * -lcrypto -lgmpxx -lgmp -pthread
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
#include <mutex>
#include <queue>

#include <openssl/sha.h>

#define QLength 160
#define PLength 2014
#define BUFFER_SIZE BITLENTH/8
#define MAXSAVEDKEY 1

using namespace std;

class Key{
public:
    int i;
    mpz_class v, p, q;
    Key(int u){
        i = u;
    }
};


mutex a;
queue<Key> KeyQueue;
bool END;


class KeyGenerator{
    public:
        void start(){
            int i = 500;
            while(!END){
                a.lock();
                    if(KeyQueue.size() < 10){
                            KeyQueue.push(Key{i});
                            cout << "insert " << i << "   ";
                            i--;
                    }
                a.unlock();
                if(i  == 0)
                    END = true;
            }

        }
    private:
        void GenerateKey(){

            mpz_class p, q, n, a, SecretKey, v, temp;
            gmp_randclass rr(gmp_randinit_default);
            rr.seed(time(NULL));

            q = rr.get_z_bits (QLength);
            cout << "q = " << q << endl << endl;

        //MAKE p = q*n + 1 and should be a PLength-long prime
            p = 1;
            mpz_mul_2exp(p.get_mpz_t(), p.get_mpz_t(), PLength - 1);
            n = p / q;
            while(true){
                p = q * n +1;
                if(mpz_probab_prime_p(p.get_mpz_t(), 25) > 0)
                    break;
                n++;
            }

            cout << "n = " << n << endl << endl << "p = " << p << endl << endl;

            temp = 3;
            while(true){
                mpz_powm(a.get_mpz_t(), temp.get_mpz_t(), n.get_mpz_t(), p.get_mpz_t());
                if(a > 1)
                    break;
                temp++;
            }
            cout << "a = " << a << endl << endl << "temp = " << temp << endl << endl;

//for verification
mpz_powm(temp.get_mpz_t(), a.get_mpz_t(), q.get_mpz_t(), p.get_mpz_t());
cout << "should be one = " << temp << endl << endl;

            SecretKey = rr.get_z_bits (rand() % (QLength*2/3 - 1) + QLength/3);
            cout << "SecretKey = " << SecretKey << endl << endl;

            mpz_powm(v.get_mpz_t(), a.get_mpz_t(), SecretKey.get_mpz_t(), p.get_mpz_t());
            cout << "v = " << v << endl << endl;
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

            while(!END){
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
            cout << "end";

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
                    if(argv[1] =="h"){
                        cout  << "Usage: " << argv[0] << "  [-sv FILE [SIGFILE]]" << endl << "            -s Sign" << "            -v Verify" << endl;
                    }
                    if(argv[1] == "-s"){
						if(argc == 2);
					}
                    else if(argv[1] == "-v"){

					}
                    else{
						return Interact(0, NULL);
					}
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

            Key *k;

            gmp_randclass rr(gmp_randinit_default);
            rr.seed(time(NULL));

            a.lock();
                if(KeyQueue.size() > 0){
                    k = &KeyQueue.front();
                    KeyQueue.pop();
                    //if(k.i <= 0)
                    //    return;
                    //cout << "k.i = " << k.i << "   ";
                }
            a.unlock();

            mpz_class v = k->v, r, x, a, p = k->p, q = k->q, e, SecretKey, y;

string M = "I AM A SECRET FILE";
cout << "M = " << M << endl << endl;

            string e_str = sha256(M + v.get_str());
            cout << "e_str = " << e_str << endl;

            r = rr.get_z_bits (rand() % (QLength*2/3 - 1) + QLength/3);
            cout << "r = " << r << endl << endl;

            mpz_powm(x.get_mpz_t(), a.get_mpz_t(), r.get_mpz_t(), p.get_mpz_t());
            cout << "x = " << x << endl << endl;

            char input[e_str.length()];
            int tt;
            for(int i = 0;i < e_str.length();i++){
                mpz_mul_2exp(e.get_mpz_t(), e.get_mpz_t(), 4);
                if(e_str[i] < 60)
                    tt = e_str[i] - 48;
                else
                    tt = e_str[i] - 87;
                e += tt;
            }
            cout << e_str.length() << endl << endl << "e_mpz 10based = " << e << endl << endl;
            y = (r - e * SecretKey) % q;

        }
        void Verify(){

            a.lock();
                if(KeyQueue.size() > 0){
                    Key k = KeyQueue.front();
                    KeyQueue.pop();
                    if(k.i <= 0)
                        return;
                    cout << "k.i = " << k.i << "   ";
                }
            a.unlock();


            mpz_class xp, temp, temp2, a, y, p, v, e;
    string M;

        //(A * B) mod C = (A mod C * B mod C) mod C
            mpz_powm(temp.get_mpz_t(), a.get_mpz_t(), y.get_mpz_t(), p.get_mpz_t());
            mpz_powm(xp.get_mpz_t(), v.get_mpz_t(), e.get_mpz_t(), p.get_mpz_t());
            temp = (xp * temp) % p;

            string ep = sha256(M + xp.get_str());
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
    END = false;

    thread t1{&KeyGenerator::start, keygenerator};
    thread t2{&Mission::start, mission};
    t1.join();
    t2.join();
    cout << "exit" << endl;
    return 0;
}
