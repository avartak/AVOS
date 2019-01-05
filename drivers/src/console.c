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

