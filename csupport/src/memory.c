#include <stdint.h>

void* memcpy (void* destination, const void* source, uint32_t num) {

	char* des = (char*)destination;
	char* src = (char*)source;

	uint32_t i = 0;
	while (i < num) {
		des[i] = src[i];
		i++;
	}

	return destination;
}

