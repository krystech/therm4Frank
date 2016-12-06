#include "ds1307.h"

bool checkAddreses(const uint8_t start_address, const uint8_t allocated_bytes) {
    if ((allocated_bytes <= 0) || (start_address < 0) || (start_address + allocated_bytes > RAM_SIZE)) {
        return false;
    }
    return true;
}

uint8_t* readTime(void) {
    uint8_t *received_data = calloc(TIME_STRUCT_SIZE, sizeof(uint8_t));

    received_data = read(0x00, TIME_STRUCT_SIZE, false);

    return received_data;
}

void decodeTime(const uint8_t *data, time *s_time) {
    // decode seconds
    uint8_t msd = 0, lsd = 0;
    uint8_t /*am_pm = -1,*/_12h_mode = -1;

    lsd = (data[0] & 0b00001111);
    msd = (data[0] & 0b01110000) >> 4;
    s_time->seconds = lsd + 10 * msd;

    lsd = (data[1] & 0b00001111);
    msd = (data[1] & 0b01110000) >> 4;
    s_time->minutes = lsd + 10 * msd;

    // If 1, then 12-hour mode is enabled, 0 - 24-hour mode
    _12h_mode = (data[2] & 0b01000000) >> 6;

    // When 12-hour mode enabled, PM = 1, AM = 0, otherwise first bit of
    // hour_msd
    if (_12h_mode) {
        //am_pm = (data[2] & 0b00100000) >> 5;
        msd = (data[2] & 0b00010000) >> 4;
    } else {
        msd = (data[2] & 0b00110000) >> 4;
    }
    lsd = (data[2] & 0b00001111);
    s_time->hours = lsd + 10 * msd;

    s_time->day_of_week = (data[3] & 0b00000111);

    lsd = (data[4] & 0b00001111);
    msd = (data[4] & 0b00110000) >> 4;
    s_time->date = lsd + 10 * msd;

    lsd = (data[5] & 0b00001111);
    msd = (data[5] & 0b00010000) >> 4;
    s_time->month = lsd + 10 * msd;

    lsd = (data[6] & 0b00001111);
    msd = (data[6] & 0b11110000) >> 4;
    s_time->year = lsd + 10 * msd;

    s_time->clock_halt = (data[0] & 0b10000000) >> 7;
    s_time->out = (data[7] & 0b10000000) >> 7;
    s_time->sqwe = (data[7] & 0b00010000) >> 4;
    s_time->rs1 = (data[7] & 0b00000010) >> 1;
    s_time->rs0 = (data[7] & 0b00000001);
}

// Each number is represented in BCD format, according to documentation
uint8_t* encodeData(const time *s_time) {
    uint8_t *data = calloc(TIME_STRUCT_SIZE, sizeof(uint8_t));
    uint8_t msd, lsd;

    // 0x00 Clock halt and seconds
    msd = s_time->seconds / 10;
    lsd = s_time->seconds - msd * 10;
    data[0] = (s_time->clock_halt << 7) | (msd << 4) | (lsd);

    // 0x01 Minutes
    msd = s_time->minutes / 10;
    lsd = s_time->minutes - msd * 10;
    data[1] = (msd << 4) | (lsd);

    // 0x02 Hours
    msd = s_time->hours / 10;
    lsd = s_time->hours - msd * 10;
    data[2] = (0 << 6 /*24h mode*/) | (msd << 4) | (lsd);

    // 0x03 Day of week
    data[3] = s_time->day_of_week;

    // 0x04 Date (day of month)
    msd = s_time->date / 10;
    lsd = s_time->date - msd * 10;
    data[4] = (msd << 4) | (lsd);

    // 0x05 Month
    msd = s_time->month / 10;
    lsd = s_time->month - msd * 10;
    data[5] = (msd << 4) | (lsd);

    // 0x06 Year
    msd = s_time->year / 10;
    lsd = s_time->year - msd * 10;
    data[6] = (msd << 4) | (lsd);

    // 0x07 Control part:
    // OUT, SQWE, RS1 and RS0
    data[7] = (s_time->out << 7) | (s_time->sqwe << 4) | (s_time->rs1 << 1)
            | (s_time->rs0);

    return data;
}

void writeTime(const time *s_time) {
    uint8_t *data = encodeData(s_time);

    write(0x00, TIME_STRUCT_SIZE, false, data);

    free(data);
}

time getTime(void) {
    uint8_t* data = readTime();
    time s_time;
    decodeTime(data, &s_time);
    free(data);
    return s_time;
}



uint8_t* read(const uint8_t start_address, const uint8_t allocated_bytes, const bool ram) {
    if (!checkAddreses(start_address, allocated_bytes)) {
        return 0;
    }

    uint8_t *received_data = calloc(allocated_bytes, sizeof(uint8_t));

    uint8_t index = start_address;

    I2C_start(I2C1, SLAVE_ADDRESS << 1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode
    if (ram) {
        I2C_write(I2C1, start_address + TIME_STRUCT_SIZE); // set pointer to read (with offset)
    } else {
        I2C_write(I2C1, start_address); //
    }
    I2C_stop(I2C1); // stop the transmission
    I2C_start(I2C1, SLAVE_ADDRESS << 1, I2C_Direction_Receiver); // start a transmission in Master receiver mode

    for (index = 0; index < allocated_bytes; ++index) {
        if (index + 1 < allocated_bytes) {
            received_data[index] = I2C_read_ack(I2C1); // read one byte, send ack and request another byte
        } else {
            received_data[index] = I2C_read_nack(I2C1); // read one byte and don't request another byte, stop transmission
        }
    }

    return received_data;
}

void write(const uint8_t start_address, const uint8_t bytes, const bool ram, uint8_t* data) {
    if (!checkAddreses(start_address, bytes)) {
        return;
    }

    I2C_start(I2C1, SLAVE_ADDRESS << 1, I2C_Direction_Transmitter); // start a transmission in Master transmitter mode

    if (ram) {
        I2C_write(I2C1, start_address + TIME_STRUCT_SIZE); // set pointer to read (with offset)
    } else {
        I2C_write(I2C1, start_address); //
    }

    uint8_t index = 0;
    for (index = 0; index <= bytes; index += 1) {
        I2C_write(I2C1, data[index]); // write one byte to the slave
    }
    I2C_stop(I2C1); // stop the transmission
}
