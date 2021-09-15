mkdir build
cd build
rm bubblebuddy
gcc -o bubblebuddy ../source/main.c -lSDL2 -lm -lGL -g
cp ./bubblebuddy ../
cd ..
