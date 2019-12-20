CC = cc
CFLAGS = -Wall -Wextra -g
CFLAGS += \
	  -ffreestanding \
	  -fno-builtin \
	  -fno-pie \
	  -fno-stack-protector \

CPPFLAGS = \
	   -Iinclude \

LDFLAGS = \
	  -no-pie \
	  -nostartfiles \
	  -nostdlib \
	  -Wl,-z,norelro \

READELF_BIN_OBJS= \
			readelf/section_header_print.o\
			readelf/elf_header_print.o \
			readelf/program_header_print.o \
			readelf/dynamic_section_print.o \
			readelf/dynsim_section_print.o \
			readelf/readelf.o \
			readelf/program_header.o \
			readelf/read_elf_file.o \
			readelf/string_table.o \
			readelf/elf_header_info.o \

LDSO_OBJS = \
	    ldso/ldso_start.o \
	    ldso/ldso.o \
		ldso/display_auxv.o \
		ldso/elf_manipulation.o \
		ldso/loader.o \
		ldso/dependency.o \
		ldso/utility.o \
		ldso/reloc.o \
		ldso/libdl.o \
		ldso/relocations.o \
		ldso/symbol_resolution.o \
	    $(LIBC_STDIO_OBJS) \
	    $(LIBC_STRING_OBJS) \
	    $(LIBC_UNISTD_OBJS) \
	    libc/malloc.o 

LIBC_BASE_OBJS = \
	    libc/libc_start_main.o \

LIBC_STDIO_OBJS = \
	    libc/printf.o \
	    libc/stdio.o \

LIBC_UNISTD_OBJS = \
	    libc/unistd.o\

LIBC_STRING_OBJS = \
	    libc/string.o \

LIBC_OBJS = \
	    $(LIBC_BASE_OBJS) \
	    $(LIBC_STDIO_OBJS) \
	    $(LIBC_STRING_OBJS) \
	    $(LIBC_UNISTD_OBJS) \
	    libc/malloc.o \


USELESS_OBJS = \
	    libc/useless.o \

TEST_LIBS = \
	    libuseless.so \
	    libc.so \
	    libunistd.so \
	    libstring.so \
	    libc2.so	\

TESTS = \
	test-onelib \
	test-libs \
	test-standalone \

UNIT_TESTS= \
	tests/functions.o


MALLOC_CPPFLAGS = -include include/malloc-internal.h

TARGETS = ld.so


all: $(TARGETS) $(TEST_LIBS) $(TESTS)

check: CFLAGS = -g -Wall -Wextra
check: LDFLAGS = -lcriterion
check: $(UNIT_TESTS)
	@$(CC) $(LDFLAGS) $(UNIT_TESTS) $(LIBC_OBJS) -o test #2> /dev/null
	./test

$(TESTS):
	$(LINK.o) $^ $(LDLIBS) -o $@


$(TESTS): LDFLAGS += -Wl,--dynamic-linker=./ld.so -Wl,-rpath-link=.


test-standalone: LDLIBS = -L. -luseless -l:ld.so
test-standalone: libc/crt0.o tests/test-standalone.o $(LIBC_OBJS)

test-onelib: LDLIBS = -L. -lc -l:ld.so
test-onelib: libc/crt0.o tests/test-standalone.o

test-libs: LDLIBS = -L. -lc2 -l:ld.so
test-libs: libc/crt0.o tests/test-standalone.o


$(TEST_LIBS): CFLAGS += -fPIC

$(LDSO_OBJS): CLAGS += -fPIC 

libc.so: $(LIBC_OBJS)

libc2.so: LDLIBS += -L. -lunistd -lstring
libc2.so: $(LIBC_BASE_OBJS) $(LIBC_STDIO_OBJS)


libunistd.so: $(LIBC_UNISTD_OBJS)
libstring.so: $(LIBC_STRING_OBJS)
libuseless.so: $(USELESS_OBJS)

ld.so: CFLAGS += -fPIC
ld.so: LDFLAGS += -Wl,--version-script,ldso/exported-symbols.map -Wl,-soname,ld.so
ld.so: $(LDSO_OBJS)

libc/printf.o: CPPFLAGS += -Iinclude/printf
libc/malloc.o: CPPFLAGS += $(MALLOC_CPPFLAGS)

dummy_readelf: CFLAGS = -Wall -Wextra -g
dummy_readelf: CPPFLAGS = 
dummy_readelf: LDFLAGS =
dummy_readelf: $(READELF_BIN_OBJS)
	$(CC) $(READELF_BIN_OBJS) -o $@

%.so:
	$(LINK.o) -shared $^ $(LDLIBS) -o $@

clean:
	@$(RM) $(TESTS) $(TEST_LIBS) $(LDSO_OBJS) $(TARGETS)
	@$(RM) $(READELF_BIN_OBJS) $(LIBC_OBJS) tests/test-standalone.o 
	@$(RM) $(UNIT_TESTS) test libc/crt0.o libc/useless.o
	@$(RM) $(READELF_BIN_OBJS) dummy_readelf
