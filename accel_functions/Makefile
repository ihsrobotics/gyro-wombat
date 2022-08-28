default: all
all: accel_cpp accel_c

accel_cpp: accel_accumulator.cpp
	echo "building cpp"
	g++ ./accel_accumulator.cpp -lkipr -lpthread -O3 -std=c++11 -fPIC --shared -o accel.so

accel_c: accel.c
	echo "building c"
	gcc ./accel.c -lkipr -lpthread -L . -l:./accel.so -O3 -std=c11 -o a.out