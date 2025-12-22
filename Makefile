all:
	gcc -c -fPIC -std=c99 src/*.c
	gcc -shared -o libjane.so *.o

install:
	cp libjane.so /usr/lib/
	if [ ! -d "/usr/include/jane/" ]; then \
        mkdir /usr/include/jane/ ; fi
	cp include/http.h /usr/include/jane/

clean:
	rm *.o
	rm *.so
