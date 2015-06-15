/*  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



//TODO    :make it batch sign whole directory

#include <stdio.h>
#include <gmpxx.h>

#include <iomanip>
#include <openssl/sha.h>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>

#define QLength 256
#define PLength 1024
#define MAXSAVEDKEY 2
#define DEBUG 1

using namespace std;

class Key{
    public:
        int i;
        mpz_class p, q, v, a , privatekey;
        Key(){}
        Key(mpz_class p,mpz_class q,mpz_class v,mpz_class a,mpz_class privatekey){
            this->p = p;
            this->q = q;
            this->v = v;
            this->a = a;
            this->privatekey = privatekey;
        }
};

mutex mutx;
queue<Key> KeyQueue;
bool END;


class KeyGenerator{
    private:
            mpz_class p, q, n, a, privatekey, v, temp;

    public:
        void start(){
            while(!END){
                mutx.lock();
                    if(KeyQueue.size() < 1){
                        GenerateKey();
                        KeyQueue.push(Key{p, q, v, a, privatekey});
                    }
                mutx.unlock();
            }
        }
    private:
        void GenerateKey(){

            gmp_randclass rr(gmp_randinit_default);
            rr.seed(time(NULL));

            //This method is going to prevent q overflow
            q = 1;
            mpz_mul_2exp(q.get_mpz_t(), q.get_mpz_t(), QLength - 1);//shift to QLength
            q += rr.get_z_bits (QLength-2);
            mpz_nextprime(q.get_mpz_t(), q.get_mpz_t());

            //p = q*n + 1 and should be a PLength-long prime
            p = 1;
            mpz_mul_2exp(p.get_mpz_t(), p.get_mpz_t(), PLength - 1);//shift to PLength
            n = p / q;
            while(true){
                p = q * n +1;
                if(mpz_probab_prime_p(p.get_mpz_t(), 25) > 0)
                    break;
                n++;
            }

            //a^q % p = 1
            //so first, a = temp^n % p, a > 1
            temp = 2;
            while(true){
                mpz_powm(a.get_mpz_t(), temp.get_mpz_t(), n.get_mpz_t(), p.get_mpz_t());
                if(a > 1)
                    break;
                temp++;
            }

            //for verification
            mpz_powm(temp.get_mpz_t(), a.get_mpz_t(), q.get_mpz_t(), p.get_mpz_t());
            if(temp != 1){
                cerr << "error";
                exit(-1);
            }

            privatekey = rr.get_z_bits (rand() % (QLength*2/3 - 1) + QLength/3);

            //a^s % p = v
            mpz_powm(v.get_mpz_t(), a.get_mpz_t(), privatekey.get_mpz_t(), p.get_mpz_t());
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

                END = true;

                vector<string> argvs;
                string ts;
                for(int i = 0;i < argc;i++){
                    ts = argv[i];
                    argvs.push_back(ts);
                }

                if((argvs[1] == "-h") || (argvs[1] == "--help")){
                    cout  << "Usage: " << argvs[0] << "  [-sv FILE [SIGFILE]]" << endl << "            -s Sign" << endl << "            -v Verify" << endl;
                    return 0;
                }
                if(argvs[1] == "-s"){
                    if(argc == 3){
                        filename = argvs[2];
                        signame = filename + ".sig";
                        cout << "the sig file will be " << filename << ".sig" << endl;
                    }
                    else if(argc == 4){
                        filename = argvs[2];
                        signame = argvs[3];
                    }
                    else{
                        cout << "type " << argvs[0] << "-h or --help for help, or just type " << argvs[0] << " to enter interactive mode";
                        return 0;
                    }
                    return CheckFile(1);
                }
                else if(argvs[1] == "-v"){
                    if(argc == 3){
                        filename = argvs[2];
                        signame = filename + ".sig";
                        cout << "using " << filename << ".sig as sigfile..." << endl;
                    }
                    else if(argc == 4){
                        filename = argvs[2];
                        signame = argvs[3];
                    }
                    else{
                        cout << "type " << argvs[0] << "-h or --help for help, or just type " << argvs[0] << " to enter interactive mode";
                        return 0;
                    }
                    return CheckFile(2);
                }
                else{
                    cout << "type " << argvs[0] << "-h or --help for help, or just type " << argvs[0] << " to enter interactive mode";
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

            Key k;

            gmp_randclass rr(gmp_randinit_default);
            rr.seed(time(NULL));

            mutx.lock();
                if(KeyQueue.size() > 0){
                    k = KeyQueue.front();
                    KeyQueue.pop();
                }
            mutx.unlock();

            mpz_class p = k.p,
                              q = k.q,
                              v = k.v,
                              a = k.a,
                              privatekey = k.privatekey,
                              r, x, e, y;

            //generate random r
            r = rr.get_z_bits (rand() % (QLength*2/3 - 1) + QLength/3);

            //x = a^r % p
            mpz_powm(x.get_mpz_t(), a.get_mpz_t(), r.get_mpz_t(), p.get_mpz_t());

            ifstream inputfile(filename, ios::in | ios::binary);
            stringstream M;
            M << inputfile.rdbuf();

            string e_str = sha256(M.str() + x.get_str());
            cout << "e_str = " << e_str << endl;
            inputfile.close();

            e = HexStringToMpz(e_str);

            //generate y
            y = (r + privatekey * e) % q;

            //r and privatekey will be secret
            ofstream sig(signame, ios::out | ios::binary | ios::trunc);
            sig << p << endl << v << endl << a << endl << e << endl << y << endl;
            sig.close();
            cout << "DONE!" << endl << endl;
if(DEBUG)
    cout << "privatekey = " << privatekey  << endl << "r = " << r  << endl << "p = " << p  << endl << "q = " << q << endl  << "v = " << v << endl << "a = " << a << endl << "e = " << e << endl << "y = " << y << endl << "x = " << x  << endl << endl;
        }

        void Verify(){

            mpz_class xp, temp, temp2, p, v, a, e, y, ep;
            ifstream sig(signame, ios::in | ios::binary);
            sig >> p >> v >> a >> e >> y;
            sig.close();

            //xp = ((a^y) * (v^(-e))) % p ;
            //(A * B) mod C = (A mod C * B mod C) mod C
            temp2 = -e;
            mpz_powm(temp.get_mpz_t(), a.get_mpz_t(), y.get_mpz_t(), p.get_mpz_t());
            mpz_powm(xp.get_mpz_t(), v.get_mpz_t(), temp2.get_mpz_t(), p.get_mpz_t());
            xp = (xp * temp) % p;

            ifstream inputfile(filename, ios::in | ios::binary);
            stringstream M;
            M << inputfile.rdbuf();
            inputfile.close();

            string ep_str = sha256(M.str() + xp.get_str());
            ep = HexStringToMpz(ep_str);

if(DEBUG)
    cout << "p = " << p  << endl << "v = " << v << endl << "a = " << a << endl << "e = " << e << endl << "y = " << y << endl << "xp = " << xp << endl << ep_str << endl << endl;

            if(ep == e)
                cout << "Match" << endl << endl;
            else
                cout << "Not Match" << endl << endl;
        }

        string sha256(const string str){
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256, str.c_str(), str.size());
            SHA256_Final(hash, &sha256);
            stringstream ss;
            for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
                ss << hex << setw(2) << setfill('0') << (int)hash[i];

            return ss.str();
        }

        mpz_class HexStringToMpz(string e_str){
            int tt;
            mpz_class e = 0;
            for(unsigned int i = 0;i < e_str.length();i++){
                mpz_mul_2exp(e.get_mpz_t(), e.get_mpz_t(), 4);
                if(e_str[i] < 60)
                    tt = e_str[i] - 48;
                else
                    tt = e_str[i] - 87;
                e += tt;
            }
            return e;
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
    return 0;
}
