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
#include <fstream>

#include <openssl/sha.h>

#define QLength 160
#define PLength 2014
#define BUFFER_SIZE BITLENTH/8
#define MAXSAVEDKEY 1

using namespace std;

class Key{
public:
    int i;
    mpz_class p, q, v, a , SecretKey;
    Key(mpz_class p,mpz_class q,mpz_class v,mpz_class a,mpz_class SecretKey){
        this->p = p;

    }
};


mutex mutx;
queue<Key> KeyQueue;
bool END;


class KeyGenerator{
    private:
            mpz_class p, q, n, a, SecretKey, v, temp;

    public:
        void start(){
            int i = 500;
            while(!END){
                mutx.lock();
                    if(KeyQueue.size() < 1){
                        GenerateKey();
                        KeyQueue.push(Key{p, q, v, a, SecretKey});
                        cout << "insert " << i << "   ";
                        i--;
                    }
                mutx.unlock();
                if(i  == 0)
                    END = true;
            }

        }
    private:
        void GenerateKey(){

            gmp_randclass rr(gmp_randinit_default);
            rr.seed(time(NULL));

        //use this method to prevent q overflow
            q = 1;
            mpz_mul_2exp(q.get_mpz_t(), q.get_mpz_t(), QLength - 1);//shift to QLength
            q += rr.get_z_bits (QLength-2);
            mpz_nextprime(q.get_mpz_t(), q.get_mpz_t());
            cout << "q = " << q << endl << endl;

        //MAKE p = q*n + 1 and should be a PLength-long prime
            p = 1;
            mpz_mul_2exp(p.get_mpz_t(), p.get_mpz_t(), PLength - 1);//shift to PLength
            n = p / q;
            while(true){
                p = q * n +1;
                if(mpz_probab_prime_p(p.get_mpz_t(), 25) > 0)
                    break;
                n++;
            }

            cout << "n = " << n << endl << endl << "p = " << p << endl << endl;

        //make a^q % p = 1
        //so first make temp^n % p = a, a != (0or1)
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
        string filename, signame;

    public:
        Mission(int argc, char* argv[]){
            this->argc = argc;
            this->argv = argv;
        }

        void start(){

            while(!END){
                switch(Interact()){
                    case 1:
                        Sign();
                        break;
                    case 2:
                        Verify();
                        break;
                    default://will go here if exit in interactive mode or (error input or file not found or -h, --help) in shell-argument mode
                        END = true;
                }
            }
        }
    private:

        int Interact(){
            string cmd;

            if(argc == 1){
                while(true){//This loop is to make interact go to the begining if file not found.
                    int act;
                    cout << "1)sign  2)verify 3)exit : ";
                    getline(cin, cmd);
                    stringstream tempstream{cmd};
                    tempstream >> act;
                    if (!((act == 1) || (act == 2)))
                        return act;
                    cout << "Please input filename : ";
                    getline(cin, filename);
                    while(true){
                        cout << "Do you want the sigfile name to be " << filename << ".sig or input manually? (y or n) : ";
                        getline(cin, cmd);
                        if ((cmd == "y") || (cmd == "Y")){
                            signame = filename + ".sig";
                            break;
                        }
                        else if((cmd == "n") || (cmd == "N")){
                            cout << "Please input signame : ";
                            getline(cin, signame);
                            break;
                        }
                    }
                    if(CheckFile(act) == act)
                        return act;
                }
            }

            else{
                if((argv[1] == "-h") || (argv[1] == "--help")){
                    cout  << "Usage: " << argv[0] << "  [-sv FILE [SIGFILE]]" << endl << "            -s Sign" << "            -v Verify" << endl;
                    return 0;
                }
                if(argv[1] == "-s"){
                    if(argc == 3){
                        filename = argv[2];
                        signame = filename + ".sig";
                        cout << "the sig file will be " << filename << ".sig" << endl;
                    }
                    else if(argc == 4){
                        filename = argv[2];
                        signame = argv[3];
                    }
                    else{
                        cout << "type " << argv[0] << "-h or --help for help, or just type " << argv[0] << " to enter interactive mode";
                        return 0;
                    }
                    return CheckFile(1);
                }
                else if(argv[1] == "-v"){
                    if(argc == 3){
                        filename = argv[2];
                        signame = filename + ".sig";
                        cout << "using " << filename << ".sig as sigfile..." << endl;
                    }
                    else if(argc == 4){
                        filename = argv[2];
                        signame = argv[3];
                    }
                    else{
                        cout << "type " << argv[0] << "-h or --help for help, or just type " << argv[0] << " to enter interactive mode";
                        return 0;
                    }
                    return CheckFile(2);
                }
                else{
                    cout << "type " << argv[0] << "-h or --help for help, or just type " << argv[0] << " to enter interactive mode";
                    return 0;
                }
            }
        }

        int CheckFile(int act){
            ifstream inputfile(filename, ios::in | ios::binary);
            if (!(inputfile.good())){
                inputfile.close();
                cout << "file " << filename << "not found";
                return 0;
            }
            inputfile.close();

            if(act == 2){
                inputfile.open(signame, ios::in | ios::binary);
                if (!(inputfile.good())){
                    inputfile.close();
                    cout << "file " << signame << "not found";
                    return 0;
                }
                inputfile.close();
            }
            return act;
        }

        void Sign(){

            Key *k;

            gmp_randclass rr(gmp_randinit_default);
            rr.seed(time(NULL));

            mutx.lock();
                if(KeyQueue.size() > 0){
                    k = &KeyQueue.front();
                    KeyQueue.pop();
                }
            mutx.unlock();

            mpz_class p = k->p,
                              q = k->q,
                              v = k->v,
                              a = k->a,
                              SecretKey = k->SecretKey,
                              r, x, e, y;

        //generate random r
            r = rr.get_z_bits (rand() % (QLength*2/3 - 1) + QLength/3);
            cout << "r = " << r << endl << endl;
        //make a^r % p = x
            mpz_powm(x.get_mpz_t(), a.get_mpz_t(), r.get_mpz_t(), p.get_mpz_t());
            cout << "x = " << x << endl << endl;


            ifstream inputfile(filename, ios::in | ios::binary);
            stringstream M;
            M << inputfile.rdbuf();
            string e_str = sha256(M.str() + x.get_str());
            cout << "e_str = " << e_str << endl;
            inputfile.close();

            char input[e_str.length()];
            int tt;
        //transfer e_str to e
            for(int i = 0;i < e_str.length();i++){
                mpz_mul_2exp(e.get_mpz_t(), e.get_mpz_t(), 4);
                if(e_str[i] < 60)
                    tt = e_str[i] - 48;
                else
                    tt = e_str[i] - 87;
                e += tt;
            }
            cout << e_str.length() << endl << endl << "e_mpz 10based = " << e << endl << endl;
        //generate y

            y = (r - e * SecretKey) % q;
//r and secretkey will be secret
//write out p v a e y
            ofstream sig(signame, ios::out | ios::binary | trunc);
            sig << p << endl << v << endl << a << endl << e << endl << y;
            sig.close();
        }

        void Verify(){

            mpz_class xp, temp, temp2, p, v, a, e, y;
            ifstream sig(signame, ios::in | ios::binary);
            sig >> p >> v >> a >> e >> y;
            sig.close();

        //(A * B) mod C = (A mod C * B mod C) mod C
            mpz_powm(temp.get_mpz_t(), a.get_mpz_t(), y.get_mpz_t(), p.get_mpz_t());
            mpz_powm(xp.get_mpz_t(), v.get_mpz_t(), e.get_mpz_t(), p.get_mpz_t());
            temp = (xp * temp) % p;

            ifstream inputfile(filename, ios::in | ios::binary);
            stringstream M;
            M << inputfile.rdbuf();
            inputfile.close();

            string ep = sha256(M.str() + xp.get_str());
            cout << "ep = " << ep << endl << endl << " = e = " << e;
//
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
