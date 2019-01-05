void cls() {

	char* video_memory = (char*)0xb8000;

    int i = 0;
	while (i < 80*25*2) {
		video_memory[i] = 0;
        i++;
	}

}

void print(const char* str, char attr) {

	char* video_memory = (char*)0xb8000;

    int i = 0;
	while (str[i] != 0) {
		video_memory[2*i] = str[i];
		video_memory[2*i+1] = attr;
        i++;
	}
}

void kmain() {

    char str[] = "AVOS!";
    char attr  = 0x04;

	char* video_memory = (char*)0xb8000;

	cls();
    
	print(str, attr);

    return;
}

