#include "oled.h"
#include "oled_font.h"
#include <string.h>

/* Display dimensions for standard 128x64 SSD1306 */
#define OLED_WIDTH    128
#define OLED_HEIGHT   64
#define OLED_PAGES    (OLED_HEIGHT / 8)

/* Local Framebuffer */
static uint8_t s_oled_buffer[OLED_PAGES][OLED_WIDTH];

/* Helper to send command to OLED */
static void OLED_WriteCommand(const OLED_Config_t *cfg, uint8_t cmd)
{
    SoftI2C_Start(&cfg->i2c_cfg);
    SoftI2C_SendByte(&cfg->i2c_cfg, (cfg->i2c_addr << 1) | 0x00); /* Write mode */
    SoftI2C_WaitAck(&cfg->i2c_cfg);
    SoftI2C_SendByte(&cfg->i2c_cfg, 0x00); /* Co = 0, D/C = 0 (Command) */
    SoftI2C_WaitAck(&cfg->i2c_cfg);
    SoftI2C_SendByte(&cfg->i2c_cfg, cmd);
    SoftI2C_WaitAck(&cfg->i2c_cfg);
    SoftI2C_Stop(&cfg->i2c_cfg);
}

/* Helper to send data to OLED */
static void OLED_WriteData(const OLED_Config_t *cfg, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    SoftI2C_Start(&cfg->i2c_cfg);
    SoftI2C_SendByte(&cfg->i2c_cfg, (cfg->i2c_addr << 1) | 0x00);
    SoftI2C_WaitAck(&cfg->i2c_cfg);
    SoftI2C_SendByte(&cfg->i2c_cfg, 0x40); /* Co = 0, D/C = 1 (Data) */
    SoftI2C_WaitAck(&cfg->i2c_cfg);
    
    for (i = 0; i < len; i++) {
        SoftI2C_SendByte(&cfg->i2c_cfg, data[i]);
        SoftI2C_WaitAck(&cfg->i2c_cfg);
    }
    SoftI2C_Stop(&cfg->i2c_cfg);
}

void OLED_Init(const OLED_Config_t *cfg)
{
    /* Initialize the I2C lines to idle state */
    cfg->i2c_cfg.sda_high();
    cfg->i2c_cfg.scl_high();
    
    cfg->i2c_cfg.delay_us(100000); /* wait 100ms for OLED to boot */

    OLED_WriteCommand(cfg, OLED_CMD_DISPLAY_OFF);
    OLED_WriteCommand(cfg, OLED_CMD_SET_DISPLAY_CLOCK_DIV);
    OLED_WriteCommand(cfg, 0x80); // Suggested ratio
    OLED_WriteCommand(cfg, OLED_CMD_SET_MULTIPLEX);
    OLED_WriteCommand(cfg, 0x3F); // 64 mux
    OLED_WriteCommand(cfg, OLED_CMD_SET_DISPLAY_OFFSET);
    OLED_WriteCommand(cfg, 0x00);
    OLED_WriteCommand(cfg, OLED_CMD_SET_START_LINE | 0x00);
    OLED_WriteCommand(cfg, OLED_CMD_CHARGE_PUMP);
    OLED_WriteCommand(cfg, 0x14); // Enable charge pump
    OLED_WriteCommand(cfg, OLED_CMD_SET_MEMORY_ADDR_MODE);
    OLED_WriteCommand(cfg, 0x00); // Horizontal addressing mode
    OLED_WriteCommand(cfg, OLED_CMD_SEG_REMAP_REVERSE);
    OLED_WriteCommand(cfg, OLED_CMD_COM_SCAN_DEC);
    OLED_WriteCommand(cfg, OLED_CMD_SET_COM_PINS);
    OLED_WriteCommand(cfg, 0x12);
    OLED_WriteCommand(cfg, OLED_CMD_SET_CONTRAST);
    OLED_WriteCommand(cfg, 0xCF);
    OLED_WriteCommand(cfg, OLED_CMD_SET_PRECHARGE);
    OLED_WriteCommand(cfg, 0xF1);
    OLED_WriteCommand(cfg, OLED_CMD_SET_VCOM_DESELECT);
    OLED_WriteCommand(cfg, 0x40);
    OLED_WriteCommand(cfg, OLED_CMD_DISPLAY_ALL_ON_RESUME);
    OLED_WriteCommand(cfg, OLED_CMD_DISPLAY_NORMAL);
    
    OLED_Clear(cfg);
    OLED_Update(cfg);
    
    OLED_WriteCommand(cfg, OLED_CMD_DISPLAY_ON);
}

void OLED_Clear(const OLED_Config_t *cfg)
{
    (void)cfg; /* Ignore cfg here if unused, handled in update */
    memset(s_oled_buffer, 0x00, sizeof(s_oled_buffer));
}

void OLED_Update(const OLED_Config_t *cfg)
{
    uint8_t i;
    for (i = 0; i < OLED_PAGES; i++) {
        OLED_WriteCommand(cfg, OLED_CMD_SET_PAGE_START + i);
        OLED_WriteCommand(cfg, OLED_CMD_SET_LOW_COL);
        OLED_WriteCommand(cfg, OLED_CMD_SET_HIGH_COL);
        OLED_WriteData(cfg, s_oled_buffer[i], OLED_WIDTH);
    }
}

void OLED_Print(const OLED_Config_t *cfg, uint8_t x, uint8_t y, const char *str)
{
    (void)cfg; // Operation is on buffer only
    if (y >= OLED_PAGES) return;

    while (*str) {
        if (x + 6 >= OLED_WIDTH) {
            x = 0;
            y++;
            if (y >= OLED_PAGES) return;
        }

        uint8_t char_idx = *str - ' ';
        if (char_idx < (sizeof(OLED_Font_6x8) / 6)) {
            uint8_t i;
            for (i = 0; i < 6; i++) {
                s_oled_buffer[y][x + i] = OLED_Font_6x8[char_idx][i];
            }
        }
        x += 6;
        str++;
    }
}


