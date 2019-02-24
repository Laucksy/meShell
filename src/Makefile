compile:
	gcc -o meShell main.c parse.c execute.c handlers.c jobs.c -lreadline -Wall -Wextra

clean:
	rm meShell

install:
	make compile
	cp meShell ../install/bin/
	mkdir -p ../install/man/man1/
	gzip < man/man1/meShell.1 > ../install/man/man1/meShell.1.gz
