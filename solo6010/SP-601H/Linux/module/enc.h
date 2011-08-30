#ifndef __ENC_H
#define __ENC_H

int solo6010_encoder_interrupt(struct SOLO6010 *solo6010);
unsigned int change_encoder_ability(struct SOLO6010 *solo6010, unsigned int ability);
int solo6010_enc_init(struct SOLO6010 *solo6010);
void solo6010_enc_exit(struct SOLO6010 *solo6010);

#endif

