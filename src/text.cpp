#include <common.hpp>
#include <cstring>

static size_t base = 0;
static size_t* row = nullptr;
static size_t* col = nullptr;
static char color = DEFAULT_COLOR;

void init(size_t fb, size_t* sync) {
	base = fb;
	row = &sync[0];
	col = &sync[1];
}

inline static char* getChar(size_t r, size_t c) {
	return ((char*)base) + (COLS * r + c)*2;
}

inline static char* getVideo() { return getChar(*row, *col); }

inline static void clearRow(size_t r) {
	char* buffer = getChar(r, 0);
	for(size_t i=0; i<COLS; ++i) {
		*buffer++ = ' ';
		*buffer++ = color;
	}
}

inline static void scroll() {
	void* from = (void*)(base + BYTES_PER_ROW);
	// Move up
	memmove((void*)base, from, (ROWS-1)*BYTES_PER_ROW);
	// Go up
	--*row;
	// Clear last line
	clearRow(ROWS-1);
}

static void goAhead() {
	// Next column
	++*col;

	if(*col >= COLS) {
		// Have to go the next line
		++*row;
		*col = 0;

		if(*row >= ROWS) {
			// Have to scroll
			scroll();
		}
	}
}

static void goBack() {
	// This doesn't scroll!
	if(!*col) {
		--*row;
		*col = COLS-1;
	} else {
		--*col;
	}
}

static bool colorMode = false;
void writec(char c) {
	if(colorMode) {
		color = c;
		colorMode = false;
		return;
	} else if(c == 033) {
		colorMode = true;
		return;
	} else if(c == '\n') {
		++*row;
		*col = 0;
		if(*row >= ROWS) scroll();
		return;
	} else if(c == '\b') {
		goBack();
		return;
	}

	char* video = getVideo();
	*video++ = c;
	*video = color;
	goAhead();
}

void writes(const char* str, size_t sz) {
	while(sz--)
		writec(*str++);

	if(*col == COLS-1) {
		if(*row == ROWS-1) {
			// It's not worth scrolling
			updateCursor(*row, *col);
		} else {
			updateCursor(*row+1, 0);
		}
	} else {
		if(*getChar(*row, *col+1) == ' ') {
			updateCursor(*row, *col);
		} else {
			updateCursor(*row, *col+1);
		}
	}
}



void resetColor() { color = DEFAULT_COLOR; }
void setColor(uint8_t c) { color = c; }

void clear() {
	for(size_t i=0; i<ROWS; ++i)
		clearRow(i);
	*row = *col = 0;
	updateCursor(0, 0);
}
