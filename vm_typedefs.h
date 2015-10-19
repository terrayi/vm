
#ifndef _VM_TYPEDEFS_H
#define _VM_TYPEDEFS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned short ushort;

union reg_u {
	unsigned val;
	struct {
		ushort l;
		ushort h;
	} words;
	struct {
		byte ll;
		byte lh;
		byte hl;
		byte hh;
	} bytes;
};

typedef union reg_u reg_t;

#ifdef __cplusplus
}
#endif

#endif

//EOF
