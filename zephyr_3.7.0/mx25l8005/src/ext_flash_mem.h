#ifndef __EXT_FLASH_MEM_H
#define __EXT_FLASH_MEM_H
// Some terminology
// A page is a 256 Byte section of memory
// A sector is a 4096 Byte section of memory
// A block is a 65536 Byte (64kB) section of memory
// Can write pages
// Can erase sectors or block or the entire chip
// Wiring (uses SPI 2 in NRF52833)
// CS : Port 1, bit 2
// MOSI : Port 0, bit 13
// MISO : Port 0, bit 1 
// SCK : Port 0, bit 17
// HOLD : pulled high to 3.3V using a few kOhms
// WP : pulled high to 3.3V using a few kOhms

int init_spi(void);
void spi_exchange(uint8_t * tx_data, uint32_t tx_len, uint8_t * rx_data, uint32_t expected_rx_len);
int read_electronic_signature(uint8_t *sig,uint32_t sig_len);

int write_enable(void);
int write_disable(void);
int read_status_register(uint8_t *status);
int write_status_register(uint8_t status);
int power_down(void);
int power_up(void);
int read_data(uint32_t address, uint8_t *data, uint32_t len);
int page_program(uint32_t address, uint8_t *data, uint32_t len); // program page.  8LSB's of address should be 0.  len <= 256;
int sector_erase(uint32_t sector_address);
int block_erase(uint32_t block_address);
int chip_erase();
int busy();
#endif
