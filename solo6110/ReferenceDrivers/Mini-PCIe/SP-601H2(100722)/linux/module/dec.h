#ifndef __DEC_H
#define __DEC_H

void solo6x10_dec_lock(struct SOLO6x10 *solo6x10);
void solo6x10_dec_unlock(struct SOLO6x10 *solo6x10);

int solo6x10_decoder_interrupt(struct SOLO6x10 *solo6x10);
int solo6x10_decoder_update(struct SOLO6x10 *solo6x10, int fi);
unsigned int change_decoder_ability(struct SOLO6x10 *solo6x10, unsigned int ability);
int solo6x10_dec_init(struct SOLO6x10 *solo6x10);
void solo6x10_dec_exit(struct SOLO6x10 *solo6x10);

#endif

