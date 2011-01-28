#ifndef __ENC_H
#define __ENC_H

int solo6x10_encoder_interrupt(struct SOLO6x10 *solo6x10);
unsigned int change_encoder_ability(struct SOLO6x10 *solo6x10, unsigned int ability);
int solo6x10_enc_init(struct SOLO6x10 *solo6x10);
void solo6x10_enc_exit(struct SOLO6x10 *solo6x10);

#endif

