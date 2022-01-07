# Makefile - Projeto de Redes de Computadores - 2º Período 2021/22 - IST

all: user DS

user: common.o Client/client_main.o Client/client_tcp.o Client/client_udp.o
	gcc -o user common.o Client/client_main.o Client/client_tcp.o Client/client_udp.o

DS: Server/server_main.o Server/server_udp.o Server/server_tcp.o
	gcc -o DS common.o Server/server_main.o Server/server_udp.o Server/server_tcp.o

clean:
	rm -rf Client/*.o Server/*.o *.o user DS GROUPS USERS DOWNLOADS