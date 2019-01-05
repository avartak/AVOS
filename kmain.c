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

void cls() {
  
    char* video_memory = (char*)0xb8000;

    int i = 0;
    while (i < 80*25*2) {
        video_memory[i] = 0;
        i++;
    }

}

void print(const char* str, char attr, int line, int pos) {

    char* video_memory = (char*)0xb8000;

	int startpos = (line*80 + pos)*2;

    int i = 0;
    while (str[i] != 0) {
        video_memory[startpos + 2*i]   = str[i];
        video_memory[startpos + 2*i+1] = attr;
        i++;
    }
}

void kmain() {

    char*  str  = "Welcome to AVOS!";
    char  attr  = 0x04;
    int   line  = 0;
	int    pos  = 32; 

	cls();
    
	print(str, attr, line, pos);

    return;
}

