default: all
all: gyro_cpp gyro_c

gyro_cpp: gyro_accumulator.cpp gyro_accumulator.h
	echo "building cpp"
	g++ -lkipr -lpthread -fPIC --shared -O3 -std=c++11 gyro_accumulator.cpp -o gyro.so

gyro_c: gyro.c gyro.so gyro_accumulator.h
	echo "building c"
	gcc -lkipr -lpthread -lpthread -L . -l:./gyro.so -O3 -std=c11 gyro.c -o a.out

clean:
	rm ./gyro.so