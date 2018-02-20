# Schnorr_Signature
This program is written in C++11, including GMP and openssl,
can sign or verify files using Schnorr_Signature.
Interactive mode(rather than passing arguement) is recommended, this can get full advantage of Schnorr algorithm
, which is able to pre-generate keys.
This program has one thread for key generating, and one another for signing or verifing.



# Requirements
openssl installed, GMP6.0 installed

#Issue about GMP
when installing gmp, cxx should be enabled, as below
./configure --enable-cxx

If your lib is installed under /usr/local/lib, 
one possible way is to create `etc/ld.so.conf.d/gmp.conf`, add "/usr/local/lib" in it, then run ldconfig.

#Go Through the program
Edit MAXSAVEDKEY for the limit of pairs of key to genarate and put in the keyqueue for use.
This program implements Hash with SHA-256, so make the QLength = 256
DEBUG 1 for debug, and 0 for release
