# Schnorr_Signature
for information security course

# Requirements
openssl installed, GMP6.0 installed

#Issue about GMP
when installing gmp, should enable cxx, like below
./configure --enable-cxx

If your lib is installed under /usr/local/lib, 
one possible way is to create etc/ld.so.conf.d/gmp.conf  put "/usr/local/lib" in it, and run ldconfig
