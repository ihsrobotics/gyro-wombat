# Gyro Functions (advanced)
## About
This is a folder with functions using the gyro.
This folder has a more advanced setup than the simple gyro
and requires compiling cpp files.
## How to use
First, compile `gyro_cpp.cpp` on the wombat using 
`g++ ./gyro_cpp.cpp -lkipr -lwallaby -std=c++11 --shared -fPIC -o gyro.so`.
Next, you can compile `gyro_stuff.c` using 
`gcc ./gyro_stuff.c -lkipr -lwallaby -L . -l:./gyro.so -std=c11`.
(Note that, as it is, gyro_stuff.c just calibrates the gyro so that there
exists a file `biases.bin`. To see it turn, you can comment out the calibrate
section and run the other sections.)