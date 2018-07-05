.PHONY: all clean

all: Makefile.qmake
	make -f $<
	./arranger

Makefile.qmake: arranger.pro
	qmake $< -o $@

clean:
	make -f Makefile.qmake distclean
