# Makefile - Projeto de Redes de Computadores - 2º Período 2021/22 - IST

all: user DS

user: Client/client_main.o Client/client_tcp.o Client/client_udp.o
	gcc -o user Client/client_main.o Client/client_tcp.o Client/client_udp.o

DS: Server/server_main.o Server/server_udp.o
	gcc -o DS Server/server_main.o Server/server_udp.o

clean:
	rm -f Client/*.o Server/*.o user DS 
	rm -rf GROUPS SERVERS