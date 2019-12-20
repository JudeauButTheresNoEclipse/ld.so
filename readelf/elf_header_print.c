#include <stdio.h>
#include <elf.h>
#include <link.h>

#include "include/header_info.h"

static void print_magic_number(unsigned char *e_ident)
{
    printf(" Magic:\t");
    for (size_t i = 0; i < EI_NIDENT - 1; i++)
        printf("%02x ", e_ident[i]);
    printf("%02x\n", e_ident[EI_NIDENT - 1]);
}

static void print_class(unsigned char c)
{
    printf(" Class:\t\t");
    if (c == ELFCLASSNONE)
        printf("NONE\n");
    if (c == ELFCLASS32)
        printf("ELF32\n");
    if (c == ELFCLASS64)
        printf("ELF64\n");
}

static void print_data(unsigned char c)
{
    printf(" Data:\t\t");
    if (c == ELFDATANONE)
        printf("unknown\n");
    if (c == ELFDATA2LSB)
        printf("2's complement, little endian\n");
    if (c == ELFDATA2MSB)
        printf("2's complement, big endian\n");
}

static void print_elf_version(unsigned char c)
{
    printf(" Version:\t");
    if (c == EV_NONE)
        printf("invalid\n");
    else
        printf("%d (current)\n", c);
}

static void print_os(unsigned char c)
{
    printf(" OS/ABI:\t");
    switch (c)
    {
    case ELFOSABI_SYSV:
        printf("UNIX - System V");
        break;
    case ELFOSABI_HPUX:
        printf("HP-UX");
        break;
    case ELFOSABI_NETBSD:
        printf("NETBSD");
        break;
    case ELFOSABI_LINUX:
        printf("Linux");
        break;
    case ELFOSABI_SOLARIS:
        printf("Solaris");
        break;
    case ELFOSABI_IRIX:
        printf("IRIX");
        break;
    case ELFOSABI_FREEBSD:
        printf("FreeBSD");
        break;
    case ELFOSABI_TRU64:
        printf("TRU64 UNIX");
        break;
    case ELFOSABI_ARM:
        printf("ARM architecture");
        break;
    case ELFOSABI_STANDALONE:
        printf("Stand-alone (embedded)");
        break;
    }
    printf("\n");
}
static void print_abi(unsigned char c)
{
    printf(" ABI Version:\t%d\n", c);
}

static void print_type(uint16_t e_type)
{
    printf(" Type:\t\t");
    if (e_type == ET_NONE)
        printf("NONE (An unknown type)\n");
    if (e_type == ET_REL)
        printf("REL (A relocatable file)\n");
    if (e_type == ET_EXEC)
        printf("EXEC (An executable file)\n");
    if (e_type == ET_DYN)
        printf("DYN (A shared object file)\n");
    if (e_type == ET_CORE)
        printf("CORE (A core file)\n");
}

static void print_machine(uint16_t e_machine)
{
    printf(" Machine:\t");
    struct dictionary_entry
    {
        uint16_t machine;
        char *msg;
    } machine_list[]
    = { { EM_NONE, "An unknown machine" },
        { EM_M32, "AT&T WE 32100" },
        { EM_SPARC, "Sun Microsystems SPARC" },
        { EM_386, "Intel 80386" },
        { EM_68K, "Motorola 68000" },
        { EM_88K, "Motorola 88000" },
        { EM_860, "Intel 80860" },
        { EM_MIPS, "MIPS RS3000 (big-endian only)" },
        { EM_PARISC, "HP/PA" },
        { EM_SPARC32PLUS, "SPARC with enhanced instruction set" },
        { EM_PPC, "PowerPC" },
        { EM_PPC64, "PowerPC 64-bit" },
        { EM_S390, "IBM S/390" },
        { EM_ARM, "Advanced RISC Machines" },
        { EM_SH, "Renesas SuperH" },
        { EM_SPARCV9, "SPARC v9 64-bit" },
        { EM_IA_64, "Intel Itanium" },
        { EM_X86_64, "AMD x86-64" },
        { EM_VAX, "DEC Vax" } };
    size_t size = sizeof(machine_list) / sizeof(machine_list[0]);
    for (size_t i = 0; i < size; i++)
        if (e_machine == machine_list[i].machine)
            printf("%s\n", machine_list[i].msg);
}

static void print_version(uint32_t e_version)
{
    printf(" Version:\t");
    if (e_version == EV_NONE)
        printf("Invalid version\n");
    else
        printf("0x%x\n", e_version);
}

static void print_entry_offset()
{
    if (get_elf_class() == ELFCLASS64)
    {
        printf(" Entry point address:\t\t0x%lx\n", *(uint64_t *)get_entry());
        printf(" Start of program headers:\t%ld\n", *(uint64_t *)get_program_table_offset());
        printf(" Start of section headers:\t%ld\n", *(uint64_t *)get_section_table_offset());
    }
    if (get_elf_class() == ELFCLASS32)
    {
        printf(" Entry point address:\t\t0x%x\n", *(uint32_t *)get_entry());
        printf(" Start of program headers:\t%d\n", *(uint32_t *)get_program_table_offset());
        printf(" Start of section headers:\t%d\n", *(uint32_t *)get_section_table_offset());
    }
}

void print_elf_header()
{
    ElfW(Ehdr) *head = get_elf_header();
    printf("ELF Header:\n");
    print_magic_number(head->e_ident);
    print_class(head->e_ident[EI_CLASS]);
    print_data(head->e_ident[EI_DATA]);
    print_elf_version(head->e_ident[EI_VERSION]);
    print_os(head->e_ident[EI_OSABI]);
    print_abi(head->e_ident[EI_ABIVERSION]);
    print_type(head->e_type);
    print_machine(head->e_machine);
    print_version(head->e_version);
    print_entry_offset();
    printf(" Flags:\t\t%d\n", head->e_flags);
    printf(" Size of this header:\t%d (bytes)\n", head->e_ehsize);
    printf(" Size of program headers:\t%d\n", get_program_entry_size());
    printf(" Number of program header:\t%d\n", get_program_table_size());
    printf(" Size of section header:\t%d\n", get_section_entry_size());
    printf(" Number of section header:\t%d\n", get_section_table_size());
    printf(" Section header string table index: %d\n\n", head->e_shstrndx);
}
