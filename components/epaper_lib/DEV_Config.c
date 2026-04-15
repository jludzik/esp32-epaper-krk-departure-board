/*****************************************************************************
* | File      	:   DEV_Config.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*----------------
* |	This version:   V3.0
* | Date        :   2019-07-31
* | Info        :   
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of theex Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/

/*
	Modified and adapted for ESP32 (ESP-IDF) by https://github.com/jludzik
    Date: 21.03.2026
*/

#include "DEV_Config.h"

static spi_device_handle_t SPI_Handle;

void DEV_SPI_WriteByte(UBYTE Value)
{
    spi_transaction_t t; 
    memset(&t, 0, sizeof(t)); 
    t.length = 8;      
    t.flags = SPI_TRANS_USE_TXDATA; 
    t.tx_data[0] = Value;
    spi_device_polling_transmit(SPI_Handle, &t);
}

/******************************************************************************
function:	Module Initialize, the library and initialize the pins, SPI protocol
parameter:
Info:
******************************************************************************/
UBYTE DEV_Module_Init(void)
{
    Debug("START SPI INIT");

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<EPD_DC_PIN) | (1ULL<<EPD_RST_PIN);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<EPD_BUSY_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; 
    gpio_config(&io_conf);

    DEV_Digital_Write(EPD_RST_PIN,1);

    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = EPD_MOSI_PIN,
        .sclk_io_num = EPD_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 65536
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000,  // 20 MHz
        .mode = 0,                           // SPI mode 0 (CPOL=0, CPHA=0)
        .spics_io_num = EPD_CS_PIN,
        .queue_size = 7,
    };

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if(ret != ESP_OK)
    {
        Debug("ESP32 SPI bus initialization failed!\n");
        return 1; 
    }

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &SPI_Handle);
    if(ret != ESP_OK)
    {
        Debug("ESP32 SPI device addition failed!\n");
        return 1;
    }

    Debug("SPI INIT DONE");
	return 0;
}

/******************************************************************************
function:	Module exits, closes SPI and BCM2835 library
parameter:
Info:
******************************************************************************/
void DEV_Module_Exit(void)
{
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_RST_PIN, 0);

    if (SPI_Handle != NULL) {
        spi_bus_remove_device(SPI_Handle);
        spi_bus_free(SPI2_HOST);
        SPI_Handle = NULL;
    }
    gpio_reset_pin(EPD_CS_PIN);
    gpio_reset_pin(EPD_DC_PIN);
    gpio_reset_pin(EPD_RST_PIN);
    gpio_reset_pin(EPD_BUSY_PIN);
}