#ifndef HEADER_INFO_H
#define HEADER_INFO_H
#include <elf.h>
#include <link.h>
void *set_elf_header(void *addr);
void *get_elf_header();
void *get_section_header();
void *get_program_headers();
char *get_string_table();
void *get_dynamic_section();
void *get_dynsim_section();
int get_dynsim_size();
void *get_str_tab();

void *get_entry();
void *get_program_table_offset();
uint16_t get_program_entry_size();
uint16_t get_program_table_size();
void *get_section_table_offset();
uint16_t get_section_entry_size();
uint16_t get_section_table_size();
unsigned char get_elf_class();
void get_needed_entry(ElfW(Dyn) *dynamic, struct link_map *map);

#endif /* !HEADER_INFO_H */
