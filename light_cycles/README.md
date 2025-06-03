# Titan light tube sim 

## Programme simulant des neon titanb avec Raylib

### Ce programme simule des cycles de temps sur le principe suivant :

Chaque machine un temps d'ecriture différentes : 

- Argile             --> 1mot minute 
- Papier             --> 6.5s par mot 
- Machine à ecrire   --> 5.6s par mot
- Vielle ordinateur  --> 3.2s par mot 
- Ordinateur moderne --> 1.7s par mot 

Les valeurs sontr des moyenne que j'ai chronometrer. 

Chaque valeur est diviser par 3 pour avoir 3 phase dans ce lapse de temps :

1. Allumage douce de la lumiere 
2. Temps de lumiere constant 
3. Lumiere qui s'etaeint de maniere douce 


## Command install

1. sudo apt install libpsl-dev zlib1g-dev libssl-dev
2. wget https://curl.se/download/curl-8.14.0.tar.gz 
3. tar -xvzf curl-8.14.0.tar.gz
4. rm zip file
5. cd curl-8.14.0
6. ./configure --prefix=$HOME/Documents/Diploma/programme/curl --disable-shared --with-ssl
7. make -j$(nproc)
8. make install


**A Faire :**

- Retravailler les couleur. 
- Voir si j'auitomatise le programme pour que chaque cycle fasse soit :  
  
   - 5min si 25min/ 8min si 40min  
   - Je relie a une "machine" qui permet de changer les cycles manuellement 






