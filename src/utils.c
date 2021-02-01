#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"


void print_message()
{
	
}

int get_nibble(uint32_t ver, int idx)
{
	return (ver >> 4 * idx) & 0xF;
}

char * trim_zero(char *buf)
{
	char *subs = buf;
	char *trimmed = malloc(strlen(buf) + 1);
	int pos = 0;
	int j = 0;

	while (*(buf++) == '0')
		pos++;

	buf = subs;

	while (buf[pos]) {
		trimmed[j++] = buf[pos++];
	}

	trimmed[j] = '\0';

	return trimmed;
}

char * squeeze(char * buf) {
	char * new_one = malloc(strlen(buf)+1);
	char c;
	int j = 0;

	while(*buf) {
		c = *buf;
		while (c == *(++buf))
			;
		new_one[j++] = c;
	}

	new_one[j] = '\0';

	return trim_zero(new_one);
}

char * get_version(uint32_t ver)
{
	char version[11];

	version[9] = get_nibble(ver, 0) + '0'; 
	version[8] = get_nibble(ver, 1) + '0';
	version[7] = '.';
	version[6] = get_nibble(ver, 2) + '0';
	version[5] = get_nibble(ver, 3) + '0';
	version[4] = '.';
	version[3] = get_nibble(ver, 4) + '0';
	version[2] = get_nibble(ver, 5) + '0';
	version[1] = get_nibble(ver, 6) + '0';
	version[0] = get_nibble(ver, 7) + '0';
	version[10] = '\0';


	return squeeze(version);
}