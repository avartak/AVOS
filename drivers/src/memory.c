void* memcpy (void* destination, const void* source, unsigned num) {

	char* des = (char*)destination;
	char* src = (char*)source;

	int i = 0;
	while (i < num) {
		des[i] = src[i];
		i++;
	}

	return destination;
}

