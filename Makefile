all:
	gcc -c -fPIC src/*.c
	gcc -shared -o libjane.so *.o
	mkdir build
	cp libjane.so build/

install:
	cp libjane.so /usr/lib/
	mkdir /usr/include/jane/
	cp -r include/* /usr/include/jane/

clean:
	rm *.so *.o
	rm -r build/