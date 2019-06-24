/*
 * dev_i2c.h
 *
 * Copyright 2018 Dimitris Tassopoulos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 */

#ifndef __DEV_I2C_H_
#define __DEV_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include "stm32f10x.h"
#include "debug_trace.h"
#include "list.h"

enum en_i2c_dev {
    DEV_I2C1 = 0,
    DEV_I2C2,

    DEV_EOL
};

/* event for both master and slave */
enum i2c_event {
    /* master */

    /* slave */
    I2C_SLAVE_ADDRESSED,
	I2C_SLAVE_READ_REQUESTED,
	I2C_SLAVE_WRITE_REQUESTED,
    I2C_MASTER_READ,
};

struct i2c_client;

typedef uint8_t (*i2c_cb_t)(struct i2c_client *, enum i2c_event, uint8_t *, uint16_t);

struct i2c_adapter {
    I2C_TypeDef *       i2c;
    GPIO_TypeDef *      port;       /* SPI output port */
    uint16_t            pin_sda;    /* SPI SDA pin */
    uint16_t            pin_scl;    /* SPI SCL pin */
};

struct i2c_data {
    uint8_t    busy;
    uint8_t    address;
    uint8_t    direction;
    uint8_t    *buffer;
    uint16_t   buffer_len;
    uint16_t   index;
};

struct i2c_info {
    enum en_i2c_dev     dev;
    uint32_t            speed_hz;
    uint8_t             enabled;
    I2C_InitTypeDef     config;
};

struct i2c_client {
    uint16_t                address;
    struct i2c_info         info;
    struct i2c_adapter *    adapter;
    i2c_cb_t                cbk;    // callback
	struct list_head list;
    volatile struct i2c_data master_data;
    // int                     irq;
};

int i2c_init(enum en_i2c_dev dev, uint16_t address, uint32_t speed_hz,
                    i2c_cb_t callback, struct i2c_client * i2c);
void i2c_enable(struct i2c_client * i2c);
void i2c_disable(struct i2c_client * i2c);
int i2c_master_write(struct i2c_client * i2c, uint8_t address, uint8_t * buff, uint16_t len);
int i2c_master_write_wait(struct i2c_client * i2c, uint8_t address, uint8_t * buff, uint16_t len);
int i2c_master_read(struct i2c_client * i2c, uint8_t address, uint8_t * buff, uint16_t len);
int i2c_master_read_wait(struct i2c_client * i2c, uint8_t address, uint8_t * buff, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif //__DEV_I2C_SLAVE_H_