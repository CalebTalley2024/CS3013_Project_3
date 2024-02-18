
CFLAGS += -Wpedantic -pedantic-errors
CFLAGS += -Werror
CFLAGS += -Wall
CFLAGS += -Wextra
#CFLAGS += -Waggregate-return
CFLAGS += -Wbad-function-cast
CFLAGS += -Wcast-align
CFLAGS += -Wno-cast-qual	# free() should accept const pointers
CFLAGS += -Wno-declaration-after-statement
CFLAGS += -Wfloat-equal
CFLAGS += -Wformat=2
CFLAGS += -Wlogical-op
CFLAGS += -Wmissing-include-dirs
CFLAGS += -Wno-missing-declarations
CFLAGS += -Wno-missing-prototypes
CFLAGS += -Wnested-externs
CFLAGS += -Wpointer-arith
CFLAGS += -Wredundant-decls
CFLAGS += -Wsequence-point
CFLAGS += -Wshadow
CFLAGS += -Wno-strict-prototypes
CFLAGS += -Wswitch
CFLAGS += -Wundef
CFLAGS += -Wunreachable-code
CFLAGS += -Wunused-but-set-parameter
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wno-maybe-uninitialized
CFLAGS += -Wwrite-strings

BINARIES += mm

all: $(BINARIES)

OPT = -O3

mm: mm_main.o mm_api.o
	gcc -g $(OPT) $^ -o $@

%.o: %.c Makefile
	gcc -g $(OPT) -x c $(CFLAGS) $< -c -o $@ -MD -MF $(@:.o=.d)

project3.zip: FORCE
	rm -rf $@ project3/ && mkdir project3/
	cp mm_main.c mm_api.h mm_api.c Makefile setup.sh dosbox.diff README.txt project3/
	zip -r $@ project3/

clean:
	rm -f *.o *.d project3.zip project3_starter.zip $(BINARIES)

dosbox-src/README:
	svn checkout https://svn.code.sf.net/p/dosbox/code-0/dosbox/trunk dosbox-src
	cd dosbox-src && patch -p1 < ../dosbox.diff

dosbox-src/Makefile: dosbox-src/README
	cd dosbox-src && ./autogen.sh
	cd dosbox-src && CFLAGS=-DWPI_OS_PROJECT_3=1 CPPFLAGS=-DWPI_OS_PROJECT_3=1 ./configure --prefix=$$(pwd)/../dosbox-bin

dosbox-bin/bin/dosbox: dosbox-src/Makefile mm_api.h mm_api.c Makefile
	cd dosbox-src && make
	cd dosbox-src && make install

dosbox: dosbox-bin/bin/dosbox

.PHONY: all clean dosbox FORCE

-include *.d

