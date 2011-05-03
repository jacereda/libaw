static __inline void bitset(unsigned char * b, unsigned bit) {
        b[bit>>3] |= 1 << (bit & 7);
}

static __inline void bitclear(unsigned char * b, unsigned bit) {
        b[bit>>3] &= ~(1 << (bit & 7));
}

static __inline void bitassign(unsigned char * b, unsigned bit, int val) {
	if (val)
		bitset(b, bit);
	else
		bitclear(b, bit);
}

static __inline int bittest(unsigned char * b, unsigned bit) {
        return b[bit>>3] & (1 << (bit & 7));
}

