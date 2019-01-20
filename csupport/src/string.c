#include <stddef.h>

void* memcpy(void* destination, const void* source, size_t num) {

	unsigned char* des = (unsigned char*)destination;
	unsigned char* src = (unsigned char*)source;

	size_t i = 0;
	while (i < num) {
		des[i] = src[i];
		i++;
	}

	return destination;
}

void* memset(void* destination, int c, size_t num) {

    unsigned char* des = (unsigned char*)destination;
	unsigned char   c8 = (unsigned char )c;

    size_t i = 0;
    while (i < num) {
        des[i] = c8;
        i++;
    }

    return destination;
}
