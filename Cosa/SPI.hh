/**
 * @file Cosa/SPI.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2012, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * @section Description
 * Serial Peripheral Interface (SPI) device class.
 * http://www.byteparadigm.com/kb/article/AA-00255/22/Introduction-to-SPI-and-IC-protocols.html
 * Typical usage is SPI device drivers inherit from SPI::Driver and 
 * define SPI commands and higher functions. 
 *
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef __COSA_SPI_HH__
#define __COSA_SPI_HH__

#include "Cosa/Types.h"
#include "Cosa/Bits.h"
#include "Cosa/Event.hh"

class SPI {

public:
  /**
   * Device drivers are friends and may have callback/
   * event handler for completion events.
   */
  class Driver : public Event::Handler {
    friend class SPI;
  };

  /**
   * Slave devices are friends and may have callback/
   * event handler for request events.
   */
  class Device : public Event::Handler {
    friend class SPI;
  public:
    /**
     * Interrupt service on data receive in slave mode.
     * @param[in] data received data.
     */
    virtual void on_interrupt(uint8_t data);
  };

  enum Clock {
    DIV4_CLOCK = 0x00,
    DIV16_CLOCK = 0x01,
    DIV64_CLOCK = 0x02,
    DIV128_CLOCK = 0x03,
    DIV2_2X_CLOCK = 0x04,
    DIV8_2X_CLOCK = 0x05,
    DIV32_2X_CLOCK = 0x06,
    DIV64_2X_CLOCK = 0x07,
    MASTER_CLOCK = 0x08,
    DEFAULT_CLOCK = 0x00
  };
  
  enum Direction {
    MSB_FIRST = 0, 
    LSB_FIRST = 1
  };

private:
  /**
   * Pins used for SPI interface (in port B, digital pins 10-13).
   */
  enum Pin {
    SS = 2,
    MOSI = 3,
    MISO = 4,
    SCK = 5
  };
  uint8_t m_cmd;
  uint8_t* m_buffer;
  uint8_t m_max;
  uint8_t m_put;
  uint8_t m_data;
  Device* m_dev;

public:
  /**
   * Construct serial peripheral interface for master.
   */
  SPI() : 
    m_buffer(0),
    m_max(0),
    m_put(0),
    m_data(0),
    m_dev(0)
  {
  }
  
  /**
   * Construct serial peripheral interface for slave.
   * @param[in] fn interrupt handler. 
   * @param[in] buffer with data to received data.
   * @param[in] max size of buffer.
   */
  SPI(Device* dev, void* buffer, uint8_t max) : 
    m_buffer((uint8_t*) buffer),
    m_max(max),
    m_put(0),
    m_data(0),
    m_dev(dev)
  {
    bit_clear(DDRB, SS);
    if (buffer == 0) {
      m_buffer = &m_data;
      m_max = 1;
    }
  }

  /**
   * Set data receive buffer for package receive mode.
   * @param[in] buffer pointer to buffer.
   * @param[in] max max size of data package.
   */
  void set_buf(void* buffer, uint8_t max) 
  { 
    m_buffer = (uint8_t*) buffer; 
    m_max = max; 
  }

  /**
   * Get data receive buffer for package receive mode.
   * @return buffer pointer to buffer.
   */
  void* get_buf()
  { 
    return (m_buffer);
  }

  /**
   * Get length of receive buffer.
   * @return number of bytes.
   */
  uint8_t get_length()
  { 
    return (m_put);
  }

  /**
   * Get slave device handler.
   * @return device reference.
   */
  Device* get_device()
  { 
    return (m_dev);
  }

  /**
   * Start master/slave serial send/receive block. 
   * @param[in] clock mode.
   * @param[in] mode data/clock sampling mode.
   * @param[in] direction data bit order.
   * @return true(1) if successful otherwise false(0)
   */
  bool begin(Clock clock = DEFAULT_CLOCK, 
	     uint8_t mode = 0, 
	     Direction direction = MSB_FIRST);

  /**
   * Exchange data with slave. Return value received. Slave select must be
   * done before send.
   * @param[in] data to send.
   * @return value received.
   */
  uint8_t exchange(uint8_t data)
  {
    SPDR = data;
    loop_until_bit_is_set(SPSR, SPIF);
    return (SPDR);
  }

  /**
   * Exchange package with slave. Received data from slave is stored in
   * given buffer. Slave selection is done for package.
   * @param[in] buffer with data to exchange (send/receive).
   * @param[in] count size of buffer.
   */
  void exchange(void* buffer, uint8_t count);

  /**
   * Exchange package in program memory to slave. Received data 
   * from slave is ignored. Slave selection is done for package.
   * @param[in] buffer with data in program memory.
   * @param[in] count size of buffer.
   */
  void exchange_P(const void* buffer, uint8_t count);

  /**
   * Read data from slave device; send address/command and return
   * received data.
   * @param[in] cmd command.
   * @return received value.
   */
  uint8_t read(uint8_t cmd)
  {
    exchange(cmd);
    return (exchange(0));
  }

  /**
   * Read data from slave device; send address/command and store 
   * received data into buffer.
   * @param[in] cmd command.
   * @param[in] buffer for received data.
   * @param[in] count size of buffer.
   * @return status.
   */
  uint8_t read(uint8_t cmd, void* buffer, uint8_t count);

  /**
   * Write data to slave device; send address/command and send data.
   * @param[in] cmd command.
   * @param[in] data to write.
   * @return status.
   */
  uint8_t write(uint8_t cmd, uint8_t data)
  {
    uint8_t status = exchange(cmd);
    exchange(data);
    return (status);
  }

  /**
   * Write data to slave device; send address/command and send data
   * from buffer.
   * @param[in] cmd command.
   * @param[in] buffer with data to send.
   * @param[in] count size of buffer.
   * @return status.
   */
  uint8_t write(uint8_t cmd, const void* buffer, uint8_t count);
  
  /**
   * Write data to slave device; send address/command and send data 
   * from buffer in program memory.
   * @param[in] cmd command.
   * @param[in] buffer with data to send.
   * @param[in] count size of buffer.
   * @return status.
   */
  uint8_t write_P(uint8_t cmd, const void* buffer, uint8_t count);
  
  /**
   * End of master/slave interaction.
   * @return true(1) if successful otherwise false(0)
   */
  bool end();
};

/**
 * Some syntactic sugar to capture the slave selection block
 */
#define SPI_transaction(ss) for (uint8_t i = (ss.clear(), 1); i != 0; i--, ss.set())

/**
 * Singleton instance of the hardware SPI module
 */
extern SPI spi;

#endif
