
#include "stm32f10x.h"
#include "dev_i2c.h"

#define DEBUG(X) TRACEL(TRACE_LEVEL_I2C, X)

LIST_HEAD(i2c_list);

static struct i2c_adapter m_i2c_adapter[] = {
    [DEV_I2C1] = {I2C1, GPIOB, GPIO_Pin_7, GPIO_Pin_6},
    [DEV_I2C2] = {I2C2, GPIOB, GPIO_Pin_11, GPIO_Pin_10},
};


static inline struct i2c_client * i2c_find(I2C_TypeDef * spi_ch)
{
	if (!list_empty(&i2c_list)) {
		struct i2c_client * i2c_it = NULL;
		list_for_each_entry(i2c_it, &i2c_list, list) {
			if (i2c_it->adapter->i2c == spi_ch) {
				/* found */
				return(i2c_it);
			}
		}
	}
	return NULL;
}

/**
 * @brief  Init i2c device
 * @param[in] dev The i2c device (see: enum en_i2c_dev)
 * @param[in] address In case the i2c is slave then pass the address
 * @param[in] speed_hz The i2c speed in Hz (e.g. 100000 for 100KHz)
 * @param[in] callback The callback function that will be used for interrupts
 * @param[out] i2c Pointer to the i2c client object which will be filled here
 * @return int 0 for success
 */
