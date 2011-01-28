#ifndef __VIN_H
#define __VIN_H

int solo6x10_motion_interrupt(struct SOLO6x10 *solo6x10);
int solo6x10_vin_init(struct SOLO6x10 *solo6x10);
void solo6x10_vin_exit(struct SOLO6x10 *solo6x10);

#endif

