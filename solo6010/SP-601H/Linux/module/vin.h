#ifndef __VIN_H
#define __VIN_H

int solo6010_motion_interrupt(struct SOLO6010 *solo6010);
int solo6010_vin_init(struct SOLO6010 *solo6010);
void solo6010_vin_exit(struct SOLO6010 *solo6010);

#endif

