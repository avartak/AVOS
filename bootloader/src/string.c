#include <stddef.h>

void* memcpy(void* destination, const void* source, size_t num) {

	unsigned char* des = (unsigned char*)destination;
	const unsigned char* src = (const unsigned char*)source;

	size_t i = 0;
	while (i < num) {
		des[i] = src[i];
		i++;
	}

	return destination;
}

void* memmove(void* destination, const void* source, size_t size) {

	unsigned char* des = (unsigned char*) destination;
	const unsigned char* src = (const unsigned char*) source;

	if (des < src) {
		for (size_t i = 0; i < size; i++) des[i] = src[i];
	} 
	else {
		for (size_t i = size; i != 0; i--) des[i-1] = src[i-1];
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

int memcmp(const void *s1, const void *s2, size_t num) {

	const unsigned char* str1 = (const unsigned char*)s1;
	const unsigned char* str2 = (const unsigned char*)s2;

    size_t i = 0;
	int retval = 0;
    while (i < num) {
		if (str1[i] < str2[i]) {
			retval = -1;
			break;
		}
		if (str1[i] > str2[i]) {
			retval = 1;
			break;
		}
		i++;
	}

	return retval;

}

size_t strlen(const char* string) {

	size_t len = 0;
	while (string[len] != 0) len++;
	return len;

}
