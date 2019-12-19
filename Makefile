CC = gcc
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

#READELF_OBJS = \
#			  ldso/program_header.o \
#			  ldso/read_elf_file.o \
#			  ldso/string_table.o \
#			  ldso/elf_header_info.o \
#
#READELF_BIN_OBJS= \
#			$(READELF_OBJS) \
#			ldso/section_header_print.o\
#			ldso/elf_header_print.o \
#			ldso/program_header_print.o \
#			ldso/dynamic_section_print.o \
#			ldso/dynsim_section_print.o \
#			ldso/readelf/readelf.o \
#				
LDSO_OBJS = \
	    ldso/ldso_start.o \
	    ldso/main.o \
		ldso/display_auxv.o \
		ldso/elf_manipulation.o \
		ldso/functions.o \
		ldso/dependency.o \
		ldso/utility.o \
		ldso/reloc.o \
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
	    libc2.so

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

#$(TESTS): LDFLAGS += -Wl,--dynamic-linker=./ld.so -Wl,-rpath,/tmp

$(TESTS): LDFLAGS += -Wl,--dynamic-linker=./ld.so -Wl,-rpath-link=.


test-standalone: LDLIBS = -L. -luseless
test-standalone: libc/crt0.o tests/test-standalone.o $(LIBC_OBJS)

test-onelib: LDLIBS = -L. -lc
test-onelib: libc/crt0.o tests/test-standalone.o

test-libs: LDLIBS = -L. -lc2
test-libs: libc/crt0.o tests/test-standalone.o


$(TEST_LIBS): CFLAGS += -fPIC

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
dummy_readelf: CPPFLAGS = -Iinclude
dummy_readelf: LDFLAGS =
dummy_readelf: $(READELF_BIN_OBJS)
	$(CC) $(READELF_BIN_OBJS) -o $@

%.so:
	$(LINK.o) -shared $^ $(LDLIBS) -o $@

clean:
	@$(RM) $(TESTS) $(TEST_LIBS) $(LDSO_OBJS) $(TARGETS)
	@$(RM) $(READELF_BIN_OBJS) $(LIBC_OBJS) tests/test-standalone.o 
	@$(RM) $(UNIT_TESTS) test libc/crt0.o libc/useless.o
