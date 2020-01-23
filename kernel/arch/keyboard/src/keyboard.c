#include <kernel/arch/keyboard/include/keyboard.h>
#include <kernel/arch/i386/include/ioports.h>
#include <kernel/arch/apic/include/apic.h>
#include <kernel/arch/console/include/console.h>
#include <kernel/core/setup/include/setup.h>

uint32_t Keyboard_screen_pos = 80;

static uint8_t Keyboard_shiftcode[256] = 
{
	[0x1D] KEY_CTL,
	[0x2A] KEY_SHIFT,
	[0x36] KEY_SHIFT,
	[0x38] KEY_ALT,
	[0x9D] KEY_CTL,
	[0xB8] KEY_ALT
};

static uint8_t Keyboard_togglecode[256] =
{
	[0x3A] KEY_CAPSLOCK,
	[0x45] KEY_NUMLOCK,
	[0x46] KEY_SCROLLLOCK
};

static uint8_t Keyboard_normalmap[256] =
{
	NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
	'7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
	'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
	'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
	'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
	'\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
	'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
	NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
	NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
	'8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
	'2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
	[0x9C] '\n',      // KP_Enter
	[0xB5] '/',       // KP_Div
	[0xC8] KEY_UP,    [0xD0] KEY_DN,
	[0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
	[0xCB] KEY_LF,    [0xCD] KEY_RT,
	[0x97] KEY_HOME,  [0xCF] KEY_END,
	[0xD2] KEY_INS,   [0xD3] KEY_DEL
};

static uint8_t Keyboard_shiftmap[256] = 
{
	NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
	'&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
	'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
	'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
	'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
	'"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
	'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
	NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
	NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
	'8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
	'2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
	[0x9C] '\n',      // KP_Enter
	[0xB5] '/',       // KP_Div
	[0xC8] KEY_UP,    [0xD0] KEY_DN,
	[0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
	[0xCB] KEY_LF,    [0xCD] KEY_RT,
	[0x97] KEY_HOME,  [0xCF] KEY_END,
	[0xD2] KEY_INS,   [0xD3] KEY_DEL
};

static uint8_t Keyboard_ctlmap[256] =
{
	NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
	NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
	C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
	C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
	C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
	NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
	C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
	[0x9C] '\r',      // KP_Enter
	[0xB5] C('/'),    // KP_Div
	[0xC8] KEY_UP,    [0xD0] KEY_DN,
	[0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
	[0xCB] KEY_LF,    [0xCD] KEY_RT,
	[0x97] KEY_HOME,  [0xCF] KEY_END,
	[0xD2] KEY_INS,   [0xD3] KEY_DEL
};


void Keyboard_Initialize() {
    APIC_IO_EnableInterrupt(1, 0x21, APIC_Local_ID());
}

uint8_t Keyboard_GetChar() {
	static uint32_t shift;
	static uint8_t *charcode[4] = {Keyboard_normalmap, Keyboard_shiftmap, Keyboard_ctlmap, Keyboard_ctlmap};
	
	if ((X86_Inb(KEYBOARD_STAT_PORT) & KEYBOARD_DATA_IN_BUFFER) == 0) return -1;
	
	uint32_t data = X86_Inb(KEYBOARD_DATA_PORT);
	if (data == 0xE0) {
		shift |= KEY_E0ESC;
		return 0;
	} 
	else if (data & 0x80) {
		// Key released
		data = (shift & KEY_E0ESC ? data : data & 0x7F);
		shift &= ~(Keyboard_shiftcode[data] | KEY_E0ESC);
		return 0;
	} 
	else if (shift & KEY_E0ESC) {
		// Last character was an E0 escape; or with 0x80
		data |= 0x80;
		shift &= ~KEY_E0ESC;
	}
	
	shift |= Keyboard_shiftcode[data];
	shift ^= Keyboard_togglecode[data];
	uint8_t c = charcode[shift & (KEY_CTL | KEY_SHIFT)][data];
	if (shift & KEY_CAPSLOCK) {
		if      ('a' <= c && c <= 'z') c += 'A' - 'a';
		else if ('A' <= c && c <= 'Z') c += 'a' - 'A';
	}
	return c;
}

void Keyboard_HandleInterrupt() {

	Console_PrintChar(Keyboard_GetChar());
}


