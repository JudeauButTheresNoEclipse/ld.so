#include "../ldso/dependency.c"         // I'm sure there is a better way to do
#include "../ldso/elf_manipulation.c"   // this, but it gets the job done 
#include "../ldso/functions.c"
#include "../ldso/main.c"
#include "../ldso/display_auxv.c"
#include <criterion/criterion.h>

Test(concatenate, simple)
{
	char dest[100]= "I want to";
	char src[100] = " join the lab";
	char *ret = concatenate(dest, src);
	cr_assert_str_eq(ret, "I want to/ join the lab");
	free(ret);
}

Test(get_elf_header, simple)
{
	elf_ehdr *ehdr = get_elf_ehdr("/bin/echo");
	cr_assert(!strncmp((unsigned char *)ehdr->e_ident + 1, "ELF", 3));
    free(ehdr);
}

Test(get_elf_phdr, simple)
{
	elf_ehdr *ehdr = get_elf_ehdr("/bin/echo");
    elf_phdr *phdr = get_program_header(ehdr, "/bin/echo");
    cr_assert_eq(phdr->p_type, PT_PHDR);
    free(phdr);
    free(ehdr);
}

Test(get_elf_shdr, simple)
{
	elf_ehdr *ehdr = get_elf_ehdr("/bin/echo");
    elf_shdr *shdr = get_section_header(ehdr, "/bin/echo");
    shdr += ehdr->e_shstrndx;
    cr_assert_eq(shdr->sh_type, SHT_STRTAB);
    free(ehdr);
}
