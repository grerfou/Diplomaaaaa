# Curl installation et dépendance





## Command install

1. sudo apt install libpsl-dev zlib1g-dev libssl-dev
2. wget https://curl.se/download/curl-8.14.0.tar.gz 
3. tar -xvzf curl-8.14.0.tar.gz
4. rm zip file
5. cd curl-8.14.0
6. ./configure --prefix=$HOME/Documents/Diploma/programme/curl --disable-shared --with-ssl
7. make -j$(nproc)
8. make install
