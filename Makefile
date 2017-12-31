all:
		cd compression; qmake .
		make -C compression
		make -C compression clean
		make -C correction

test:
		make -C correction

clean:
		cd compression && ./clean
		make -C correction clean
