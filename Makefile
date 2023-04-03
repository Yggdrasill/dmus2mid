all:
	+${MAKE} -C src/
	mv src/dmus2mid .

clean:
	rm dmus2mid
	rm src/*.o
