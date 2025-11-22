# Yao's Millionare Problem Demo makefile

all: main

main: src/main.cc src/main.h src/keygen.cc src/keygen.h src/encryptDecrypt.cc src/encryptDecrypt.h
	g++ -std=c++20 src/keygen.cc src/encryptDecrypt.cc src/main.cc -lgmp -lgmpxx -o main

clean:
	rm -rf build/*
	rm main