int i2c_init(enum en_i2c_dev dev, uint16_t address, uint32_t speed_hz,
                    i2c_cb_t callback, struct i2c_client * i2c)
{

    // test();
    // return 0;

    if (dev >= DEV_EOL) {
        return -1;
    }
	RCC_PCLK1Config(RCC_HCLK_Div4);    /* Clock APB1 */
    
	list_add(&i2c->list, &i2c_list);

    i2c->adapter = &m_i2c_adapter[dev];
    i2c->address = address;
    i2c->info.dev = dev;
    i2c->info.speed_hz = speed_hz;
    i2c->cbk = callback;

    I2C_SoftwareResetCmd(i2c->adapter->i2c, ENABLE);
	I2C_SoftwareResetCmd(i2c->adapter->i2c, DISABLE);

    /* Set clock power */
    if (i2c->adapter->i2c == I2C1) {
        I2C_Cmd(I2C1, DISABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    }
    else if (i2c->adapter->i2c == I2C1) {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    }
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /* set up GPIOs */
    GPIO_InitTypeDef  gpio;
    gpio.GPIO_Pin = i2c->adapter->pin_sda | i2c->adapter->pin_scl;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(i2c->adapter->port, &gpio);

    /* set up interrupts */
    NVIC_InitTypeDef nvic;
    if (i2c->adapter->i2c == I2C1)
        nvic.NVIC_IRQChannel = I2C1_EV_IRQn;
    else if (i2c->adapter->i2c == I2C2)
        nvic.NVIC_IRQChannel = I2C2_EV_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    /* error interrupt */
    if (i2c->adapter->i2c == I2C1)
        nvic.NVIC_IRQChannel = I2C1_ER_IRQn;
    else if (i2c->adapter->i2c == I2C2)
        nvic.NVIC_IRQChannel = I2C2_ER_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
    
    I2C_SoftwareResetCmd(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, DISABLE);

    /* I2C configuration */
    I2C_InitTypeDef * conf = &i2c->info.config;
    I2C_StructInit(conf);
    conf->I2C_Mode = I2C_Mode_I2C;
    conf->I2C_DutyCycle = I2C_DutyCycle_2;
    conf->I2C_OwnAddress1 = (i2c->address << 1);
    conf->I2C_Ack = I2C_Ack_Enable;
    conf->I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    conf->I2C_ClockSpeed = i2c->info.speed_hz;
    I2C_Init(i2c->adapter->i2c, conf);

    I2C_ITConfig(i2c->adapter->i2c, I2C_IT_EVT | I2C_IT_ERR | I2C_IT_BUF, ENABLE); //Part of the STM32 I2C driver
    // I2C_ITConfig(i2c->adapter->i2c, I2C_IT_BUF, ENABLE);
    /* I2C Peripheral Enable */
    I2C_Cmd(i2c->adapter->i2c, ENABLE);
    i2c->info.enabled = 1;

    DEBUG(("Setting up I2C with address: 0x%02X and speed: %lu\n", i2c->address, i2c->info.speed_hz));

    return 0;
}

/**
 * @brief  Enable the i2c device
 * @param[in] i2c Pointer to the i2c client object
 */
void i2c_enable(struct i2c_client * i2c)
{
    I2C_Cmd(i2c->adapter->i2c, ENABLE);
    i2c->info.enabled = 1;
}

/**
 * @brief  Disable the i2c device
 * @param[in] i2c Pointer to the i2c client object
 */
void i2c_disable(struct i2c_client * i2c)
{
    I2C_Cmd(i2c->adapter->i2c, DISABLE);
    i2c->info.enabled = 0;
}

/**
 * @brief  Master send data to address and wait until transaction is complete
 * @param[in] i2c Pointer to the i2c client object
 * @param[in] address The address of the I2C slave
 * @param[in] buff The buffer to send
 * @param[in] len The buffer's length in bytes
 * @return int The lenght of the bytes that were sent
*/
int i2c_master_write_wait(struct i2c_client * i2c, uint8_t address, uint8_t * buff, uint16_t len)
{
    int resp = i2c_master_write(i2c, address, buff, len);
    if (resp) {
        while(i2c->master_data.busy);
    }
    return(resp);
}

/**
 * @brief  Master send data to address async (using interrupts)
 * @param[in] i2c Pointer to the i2c client object
 * @param[in] address The address of the I2C slave
 * @param[in] buff The buffer to send
 * @param[in] len The buffer's length in bytes
 * @return int The lenght of the bytes that were sent
*/
int i2c_master_write(struct i2c_client * i2c, uint8_t address, uint8_t * buff, uint16_t len)
{
    if (!len || !buff || !address || i2c->master_data.busy) return -1;

    i2c->master_data.address = address;
    i2c->master_data.buffer = buff;
    i2c->master_data.buffer_len = len;
    i2c->master_data.direction = I2C_Direction_Transmitter;
    i2c->master_data.index = 0;
    /* This will start the send procedure */
    I2C_GenerateSTART(i2c->adapter->i2c, ENABLE);
    i2c->master_data.busy = 1;
    return len;
}

/**
 * @brief  Master read from address blocking
 * @param[in] i2c Pointer to the i2c client object
 * @param[in] address The address of the slave device
 * @param[in] buff The buffer to use for received data
 * @param[in] len The buffer's length in bytes
 * @return int The number of bytes were read
 */ 
int i2c_master_read_wait(struct i2c_client * i2c, uint8_t address, uint8_t * buff, uint16_t len)
{
    if (!i2c_master_read(i2c, address, buff, len)) {
        while(i2c->master_data.busy);
    }
    return(i2c->master_data.index);
}

/**
 * @brief  Master read from address async (using interrupts)
 * @param[in] i2c Pointer to the i2c client object
 * @param[in] address The address of the slave device
 * @param[in] buff The buffer to use for received data
 * @param[in] len The buffer's length in bytes
 * @return int 0 for success, else -1
 */ 
int i2c_master_read(struct i2c_client * i2c, uint8_t address, uint8_t * buff, uint16_t len)
{
    if (!len || !buff || !address || i2c->master_data.busy) return -1;

    i2c->master_data.address = address;
    i2c->master_data.buffer = buff;
    i2c->master_data.buffer_len = len;
    i2c->master_data.direction = I2C_Direction_Receiver;
    i2c->master_data.index = 0;
    /* This will start the send procedure */
    I2C_GenerateSTART(i2c->adapter->i2c, ENABLE);
    i2c->master_data.busy = 1;
    return 0;
}

void I2Cx_ClearFlag(struct i2c_client * i2c) {
    // ADDR-Flag clear
    I2C_TypeDef * dev = i2c->adapter->i2c;
    while((dev->SR1 & I2C_SR1_ADDR) == I2C_SR1_ADDR) {
        dev->SR1;
        dev->SR2;
    }

    // STOPF Flag clear
    while((dev->SR1 & I2C_SR1_STOPF) == I2C_SR1_STOPF) {
        dev->SR1;
        dev->CR1 |= 0x1;
    }
}

static void inline i2c_handle_int(struct i2c_client * i2c)
{
    uint32_t event;
    uint8_t byte;

    I2C_TypeDef * dev = i2c->adapter->i2c;

    // if (i2c->adapter->i2c == I2C1)
    //     PORT_STATUS_LED->ODR ^= PIN_STATUS_LED;

    // Reading last event
    event = I2C_GetLastEvent(dev);

    //i2c->cbk(i2c, event, &byte);

    switch(event) {
        /* master events */
        case I2C_EVENT_MASTER_MODE_SELECT:
            DEBUG(("1\n"));
            // I2C_AcknowledgeConfig(i2c->adapter->i2c, ENABLE);
            // I2C_NACKPositionConfig(i2c->adapter->i2c, I2C_NACKPosition_Current);
            I2C_Send7bitAddress(i2c->adapter->i2c, i2c->master_data.address << 1,
                    i2c->master_data.direction);
            break;
        case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:
            DEBUG(("2\n"));
            // I2C_SendData(i2c->adapter->i2c, i2c->master_data.buffer[i2c->master_data.index++]);
            break;
        case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:
            DEBUG(("3\n"));
            // i2c->master_data.buffer[i2c->master_data.index++] = I2C_ReceiveData(i2c->adapter->i2c);
            break;
        case I2C_EVENT_MASTER_BYTE_RECEIVED:
            if (i2c->master_data.index >= i2c->master_data.buffer_len) {
                if (i2c->master_data.address) {
                    DEBUG(("4b\n"));
                    // I2C_AcknowledgeConfig(i2c->adapter->i2c, DISABLE);
                    I2C_ReceiveData(i2c->adapter->i2c);
                    I2C_GenerateSTOP(i2c->adapter->i2c, ENABLE);
                    i2c->master_data.address = 0;
                }
                else {
                    DEBUG(("4c\n"));
                    i2c->cbk(i2c, I2C_MASTER_READ, i2c->master_data.buffer, i2c->master_data.index);
                    I2C_ReceiveData(i2c->adapter->i2c);
                    i2c->master_data.busy = 0;
                }
            }
            else {
                uint8_t byte = I2C_ReceiveData(i2c->adapter->i2c);
                i2c->master_data.buffer[i2c->master_data.index++] = byte;
                DEBUG(("4a: %c\n", byte));
            }
            break;
        // case I2C_EVENT_MASTER_BYTE_TRANSMITTING:
        //     // do nothing
        //     break;
        case I2C_EVENT_MASTER_BYTE_TRANSMITTED:
            if (i2c->master_data.index < i2c->master_data.buffer_len) {
                DEBUG(("6\n"));
                I2C_SendData(i2c->adapter->i2c, i2c->master_data.buffer[i2c->master_data.index++]);
            }
            else {
                /* When finished then set address to 0 */
                if (i2c->master_data.address) {
                    DEBUG(("7\n"));
                    I2C_GenerateSTOP(i2c->adapter->i2c, ENABLE);
                    i2c->master_data.address = 0;
                }
                else
                    i2c->master_data.busy = 0;
            }
            break;

        /* slave events */
        case I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED:
            // Master has sent the slave address to send data to the slave
            i2c->cbk(i2c, I2C_SLAVE_ADDRESSED, NULL, 0);
            break;
        case I2C_EVENT_SLAVE_BYTE_RECEIVED:
            // Master has sent a byte to the slave
            byte = I2C_ReceiveData(dev);
            i2c->cbk(i2c, I2C_SLAVE_WRITE_REQUESTED, &byte, 1);
            break;
        case I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED:
            byte = 0xAA;
            byte = i2c->cbk(i2c, I2C_SLAVE_READ_REQUESTED, &byte, 1);
            I2C_SendData(dev, byte);
            break;
        case I2C_EVENT_SLAVE_BYTE_TRANSMITTED:
            byte = 0xBB;
            byte = i2c->cbk(i2c, I2C_SLAVE_READ_REQUESTED, &byte, 1);
            I2C_SendData(dev, byte);
            break;
        case I2C_EVENT_SLAVE_STOP_DETECTED:
            // Master has STOP sent
            I2Cx_ClearFlag(i2c);
            break;
    };

}

void I2C1_EV_IRQHandler(void)
{
    struct i2c_client * i2c = i2c_find(I2C1);
    if (!i2c)
        return;
    else
        i2c_handle_int(i2c);
}

void I2C2_EV_IRQHandler(void)
{
    struct i2c_client * i2c = i2c_find(I2C2);
    if (!i2c)
        return;
    else
        i2c_handle_int(i2c);
}

void I2C1_ER_IRQHandler(void) {
    if (I2C_GetITStatus(I2C1, I2C_IT_AF)) {
        I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
    }
}

void I2C2_ER_IRQHandler(void) {
    if (I2C_GetITStatus(I2C2, I2C_IT_AF)) {
        I2C_ClearITPendingBit(I2C2, I2C_IT_AF);
    }
}

/**
 * Example of interrupt callback routine in main:
 * 
 
 uint8_t i2c_interrupt(struct i2c_client * i2c, enum i2c_slave_event event, uint8_t * byte)
{
	uint8_t resp = 0;
	PORT_STATUS_LED->ODR ^= PIN_STATUS_LED;
	// TRACE(("%d: 0x%02X", event, *byte));
	switch (event) {
	case I2C_SLAVE_ADDRESSED:
		TRACE(("1 - %d: 0x%02X\n", event, *byte));
		break;
	case I2C_SLAVE_READ_REQUESTED:
		resp = 0xAA; //i2c_counter++;
		TRACE(("2 - %d: 0x%02X\n", event, *byte));
		break;
	case I2C_SLAVE_WRITE_REQUESTED:
		TRACE(("3 - %d: 0x%02X\n", event, *byte));
		resp = 0xBB;
		break;
	};
	return resp;
}

*/