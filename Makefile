# This is a simple Makefile for small projects.  When you  
# type make at the command prompt, it will compile the code.
# For more depth, see http://www.gnu.org/software/make/manual/make.html

CC=g++
CFLAGS=-lglut -lGLU -lGL -lm -lSDL2 -lSDL2_mixer -g -Wall -Wextra -std=c++17 -O3 -lGLEW

main: cannon.cc
	$(CC) -o cannon cannon.cc $(CFLAGS)