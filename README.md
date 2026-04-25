# Embedded-Drivers-Generic

A modular, hardware-independent C driver library for STM32, ESP32, 51, and
other microcontrollers.

The library follows a **Hardware Abstraction Layer (HAL)** approach: every
driver is completely decoupled from MCU registers.  Hardware-specific
operations (GPIO toggling, delays, SPI/I2C primitives) are injected at
run-time through *handle structs* containing function pointers, so the same
driver source/API can be reused across platforms, with only the platform
layer needing adaptation and the project recompiled for the target MCU.

---

## Repository Layout

```
Embedded-Drivers-Generic/
├── drivers/          # Peripheral device drivers (OLED, MPU6050, …)
├── protocols/        # Communication protocol drivers
│   └── soft_i2c/     # Bit-banged (software) I2C master
│       ├── soft_i2c.h
│       └── soft_i2c.c
├── utils/            # Common utilities (RingBuffer, PID, …)
└── README.md
```

---

## Adding This Library to Your Project via Git Submodules

### 1 – Add the submodule

```bash
# Run this inside your application repository
git submodule add https://github.com/chengyouwei905-ops/Embedded-Drivers-Generic.git lib/embedded-drivers
git submodule update --init --recursive
```

### 2 – Include the submodule in your build

**CMake** example:

```cmake
add_subdirectory(lib/embedded-drivers/protocols/soft_i2c)
target_link_libraries(my_app PRIVATE soft_i2c)
```

**Makefile / bare-metal** example – add the source and include path manually:

```makefile
SRCS += lib/embedded-drivers/protocols/soft_i2c/soft_i2c.c
CFLAGS += -Ilib/embedded-drivers/protocols/soft_i2c
```

### 3 – Keep the submodule up to date

```bash
# Pull the latest commits from this library
git submodule update --remote --merge
git add lib/embedded-drivers
git commit -m "chore: update embedded-drivers submodule"
```

---

## Integrating the Software I2C Driver

### Step 1 – Implement the hardware callbacks

Create five GPIO-control/read functions and one microsecond-delay function
that match the signatures required by `SoftI2C_Config_t`.  Only these six
functions contain any MCU-specific code.

**STM32 HAL example** (`my_i2c_hw.c`):

```c
#include "stm32f4xx_hal.h"   /* or whichever HAL header your MCU needs */
#include "soft_i2c.h"

/* --- GPIO helpers -------------------------------------------------------- */
static void my_sda_high(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
}

static void my_sda_low(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
}

static void my_scl_high(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

static void my_scl_low(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
}

static uint8_t my_sda_read(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);
}

static void my_delay_us(uint32_t us)
{
    /* Simple busy-wait; replace with a timer-based delay for accuracy */
    volatile uint32_t cycles = us * (SystemCoreClock / 1000000UL) / 5UL;
    while (cycles--) { __NOP(); }
}

/* --- Config handle ------------------------------------------------------- */
const SoftI2C_Config_t g_i2c_cfg = {
    .sda_high  = my_sda_high,
    .sda_low   = my_sda_low,
    .scl_high  = my_scl_high,
    .scl_low   = my_scl_low,
    .sda_read  = my_sda_read,
    .delay_us  = my_delay_us,
};
```

**ESP32 (ESP-IDF)** or **51-MCU** implementations follow the same pattern –
only the body of each callback changes.

### Step 2 – Use the driver

```c
#include "soft_i2c.h"

extern const SoftI2C_Config_t g_i2c_cfg;  /* defined in my_i2c_hw.c */

void write_register(uint8_t dev_addr, uint8_t reg, uint8_t value)
{
    SoftI2C_Start(&g_i2c_cfg);

    SoftI2C_SendByte(&g_i2c_cfg, (dev_addr << 1U) | 0x00U); /* write */
    if (SoftI2C_WaitAck(&g_i2c_cfg) != SOFT_I2C_OK) { goto stop; }

    SoftI2C_SendByte(&g_i2c_cfg, reg);
    if (SoftI2C_WaitAck(&g_i2c_cfg) != SOFT_I2C_OK) { goto stop; }

    SoftI2C_SendByte(&g_i2c_cfg, value);
    SoftI2C_WaitAck(&g_i2c_cfg);

stop:
    SoftI2C_Stop(&g_i2c_cfg);
}

uint8_t read_register(uint8_t dev_addr, uint8_t reg)
{
    uint8_t data = 0;

    SoftI2C_Start(&g_i2c_cfg);
    SoftI2C_SendByte(&g_i2c_cfg, (dev_addr << 1U) | 0x00U); /* write */
    if (SoftI2C_WaitAck(&g_i2c_cfg) != SOFT_I2C_OK) { goto stop; }

    SoftI2C_SendByte(&g_i2c_cfg, reg);
    if (SoftI2C_WaitAck(&g_i2c_cfg) != SOFT_I2C_OK) { goto stop; }

    SoftI2C_Start(&g_i2c_cfg);  /* repeated START */
    SoftI2C_SendByte(&g_i2c_cfg, (dev_addr << 1U) | 0x01U); /* read */
    if (SoftI2C_WaitAck(&g_i2c_cfg) != SOFT_I2C_OK) { goto stop; }

    data = SoftI2C_ReadByte(&g_i2c_cfg, 0U); /* 0 = send NACK (last byte) */

stop:
    SoftI2C_Stop(&g_i2c_cfg);
    return data;
}
```

---

## API Reference – Software I2C (`protocols/soft_i2c`)

| Symbol | Description |
|---|---|
| `SoftI2C_Config_t` | Handle struct; fill its six function pointers once at startup |
| `SOFT_I2C_OK` | Return code – slave sent ACK |
| `SOFT_I2C_NACK` | Return code – slave sent NACK or did not respond |
| `SoftI2C_Start()` | Generate I2C START condition |
| `SoftI2C_Stop()` | Generate I2C STOP condition |
| `SoftI2C_SendByte()` | Transmit one byte, MSB first |
| `SoftI2C_WaitAck()` | Sample and return the slave ACK/NACK bit |
| `SoftI2C_ReadByte()` | Receive one byte and send ACK or NACK |

Full Doxygen comments are in `protocols/soft_i2c/soft_i2c.h`.

---

## Code Style

* C99 (`-std=c99`) with `<stdint.h>` types (`uint8_t`, `uint32_t`, …).
* Public headers use `#ifndef / #define / #endif` include guards.
* Public functions are documented with Doxygen-style `/** … */` comments.
* No hardware-specific code inside `/protocols` or `/drivers` – all MCU
  details live in your application's callback implementations.
