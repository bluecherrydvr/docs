#ifndef __DEC_H
#define __DEC_H

int solo6010_decoder_interrupt(struct SOLO6010 *solo6010);
unsigned int change_decoder_ability(struct SOLO6010 *solo6010, unsigned int ability);
int solo6010_dec_init(struct SOLO6010 *solo6010);
void solo6010_dec_exit(struct SOLO6010 *solo6010);

#endif

