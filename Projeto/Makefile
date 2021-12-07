# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2020-21

CC   = gcc

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

all: user #Colocar aqui DS

user: client.o
	$(CC) -o user client_main.c client_tcp.c client_udp.c client.h

clean:
	@echo Cleaning...
	rm -f *.o user