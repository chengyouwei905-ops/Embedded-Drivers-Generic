/**
 * @file    soft_i2c.h
 * @brief   Hardware-independent Software I2C protocol driver.
 *
 * This module implements a bit-banged I2C master using function pointers
 * supplied by the application layer, keeping the driver fully decoupled
 * from any specific MCU or register map.
 *
 * Usage
 * -----
 * -# Define the GPIO-toggle functions and a microsecond-delay function that
 *    match the signatures required by SoftI2C_Config_t.
 * -# Populate a SoftI2C_Config_t instance with those function pointers.
 * -# Pass a pointer to that instance to every SoftI2C_* function.
 *
 * @note No hardware-specific code appears in this file or its companion
 *       soft_i2c.c.  All GPIO/timing operations go through the config handle.
 */

#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Configuration / handle type
 * ---------------------------------------------------------------------- */

/**
 * @brief Hardware abstraction handle for the software I2C driver.
 *
 * Populate all six function pointers before passing this struct to any
 * SoftI2C_* function.  The driver never accesses hardware directly; every
 * GPIO toggle and delay goes through these callbacks.
 *
 * STM32 HAL example:
 * @code{.c}
 * static void my_sda_high(void)
 * {
 *     HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET);
 * }
 * static void my_sda_low(void)
 * {
 *     HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET);
 * }
 * static void my_scl_high(void)
 * {
 *     HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET);
 * }
 * static void my_scl_low(void)
 * {
 *     HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET);
 * }
 * static uint8_t my_sda_read(void)
 * {
 *     return (uint8_t)HAL_GPIO_ReadPin(SDA_GPIO, SDA_PIN);
 * }
 * static void my_delay_us(uint32_t us)
 * {
 *     busy_wait_us(us); // platform-specific implementation
 * }
 *
 * SoftI2C_Config_t i2c_cfg = {
 *     .sda_high  = my_sda_high,
 *     .sda_low   = my_sda_low,
 *     .scl_high  = my_scl_high,
 *     .scl_low   = my_scl_low,
 *     .sda_read  = my_sda_read,
 *     .delay_us  = my_delay_us,
 * };
 * @endcode
 */
typedef struct {
    /** @brief Drive the SDA line HIGH (input/open-drain release). */
    void (*sda_high)(void);

    /** @brief Drive the SDA line LOW. */
    void (*sda_low)(void);

    /** @brief Drive the SCL line HIGH. */
    void (*scl_high)(void);

    /** @brief Drive the SCL line LOW. */
    void (*scl_low)(void);

    /**
     * @brief Read the current logic level of the SDA line.
     * @return 1 if SDA is HIGH, 0 if SDA is LOW.
     */
    uint8_t (*sda_read)(void);

    /**
     * @brief Block for the requested number of microseconds.
     * @param us  Delay duration in microseconds.
     */
    void (*delay_us)(uint32_t us);
} SoftI2C_Config_t;

/* -------------------------------------------------------------------------
 * Return codes
 * ---------------------------------------------------------------------- */

/** @brief Operation completed successfully / ACK received from slave. */
#define SOFT_I2C_OK    (0U)

/** @brief No acknowledgment received from slave (NACK or timeout). */
#define SOFT_I2C_NACK  (1U)

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

/**
 * @brief Generate an I2C START condition.
 *
 * Drives SDA LOW while SCL remains HIGH, signalling a bus acquisition.
 * After this call SCL is left LOW, ready for the first data bit.
 *
 * @param[in] cfg  Pointer to a fully initialised SoftI2C_Config_t.
 */
void SoftI2C_Start(const SoftI2C_Config_t *cfg);

/**
 * @brief Generate an I2C STOP condition.
 *
 * Drives SDA HIGH while SCL is HIGH, releasing the bus.
 *
 * @param[in] cfg  Pointer to a fully initialised SoftI2C_Config_t.
 */
void SoftI2C_Stop(const SoftI2C_Config_t *cfg);

/**
 * @brief Send one byte (8 bits) over the I2C bus, MSB first.
 *
 * The caller must follow up with SoftI2C_WaitAck() to verify that the
 * slave acknowledged the byte.
 *
 * @param[in] cfg   Pointer to a fully initialised SoftI2C_Config_t.
 * @param[in] byte  Data byte to transmit.
 */
void SoftI2C_SendByte(const SoftI2C_Config_t *cfg, uint8_t byte);

/**
 * @brief Read one byte (8 bits) from the I2C bus, MSB first.
 *
 * After reading, the master sends either an ACK or NACK to the slave.
 *
 * @param[in] cfg  Pointer to a fully initialised SoftI2C_Config_t.
 * @param[in] ack  Set to 1 to send ACK (more bytes will follow),
 *                 or 0 to send NACK (this is the last byte).
 * @return         The byte received from the slave.
 */
uint8_t SoftI2C_ReadByte(const SoftI2C_Config_t *cfg, uint8_t ack);

/**
 * @brief Wait for and sample the slave's ACK/NACK bit.
 *
 * Must be called after every SoftI2C_SendByte() to verify delivery.
 *
 * @param[in] cfg  Pointer to a fully initialised SoftI2C_Config_t.
 * @return         SOFT_I2C_OK  (0) if the slave ACKed,
 *                 SOFT_I2C_NACK (1) if no ACK was detected.
 */
uint8_t SoftI2C_WaitAck(const SoftI2C_Config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* SOFT_I2C_H */
