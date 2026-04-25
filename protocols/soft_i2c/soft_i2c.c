/**
 * @file    soft_i2c.c
 * @brief   Software I2C master implementation.
 *
 * All bus operations are performed exclusively through the function pointers
 * inside the @ref SoftI2C_Config_t handle.  No MCU-specific headers,
 * registers, or GPIO macros appear in this file.
 */

#include "soft_i2c.h"

/* -------------------------------------------------------------------------
 * Internal helper macro
 * ---------------------------------------------------------------------- */

/**
 * Half-period delay used between every clock edge.
 * A value of 2 us gives a theoretical ~250 kHz SCL.
 * To use a lower clock rate, change this compile-time constant.
 */
#define I2C_HALF_PERIOD_US (2U)

/* -------------------------------------------------------------------------
 * Public function implementations
 * ---------------------------------------------------------------------- */

void SoftI2C_Start(const SoftI2C_Config_t *cfg)
{
    /* Ensure SDA and SCL start high (idle state) */
    cfg->sda_high();
    cfg->scl_high();
    cfg->delay_us(I2C_HALF_PERIOD_US);

    /* Pull SDA low while SCL is still high -> START condition */
    cfg->sda_low();
    cfg->delay_us(I2C_HALF_PERIOD_US);

    /* Pull SCL low to begin the first bit period */
    cfg->scl_low();
    cfg->delay_us(I2C_HALF_PERIOD_US);
}

void SoftI2C_Stop(const SoftI2C_Config_t *cfg)
{
    /* SCL is expected to be low on entry; ensure SDA is also low */
    cfg->sda_low();
    cfg->delay_us(I2C_HALF_PERIOD_US);

    /* Release SCL high */
    cfg->scl_high();
    cfg->delay_us(I2C_HALF_PERIOD_US);

    /* Release SDA high while SCL is high -> STOP condition */
    cfg->sda_high();
    cfg->delay_us(I2C_HALF_PERIOD_US);
}

void SoftI2C_SendByte(const SoftI2C_Config_t *cfg, uint8_t byte)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++) {
        /* Present MSB first */
        if (byte & 0x80U) {
            cfg->sda_high();
        } else {
            cfg->sda_low();
        }
        byte <<= 1U;

        cfg->delay_us(I2C_HALF_PERIOD_US);
        cfg->scl_high();
        cfg->delay_us(I2C_HALF_PERIOD_US);
        cfg->scl_low();
        cfg->delay_us(I2C_HALF_PERIOD_US);
    }
}

uint8_t SoftI2C_WaitAck(const SoftI2C_Config_t *cfg)
{
    uint8_t ack;

    /* Release SDA so the slave can drive it */
    cfg->sda_high();
    cfg->delay_us(I2C_HALF_PERIOD_US);

    /* Raise SCL to sample the ACK bit */
    cfg->scl_high();
    cfg->delay_us(I2C_HALF_PERIOD_US);

    /* A LOW on SDA means ACK; HIGH means NACK */
    ack = cfg->sda_read();

    /* Pull SCL low to end the ACK bit period */
    cfg->scl_low();
    cfg->delay_us(I2C_HALF_PERIOD_US);

    return (ack == 0U) ? SOFT_I2C_OK : SOFT_I2C_NACK;
}

uint8_t SoftI2C_ReadByte(const SoftI2C_Config_t *cfg, uint8_t ack)
{
    uint8_t i;
    uint8_t byte = 0U;

    /* Release SDA so the slave can drive it */
    cfg->sda_high();

    for (i = 0U; i < 8U; i++) {
        byte <<= 1U;

        cfg->delay_us(I2C_HALF_PERIOD_US);
        cfg->scl_high();
        cfg->delay_us(I2C_HALF_PERIOD_US);

        /* Sample SDA on the rising edge of SCL */
        if (cfg->sda_read()) {
            byte |= 0x01U;
        }

        cfg->scl_low();
        cfg->delay_us(I2C_HALF_PERIOD_US);
    }

    /* Send ACK or NACK to the slave */
    if (ack) {
        cfg->sda_low();  /* ACK: master pulls SDA low */
    } else {
        cfg->sda_high(); /* NACK: master releases SDA high */
    }

    cfg->delay_us(I2C_HALF_PERIOD_US);
    cfg->scl_high();
    cfg->delay_us(I2C_HALF_PERIOD_US);
    cfg->scl_low();
    cfg->delay_us(I2C_HALF_PERIOD_US);

    return byte;
}
