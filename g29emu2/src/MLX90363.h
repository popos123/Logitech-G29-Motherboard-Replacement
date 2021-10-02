#ifdef Software
#include <SoftSPI.h>
#else
#include <SPI.h>
#endif

class Magnet {

  private:
    #ifdef Software
    SoftSPI *mySPI = NULL;
    #else
    SPIClass *hspi = NULL;
    #endif
    char pin_SS1;
    char u8_spi_read_buffer[8]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    char u8_spi_write_buffer[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    float f32_angle_degrees = 0.0;
    unsigned int u16_angle_lsb = 0;
    char u8_error_lsb = 0, u8_rollcnt_dec = 0, u8_virtualgain_dec = 0, u8_crc_dec = 0;
    // 360 degrees is mapped to 14 bits = 360/2^14 = 0.02197
    const float f32_lsb_to_dec_degrees = 0.02197;

  public:
    void begin(char pin_MOSI, char pin_MISO, char pin_SLCK, char _pin_SS1) {
      pinMode(pin_MOSI, OUTPUT);
      pinMode(pin_MISO, INPUT);
      pinMode(pin_SLCK, OUTPUT);
      this->pin_SS1 = _pin_SS1;
      pinMode(pin_SS1, OUTPUT);
      digitalWrite(pin_SS1, HIGH); // Turn off the SS
      #ifdef Software
      this->mySPI = new SoftSPI(pin_MOSI, pin_MISO, pin_SLCK);
      this->mySPI->begin();
      #else
      this->hspi = new SPIClass();
      this->hspi->begin(pin_SS1);
      #endif
    }

    void ReadData() {
      delayMicroseconds(110); // just for STM32, for leonardo don't needed (50 - 100us)
      // Issue GET1 message
      this->u8_spi_write_buffer[0] = 0x00;
      this->u8_spi_write_buffer[1] = 0x00;
      this->u8_spi_write_buffer[2] = 0xFF;
      this->u8_spi_write_buffer[3] = 0xFF;
      this->u8_spi_write_buffer[4] = 0x00;
      this->u8_spi_write_buffer[5] = 0x00;
      this->u8_spi_write_buffer[6] = 0x13;
      this->u8_spi_write_buffer[7] = 0xEA;
      #ifndef Software
      this->hspi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE1));
      #endif
      digitalWrite(this->pin_SS1, LOW); // Select
      for (int i = 0; i < 8; i++) {
        #ifdef Software
        this->u8_spi_read_buffer[i] = mySPI->transfer(this->u8_spi_write_buffer[i]);
        #else
        this->u8_spi_read_buffer[i] = hspi->transfer(this->u8_spi_write_buffer[i]);
        #endif
      }
      digitalWrite(this->pin_SS1, HIGH); // Deselect
      #ifndef Software
      this->hspi->endTransaction();
      #endif

      delayMicroseconds(800); // about 700uS is the minimum, but 800 is safe value
      // Issue NOP message
      this->u8_spi_write_buffer[0] = 0x00;
      this->u8_spi_write_buffer[1] = 0x00;
      this->u8_spi_write_buffer[2] = 0xAA;
      this->u8_spi_write_buffer[3] = 0xAA;
      this->u8_spi_write_buffer[4] = 0x00;
      this->u8_spi_write_buffer[5] = 0x00;
      this->u8_spi_write_buffer[6] = 0xD0;
      this->u8_spi_write_buffer[7] = 0xAB;

      #ifndef Software
      this->hspi->beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE1));
      #endif
      digitalWrite(this->pin_SS1, LOW); // Select
      for (int i = 0; i < 8; i++) {
        #ifdef Software
        this->u8_spi_read_buffer[i] = mySPI->transfer(this->u8_spi_write_buffer[i]);
        #else
        this->u8_spi_read_buffer[i] = hspi->transfer(this->u8_spi_write_buffer[i]);
        #endif
      }
      digitalWrite(this->pin_SS1, HIGH); // Deselect
      #ifndef Software
      this->hspi->endTransaction();
      #endif
    }

    int AngleLSB() {
      // Extract and convert the angle to degrees
      // remove error bits and shift to high byte
      this->u16_angle_lsb = (this->u8_spi_read_buffer[1] & 0x3F) << 8;
      // Add LSB of angle
      this->u16_angle_lsb = u16_angle_lsb + this->u8_spi_read_buffer[0];
      return this->u16_angle_lsb;
    }

    float AngleDEC() {
      AngleLSB(); // get data to convert
      // Convert to decimal degrees
      this->f32_angle_degrees = this->u16_angle_lsb * this->f32_lsb_to_dec_degrees;
      return this->f32_angle_degrees;
    }

    int ErrorBits() {
      this->u8_error_lsb = this->u8_spi_read_buffer[1] >> 6;
      return this->u8_error_lsb;
    }

    int crc() {
      this->u8_crc_dec = this->u8_spi_read_buffer[7];
      return this->u8_crc_dec;
    }

    int VirtualGain() {
      this->u8_virtualgain_dec = this->u8_spi_read_buffer[4];
      return this->u8_virtualgain_dec;
    }

    int RollingCounter() {
      this->u8_rollcnt_dec = this->u8_spi_read_buffer[6] & 0x3F;
      return this->u8_rollcnt_dec;
    }
};
