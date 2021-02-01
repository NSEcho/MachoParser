#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mach-o/loader.h>
#include <stdint.h>
#include <string.h>
#include <mach/vm_prot.h>

#include "utils.h"

#define TAB "\t"
#define NUMBER_OF_PROTECTIONS 	7
#define MAX_SIZE				50

#define CYAN					"\033[0;36m"
#define RED 					"\033[0;31m"
#define RESET					"\033[0;0m"


void dumper(FILE *);

int main(int argc, char *argv[])
{
	const char *filename = argv[1];
	FILE *f = fopen(filename, "rb");
	dumper(f);
}

uint32_t get_magic(FILE *obj_file, int offset) {
  uint32_t magic;
  fseek(obj_file, offset, SEEK_SET);
  fread(&magic, sizeof(uint32_t), 1, obj_file);
  return magic;
}

uint32_t dump_header(FILE *obj_file)
{
	int header_size = sizeof(struct mach_header_64);
	struct mach_header_64 *header = malloc(sizeof(struct mach_header_64));
	fread(header, header_size, 1, obj_file);
	printf("ARCH\t\t\t=> 0x%02x\n", header->magic);
	printf("CPUTYPE\t\t\t=> 0x%02x\n", header->cputype);
	printf("FILE TYPE\t\t=> 0x%02x\n", header->filetype);
	printf("NCMDS\t\t\t=> 0x%02x\n", header->ncmds);
	printf("SIZE\t\t\t=> 0x%02x\n", header->sizeofcmds);
	printf("FLAGS\t\t\t=> 0x%02x\n", header->flags);

	return header->ncmds;
}

char * get_flags(vm_prot_t flags)
{

	if (flags == 0)
		return "VM_PROT_NONE";

	vm_prot_t all_flags[] = {VM_PROT_NONE, VM_PROT_READ, VM_PROT_WRITE, 0, VM_PROT_EXECUTE};
	char *all_flags_c[] = {"VM_PROT_NONE", "VM_PROT_READ", "VM_PROT_WRITE", "", "VM_PROT_EXECUTE"};

	char *result = malloc(100);

	for(int i = 0; i < 5; i++) {
		if (flags & all_flags[i] && i != 3) {
			strcat(result, all_flags_c[all_flags[i]]);
			strcat(result, " | ");
		}
	}

	/* Find last | and put \0 there*/
	for (int i = strlen(result)-1; i >= 0; i--) {
		if (result[i] == '|') {
			result[i] = '\0';
			break;
		}
	}

	return result;
}

void dump_lc_segments(FILE *obj_file)
{
	struct segment_command_64 *seg = malloc(sizeof(struct segment_command_64));
	fread(seg, sizeof(struct segment_command_64), 1, obj_file);
	printf(RED "\t\tSegment name:\t\t" RESET CYAN "%s\n" RESET, seg->segname);
	printf(RED "\t\tVM Address:\t\t" RESET CYAN "%lu\n" RESET, seg->vmaddr);
	printf(RED "\t\tVM Size:\t\t" RESET CYAN "%lu\n" RESET, seg->vmsize);
	printf(RED "\t\tFile Offset:\t\t" RESET CYAN "%lu\n" RESET, seg->fileoff);
	printf(RED "\t\tFile Size:\t\t" RESET CYAN "%lu\n" RESET, seg->filesize);
	printf(RED "\t\tMaximum VM Protection:" TAB RESET CYAN "%lu\n" RESET, seg->maxprot);
	printf(TAB TAB TAB TAB TAB CYAN "%s\n" RESET, get_flags(seg->maxprot));
	printf(TAB TAB RED "Initial VM Protection:" TAB RESET CYAN "%lu\n" RESET, seg->initprot);
	printf(TAB TAB TAB TAB TAB CYAN "%s\n" RESET, get_flags(seg->initprot));
	printf(TAB TAB RED "Number of sections:" TAB RESET CYAN "%lu\n" RESET, seg->nsects);

	/*
	Dumping sections of each segment
	for (int i = 0; i < seg->nsects; i++) {
		struct section_64 *sect = malloc(sizeof(struct section_64));
		fread(sect, sizeof(struct section_64), 1, obj_file);
		printf("\t\t\t\tSection => %s\n", sect->sectname);
	}*/

	free(seg);
}

void dump_segments(FILE *obj_file, uint32_t ncmds)
{
	int offset = sizeof(struct mach_header_64);
	for (int i = 0; i < ncmds; i++) {
		struct load_command * lc = malloc(sizeof(struct load_command));
		fread(lc, sizeof(struct load_command), 1, obj_file);

		if (lc->cmd == LC_SEGMENT_64) {
			printf(CYAN "LC_SEGMENT_64:\n" RESET);
			fseek(obj_file, offset, SEEK_SET);
			dump_lc_segments(obj_file);
		}

		if (lc->cmd == LC_UUID) {
			printf(CYAN "LC_UUID:\n" RESET);
			fseek(obj_file, offset, SEEK_SET);
			struct uuid_command *uuid = malloc(sizeof(struct uuid_command));
			fread(uuid, sizeof(struct uuid_command), 1, obj_file);
			printf(TAB TAB RED "UUID:\t\t\t" RESET CYAN);
			for (int j = 0; j < 16; j++) {
				if (j == 4 || j == 6 || j == 8 || j == 10)
					printf("-");
				printf("%02x", uuid->uuid[j]);
			}
			printf(RESET "\n");

			free(uuid);
		}

		if (lc->cmd == LC_LOAD_DYLIB) {
			printf("LC_LOAD_DYLIB:\n\n");
			fseek(obj_file, offset, SEEK_SET);
			struct dylib_command *dcmd = malloc(sizeof(struct dylib_command));
			fread(dcmd, sizeof(struct dylib_command), 1, obj_file);
			printf("\t\tcmdsize = %d\n", dcmd->cmdsize);
			printf("\t\t%s\n", get_version(dcmd->dylib.current_version));
		}

		if (lc->cmd == LC_VERSION_MIN_IPHONEOS) {
			printf("LC_VERSION_MIN_IPHONEOS:\n\n");
			fseek(obj_file, offset, SEEK_SET);
			struct version_min_command *v = malloc(sizeof(struct version_min_command));
			fread(v, sizeof(struct version_min_command), 1, obj_file);
			printf("\t\t%s\n", get_version(v->version));
		}


		offset += lc->cmdsize;
		fseek(obj_file, offset, SEEK_SET);
		free(lc);

	}
}

void dumper(FILE *obj_file)
{
	uint32_t magic = get_magic(obj_file, 0);
	if (magic == 0xfeedfacf) {
		printf("ARCH => 64 (Magic number: 0x%02x)\n", magic);
	} else {
		printf("ARCH => 32 (Magic number: 0x%02x)\n", magic);
	}
	fseek(obj_file, 0, SEEK_SET);
	uint32_t ncmds = dump_header(obj_file);
	dump_segments(obj_file, ncmds);
}
