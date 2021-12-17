# Makefile - Projeto de Redes de Computadores - 2º Período 2021/22 - IST

all: user 

user: client_main.o client_tcp.o client_udp.o
	gcc -o user client_main.o client_tcp.o client_udp.o

clean:
	rm *.o user