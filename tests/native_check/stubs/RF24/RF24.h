// A STAND-IN for the RF24 library's header: only what radio_nrf24.cpp uses, with the real
// library's names and types, so the hardware half of the driver can be COMPILED (not run) on a
// PC. tests/check_native.sh uses it. The real header comes from the RF24 library on the Pi.
#ifndef STUB_RF24_H
#define STUB_RF24_H
#include <cstdint>
typedef enum { RF24_PA_MIN = 0, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX, RF24_PA_ERROR } rf24_pa_dbm_e;
typedef enum { RF24_1MBPS = 0, RF24_2MBPS, RF24_250KBPS } rf24_datarate_e;
class RF24 {
public:
    RF24(uint16_t ce_pin, uint16_t csn_pin, uint32_t spi_speed = 10000000);
    bool begin();
    bool isChipConnected();
    void startListening();
    void stopListening();
    bool available();
    void read(void* buf, uint8_t len);
    bool write(const void* buf, uint8_t len);
    void startFastWrite(const void* buf, uint8_t len, const bool multicast);
    void flush_tx();
    bool txStandBy(uint32_t timeout);
    void openWritingPipe(const uint8_t* address);
    void openReadingPipe(uint8_t number, const uint8_t* address);
    void printDetails();
    void setChannel(uint8_t channel);
    uint8_t getChannel();
    void setPayloadSize(uint8_t size);
    void setPALevel(uint8_t level, bool lnaEnable = true);
    bool setDataRate(rf24_datarate_e speed);
    void setAutoAck(bool enable);
    void setRetries(uint8_t delay, uint8_t count);
    bool testRPD();
    void startConstCarrier(rf24_pa_dbm_e level, uint8_t channel);
    void stopConstCarrier();
};
#endif
