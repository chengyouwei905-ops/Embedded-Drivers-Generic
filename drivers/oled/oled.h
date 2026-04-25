#ifndef OLED_H
#define OLED_H

#include "../../protocols/soft_i2c/soft_i2c.h"

/* -------------------------------------------------------------------------
 * OLED (SSD1306) Command Set
 * ---------------------------------------------------------------------- */
#define OLED_CMD_SET_LOW_COL            0x00
#define OLED_CMD_SET_HIGH_COL           0x10
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20
#define OLED_CMD_SET_COL_ADDR           0x21
#define OLED_CMD_SET_PAGE_ADDR          0x22
#define OLED_CMD_SET_START_LINE         0x40
#define OLED_CMD_SET_CONTRAST           0x81
#define OLED_CMD_CHARGE_PUMP            0x8D
#define OLED_CMD_SEG_REMAP_NORMAL       0xA0
#define OLED_CMD_SEG_REMAP_REVERSE      0xA1
#define OLED_CMD_DISPLAY_ALL_ON_RESUME  0xA4
#define OLED_CMD_DISPLAY_ALL_ON         0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERT         0xA7
#define OLED_CMD_SET_MULTIPLEX          0xA8
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF
#define OLED_CMD_SET_PAGE_START         0xB0
#define OLED_CMD_COM_SCAN_INC           0xC0
#define OLED_CMD_COM_SCAN_DEC           0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3
#define OLED_CMD_SET_DISPLAY_CLOCK_DIV  0xD5
#define OLED_CMD_SET_PRECHARGE          0xD9
#define OLED_CMD_SET_COM_PINS           0xDA
#define OLED_CMD_SET_VCOM_DESELECT      0xDB

/* -------------------------------------------------------------------------
 * Configuration / handle type
 * ---------------------------------------------------------------------- */
typedef struct {
    SoftI2C_Config_t i2c_cfg;
    uint8_t i2c_addr;
} OLED_Config_t;

/* -------------------------------------------------------------------------
 * Public APIs
 * ---------------------------------------------------------------------- */
void OLED_Init(const OLED_Config_t *cfg);
void OLED_Print(const OLED_Config_t *cfg, uint8_t x, uint8_t y, const char *str);
void OLED_Clear(const OLED_Config_t *cfg);
void OLED_Update(const OLED_Config_t *cfg);

#endif /* OLED_H */
