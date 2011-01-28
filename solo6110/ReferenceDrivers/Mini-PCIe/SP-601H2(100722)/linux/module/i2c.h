
int solo6x10_i2c_interrupt(struct SOLO6x10 *solo6x10);
int solo6x10_i2c_init(struct SOLO6x10 *solo6x10);
void solo6x10_i2c_exit(struct SOLO6x10 *solo6x10);

u8 solo6x10_i2c_readbyte(struct SOLO6x10 *solo6x10, int i2c_channel, u8 dev_addr, u8 sub_addr);
void solo6x10_i2c_writebyte(struct SOLO6x10 *solo6x10, int i2c_channel, u8 dev_addr, u8 sub_addr, u8 data);

