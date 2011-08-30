
int solo6010_i2c_interrupt(struct SOLO6010 *solo6010);
int solo6010_i2c_init(struct SOLO6010 *solo6010);
void solo6010_i2c_exit(struct SOLO6010 *solo6010);

u8 solo6010_i2c_readbyte(struct SOLO6010 *solo6010, int i2c_channel, u8 dev_addr, u8 sub_addr);
void solo6010_i2c_writebyte(struct SOLO6010 *solo6010, int i2c_channel, u8 dev_addr, u8 sub_addr, u8 data);

