all:
	gcc main.c -o app
clean:
	rm -f app fileA.bin fileB.bin fileC.bin fileD.bin fileA.bin.gz fileB.bin.gz result.txt
