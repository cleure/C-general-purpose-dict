#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "dict.h"

#define DEV_RANDOM "/dev/urandom"

char *keys[] = {
	"foo",
	"bar",
	"biz",
	"baz",
	"etc",
	"qny",
	"os1",
	"walter",
	"factory",
	"singleton",
	"3d",
	"world",
	"mario",
	"land",
	"garbage",
	"target"
};

void get_random_bytes(void *ptr, size_t bytes)
{
	FILE *fp;
	
	fp = fopen(DEV_RANDOM, "rb");
	fread(ptr, 1, bytes, fp);
	fclose(fp);
}

int main(void)
{
	size_t	i,
			len = sizeof(keys) / sizeof(keys[0]);
			
	uint32_t seed;
	struct dict *d;
	struct dict_node *n;
	
	get_random_bytes(&seed, sizeof(seed));
	printf("Seed: %u\n", seed);
	
	d = dict_new(seed, 512, free, free);
	for (i = 0; i < len; i++) {
		dict_set(d, strdup(keys[i]), strdup(keys[i]));
	}
	
	dict_resize(d, 128);
	n = dict_get(d, "foo");
	printf("%s\n", n->key);

	dict_delete(d);

	exit(0);
}
