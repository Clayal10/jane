all:
	gcc -c -fPIC src/*.c
	gcc -shared -o libjane.so *.o

install:
	cp libjane.so /usr/lib/
	if [ ! -d "/usr/include/jane/" ]; then \
        mkdir /usr/include/jane/ ; fi
	cp -r include/* /usr/include/jane/

clean:
	rm *.o
	rm *.so