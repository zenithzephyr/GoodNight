// RH_RF22.cpp
//
// Copyright (C) 2011 Mike McCauley
// $Id: RH_RF22.cpp,v 1.27 2017/01/12 23:58:00 mikem Exp $

#include <asf.h>

#include <string.h>
#include <math.h>
#include <RH_RF22.h>

#include "gpio_spi.h"

// Interrupt vectors for the 2 Arduino interrupt pins
// Each interrupt can be handled by a different instance of RH_RF22, allowing you to have
// 2 RH_RF22s per Arduino
uint8_t             _idleMode;
uint8_t             _deviceType;
CRCPolynomial       _polynomial;
volatile uint8_t    _bufLen;
uint8_t             _buf[RH_RF22_MAX_MESSAGE_LEN];
volatile uint8_t       _rxBufValid;
volatile uint8_t    _txBufSentIndex;
uint32_t            _lastPreambleTime;
volatile RHMode     _mode;
uint8_t             _thisAddress;
uint8_t             _promiscuous;
volatile uint8_t    _rxHeaderTo;
volatile uint8_t    _rxHeaderFrom;
volatile uint8_t    _rxHeaderId;
volatile uint8_t    _rxHeaderFlags;
uint8_t             _txHeaderTo;
uint8_t             _txHeaderFrom;
uint8_t             _txHeaderId;
uint8_t             _txHeaderFlags;
volatile int8_t     _lastRssi;
volatile uint16_t   _rxBad;
volatile uint16_t   _rxGood;
volatile uint16_t   _txGood;

// These are indexed by the values of ModemConfigChoice
// Canned modem configurations generated with
// http://www.hoperf.com/upload/rf/RH_RF22B%2023B%2031B%2042B%2043B%20Register%20Settings_RevB1-v5.xls
// Stored in flash (program) memory to save SRAM
static const ModemConfig MODEM_CONFIG_TABLE[] =
{
    { 0x2b, 0x03, 0xf4, 0x20, 0x41, 0x89, 0x00, 0x36, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x10, 0x62, 0x2c, 0x00, 0x08 }, // Unmodulated carrier
    { 0x2b, 0x03, 0xf4, 0x20, 0x41, 0x89, 0x00, 0x36, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x10, 0x62, 0x2c, 0x33, 0x08 }, // FSK, PN9 random modulation, 2, 5

    // All the following enable FIFO with reg 71
    //  1c,   1f,   20,   21,   22,   23,   24,   25,   2c,   2d,   2e,   58,   69,   6e,   6f,   70,   71,   72
    // FSK, No Manchester, Max Rb err <1%, Xtal Tol 20ppm
    { 0x2b, 0x03, 0xf4, 0x20, 0x41, 0x89, 0x00, 0x36, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x10, 0x62, 0x2c, 0x22, 0x08 }, // 2, 5
    { 0x1b, 0x03, 0x41, 0x60, 0x27, 0x52, 0x00, 0x07, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x13, 0xa9, 0x2c, 0x22, 0x3a }, // 2.4, 36
    { 0x1d, 0x03, 0xa1, 0x20, 0x4e, 0xa5, 0x00, 0x13, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x27, 0x52, 0x2c, 0x22, 0x48 }, // 4.8, 45
    { 0x1e, 0x03, 0xd0, 0x00, 0x9d, 0x49, 0x00, 0x45, 0x40, 0x0a, 0x20, 0x80, 0x60, 0x4e, 0xa5, 0x2c, 0x22, 0x48 }, // 9.6, 45
    { 0xac, 0x03, 0x9c, 0x00, 0xd1, 0xb7, 0x00, 0xa3, 0x40, 0x0a, 0x1b, 0x80, 0x60, 0x9d, 0x49, 0x2c, 0x22, 0x50 }, // 19.2, 50
    //{ 0x2b, 0x03, 0x34, 0x02, 0x75, 0x25, 0x07, 0xff, 0x40, 0x0a, 0x1b, 0x80, 0x60, 0x9d, 0x49, 0x2c, 0x22, 0x0f }, // 19.2, 9.6
    { 0x02, 0x03, 0x68, 0x01, 0x3a, 0x93, 0x04, 0xd5, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x09, 0xd5, 0x0c, 0x22, 0x1f }, // 38.4, 19.6
    { 0x06, 0x03, 0x45, 0x01, 0xd7, 0xdc, 0x07, 0x6e, 0x40, 0x0a, 0x2d, 0x80, 0x60, 0x0e, 0xbf, 0x0c, 0x22, 0x2e }, // 57.6. 28.8
    { 0x8a, 0x03, 0x60, 0x01, 0x55, 0x55, 0x02, 0xad, 0x40, 0x0a, 0x50, 0x80, 0x60, 0x20, 0x00, 0x0c, 0x22, 0xc8 }, // 125, 125

    { 0x2b, 0x03, 0xa1, 0xe0, 0x10, 0xc7, 0x00, 0x09, 0x40, 0x0a, 0x1d,  0x80, 0x60, 0x04, 0x32, 0x2c, 0x22, 0x04 }, // 512 baud, FSK, 2.5 Khz fd for POCSAG compatibility
    { 0x27, 0x03, 0xa1, 0xe0, 0x10, 0xc7, 0x00, 0x06, 0x40, 0x0a, 0x1d,  0x80, 0x60, 0x04, 0x32, 0x2c, 0x22, 0x07 }, // 512 baud, FSK, 4.5 Khz fd for POCSAG compatibility

    // GFSK, No Manchester, Max Rb err <1%, Xtal Tol 20ppm
    // These differ from FSK only in register 71, for the modulation type
    { 0x2b, 0x03, 0xf4, 0x20, 0x41, 0x89, 0x00, 0x36, 0x40, 0x0a, 0x1d, 0x80, 0x60, 0x10, 0x62, 0x2c, 0x23, 0x08 }, // 2, 5
    { 0x1b, 0x03, 0x41, 0x60, 0x27, 0x52, 0x00, 0x07, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x13, 0xa9, 0x2c, 0x23, 0x3a }, // 2.4, 36
    { 0x1d, 0x03, 0xa1, 0x20, 0x4e, 0xa5, 0x00, 0x13, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x27, 0x52, 0x2c, 0x23, 0x48 }, // 4.8, 45
    { 0x1e, 0x03, 0xd0, 0x00, 0x9d, 0x49, 0x00, 0x45, 0x40, 0x0a, 0x20, 0x80, 0x60, 0x4e, 0xa5, 0x2c, 0x23, 0x48 }, // 9.6, 45
    { 0x2b, 0x03, 0x34, 0x02, 0x75, 0x25, 0x07, 0xff, 0x40, 0x0a, 0x1b, 0x80, 0x60, 0x9d, 0x49, 0x2c, 0x23, 0x0f }, // 19.2, 9.6
    { 0x02, 0x03, 0x68, 0x01, 0x3a, 0x93, 0x04, 0xd5, 0x40, 0x0a, 0x1e, 0x80, 0x60, 0x09, 0xd5, 0x0c, 0x23, 0x1f }, // 38.4, 19.6
    { 0x06, 0x03, 0x45, 0x01, 0xd7, 0xdc, 0x07, 0x6e, 0x40, 0x0a, 0x2d, 0x80, 0x60, 0x0e, 0xbf, 0x0c, 0x23, 0x2e }, // 57.6. 28.8
    { 0x8a, 0x03, 0x60, 0x01, 0x55, 0x55, 0x02, 0xad, 0x40, 0x0a, 0x50, 0x80, 0x60, 0x20, 0x00, 0x0c, 0x23, 0xc8 }, // 125, 125

    // OOK, No Manchester, Max Rb err <1%, Xtal Tol 20ppm
    { 0x51, 0x03, 0x68, 0x00, 0x3a, 0x93, 0x01, 0x3d, 0x2c, 0x11, 0x28, 0x80, 0x60, 0x09, 0xd5, 0x2c, 0x21, 0x08 }, // 1.2, 75
    { 0xc8, 0x03, 0x39, 0x20, 0x68, 0xdc, 0x00, 0x6b, 0x2a, 0x08, 0x2a, 0x80, 0x60, 0x13, 0xa9, 0x2c, 0x21, 0x08 }, // 2.4, 335
    { 0xc8, 0x03, 0x9c, 0x00, 0xd1, 0xb7, 0x00, 0xd4, 0x29, 0x04, 0x29, 0x80, 0x60, 0x27, 0x52, 0x2c, 0x21, 0x08 }, // 4.8, 335
    { 0xb8, 0x03, 0x9c, 0x00, 0xd1, 0xb7, 0x00, 0xd4, 0x28, 0x82, 0x29, 0x80, 0x60, 0x4e, 0xa5, 0x2c, 0x21, 0x08 }, // 9.6, 335
    { 0xa8, 0x03, 0x9c, 0x00, 0xd1, 0xb7, 0x00, 0xd4, 0x28, 0x41, 0x29, 0x80, 0x60, 0x9d, 0x49, 0x2c, 0x21, 0x08 }, // 19.2, 335
    { 0x98, 0x03, 0x9c, 0x00, 0xd1, 0xb7, 0x00, 0xd4, 0x28, 0x20, 0x29, 0x80, 0x60, 0x09, 0xd5, 0x0c, 0x21, 0x08 }, // 38.4, 335
    { 0x98, 0x03, 0x96, 0x00, 0xda, 0x74, 0x00, 0xdc, 0x28, 0x1f, 0x29, 0x80, 0x60, 0x0a, 0x3d, 0x0c, 0x21, 0x08 }, // 40, 335
};


void si4432_setIdleMode(uint8_t idleMode)
{
    _idleMode = idleMode;
}

uint8_t si4432_init()
{
    _idleMode = RH_RF22_XTON; // Default idle state is READY mode
    _polynomial = CRC_16_IBM; // Historical

    // Software reset the device
    si4432_reset();

    _deviceType = SPIReadByte(RH_RF22_REG_00_DEVICE_TYPE);
    if (   _deviceType != RH_RF22_DEVICE_TYPE_RX_TRX
        && _deviceType != RH_RF22_DEVICE_TYPE_TX)
    {
      return false;
    }

    // Enable interrupt output on the radio. Interrupt line will now go high until
    // an interrupt occurs
    SPIWriteByte(RH_RF22_REG_05_INTERRUPT_ENABLE1, RH_RF22_ENTXFFAEM | RH_RF22_ENRXFFAFULL | RH_RF22_ENPKSENT | RH_RF22_ENPKVALID | RH_RF22_ENCRCERROR | RH_RF22_ENFFERR);
    SPIWriteByte(RH_RF22_REG_06_INTERRUPT_ENABLE2, RH_RF22_ENPREAVAL);

    si4432_setModeIdle();

    si4432_clearTxBuf();
    si4432_clearRxBuf();

    // Most of these are the POR default
    SPIWriteByte(RH_RF22_REG_7D_TX_FIFO_CONTROL2, RH_RF22_TXFFAEM_THRESHOLD);
    SPIWriteByte(RH_RF22_REG_7E_RX_FIFO_CONTROL,  RH_RF22_RXFFAFULL_THRESHOLD);
    SPIWriteByte(RH_RF22_REG_30_DATA_ACCESS_CONTROL, RH_RF22_ENPACRX /*| RH_RF22_ENPACTX | RH_RF22_ENCRC | (_polynomial & RH_RF22_CRC)*/);
    SPIWriteByte(RH_RF22_REG_32_HEADER_CONTROL1, RH_RF22_BCEN_NONE);
    SPIWriteByte(RH_RF22_REG_33_HEADER_CONTROL2, RH_RF22_HDLEN_0 | RH_RF22_SYNCLEN_4);

    si4432_setPreambleLength(6);
    uint8_t syncwords[] = { 0x01, 0x01, 0x01, 0x01};
    si4432_setSyncWords(syncwords, sizeof(syncwords));
    si4432_setPromiscuous(false);

    si4432_setFrequency(434.0, 0.05);
    si4432_setModemConfig(FSK_Rb19_2Fd9_6); //19.2 50kHz
    si4432_setGpioReversed(false);
    si4432_setTxPower(RH_RF22_TXPOW_8DBM);

    return true;
}

void si4432_handleInterrupt()
{
    uint8_t _lastInterruptFlags[2];
    // Read the interrupt flags which clears the interrupt
    SPIBurstReadByte(RH_RF22_REG_03_INTERRUPT_STATUS1, _lastInterruptFlags, 2);

#if 0
    // DEVELOPER TESTING ONLY
    // Caution: Serial printing in this interrupt routine can cause mysterious crashes
    printf("interrupt %x %x\r\n",_lastInterruptFlags[0], _lastInterruptFlags[1]);
    if (_lastInterruptFlags[0] == 0 && _lastInterruptFlags[1] == 0)
	   printf("FUNNY: no interrupt!");
#endif

#if 0
    // DEVELOPER TESTING ONLY
    // TESTING: fake an RH_RF22_IFFERROR
    static int counter = 0;
    if (_lastInterruptFlags[0] & RH_RF22_IPKSENT && counter++ == 10)
    {
	_lastInterruptFlags[0] = RH_RF22_IFFERROR;
	counter = 0;
    }
#endif

    if (_lastInterruptFlags[0] & RH_RF22_IFFERROR)
    {
      si4432_resetFifos(); // Clears the interrupt
      if (_mode == RHModeTx)
      si4432_restartTransmit();
      else if (_mode == RHModeRx)
      si4432_clearRxBuf();
      //Serial.println("IFFERROR");
    }
    // Caution, any delay here may cause a FF underflow or overflow
    if (_lastInterruptFlags[0] & RH_RF22_ITXFFAEM)
    {
      // See if more data has to be loaded into the Tx FIFO
      si4432_sendNextFragment();
      //Serial.println("ITXFFAEM");
    }
    if (_lastInterruptFlags[0] & RH_RF22_IRXFFAFULL)
    {
      // Caution, any delay here may cause a FF overflow
      // Read some data from the Rx FIFO
      si4432_readNextFragment();
      //Serial.println("IRXFFAFULL");
    }
    if (_lastInterruptFlags[0] & RH_RF22_IEXT)
    {
      // This is not enabled by the base code, but users may want to enable it
      si4432_handleExternalInterrupt();
      //Serial.println("IEXT");
    }
    if (_lastInterruptFlags[1] & RH_RF22_IWUT)
    {
      // This is not enabled by the base code, but users may want to enable it
      si4432_handleWakeupTimerInterrupt();
      //Serial.println("IWUT");
    }
    if (_lastInterruptFlags[0] & RH_RF22_IPKSENT)
    {
      //Serial.println("IPKSENT");
      _txGood++;
      // Transmission does not automatically clear the tx buffer.
      // Could retransmit if we wanted
      // RH_RF22 transitions automatically to Idle
      _mode = RHModeIdle;
    }
    if (_lastInterruptFlags[0] & RH_RF22_IPKVALID)
    {
      uint8_t len = SPIReadByte(RH_RF22_REG_4B_RECEIVED_PACKET_LENGTH);
      //Serial.println("IPKVALID");

      // May have already read one or more fragments
      // Get any remaining unread octets, based on the expected length
      // First make sure we dont overflow the buffer in the case of a stupid length
      // or partial bad receives
      if (   len >  RH_RF22_MAX_MESSAGE_LEN
        || len < _bufLen)
        {
          _rxBad++;
          _mode = RHModeIdle;
          si4432_clearRxBuf();
          return; // Hmmm receiver buffer overflow.
        }

        SPIBurstReadByte(RH_RF22_REG_7F_FIFO_ACCESS, _buf + _bufLen, len - _bufLen);
        _rxHeaderTo = SPIReadByte(RH_RF22_REG_47_RECEIVED_HEADER3);
        _rxHeaderFrom = SPIReadByte(RH_RF22_REG_48_RECEIVED_HEADER2);
        _rxHeaderId = SPIReadByte(RH_RF22_REG_49_RECEIVED_HEADER1);
        _rxHeaderFlags = SPIReadByte(RH_RF22_REG_4A_RECEIVED_HEADER0);
        _rxGood++;
        _bufLen = len;
        _mode = RHModeIdle;
        _rxBufValid = true;
      }
      if (_lastInterruptFlags[0] & RH_RF22_ICRCERROR)
      {
        //Serial.println("ICRCERR");
        _rxBad++;
        si4432_clearRxBuf();
        si4432_resetRxFifo();
        _mode = RHModeIdle;
	si4432_setModeRx(); // Keep trying
    }
    if (_lastInterruptFlags[1] & RH_RF22_IPREAVAL)
    {
	//Serial.println("IPREAVAL");
	_lastRssi = (int8_t)(-120 + ((SPIReadByte(RH_RF22_REG_26_RSSI) / 2)));
	//_lastPreambleTime = si4432_millis();
	si4432_resetRxFifo();
	si4432_clearRxBuf();
    }
}

void si4432_reset()
{
    SPIWriteByte(RH_RF22_REG_07_OPERATING_MODE1, RH_RF22_SWRES);
    // Wait for it to settle
    delay_ms(1); // SWReset time is nominally 100usec
}

uint8_t si4432_statusRead()
{
    return SPIReadByte(RH_RF22_REG_02_DEVICE_STATUS);
}

uint8_t si4432_adcRead(uint8_t adcsel,
                      uint8_t adcref ,
                      uint8_t adcgain,
                      uint8_t adcoffs)
{
    uint8_t configuration = adcsel | adcref | (adcgain & RH_RF22_ADCGAIN);
    SPIWriteByte(RH_RF22_REG_0F_ADC_CONFIGURATION, configuration | RH_RF22_ADCSTART);
    SPIWriteByte(RH_RF22_REG_10_ADC_SENSOR_AMP_OFFSET, adcoffs);

    // Conversion time is nominally 305usec
    // Wait for the DONE bit
    while (!(SPIReadByte(RH_RF22_REG_0F_ADC_CONFIGURATION) & RH_RF22_ADCDONE))
	;
    // Return the value
    return SPIReadByte(RH_RF22_REG_11_ADC_VALUE);
}

uint8_t si4432_temperatureRead(uint8_t tsrange, uint8_t tvoffs)
{
    SPIWriteByte(RH_RF22_REG_12_TEMPERATURE_SENSOR_CALIBRATION, tsrange | RH_RF22_ENTSOFFS);
    SPIWriteByte(RH_RF22_REG_13_TEMPERATURE_VALUE_OFFSET, tvoffs);
    return si4432_adcRead(RH_RF22_ADCSEL_INTERNAL_TEMPERATURE_SENSOR, RH_RF22_ADCREF_BANDGAP_VOLTAGE, 0, 0);
}

uint16_t si4432_wutRead()
{
    uint8_t buf[2];
    SPIBurstReadByte(RH_RF22_REG_17_WAKEUP_TIMER_VALUE1, buf, 2);
    return ((uint16_t)buf[0] << 8) | buf[1]; // Dont rely on byte order
}

// RFM-22 doc appears to be wrong: WUT for wtm = 10000, r, = 0, d = 0 is about 1 sec
void si4432_setWutPeriod(uint16_t wtm, uint8_t wtr, uint8_t wtd)
{
    uint8_t period[3];

    period[0] = ((wtr & 0xf) << 2) | (wtd & 0x3);
    period[1] = wtm >> 8;
    period[2] = wtm & 0xff;
    SPIBurstWriteByte(RH_RF22_REG_14_WAKEUP_TIMER_PERIOD1, period, sizeof(period));
}

// Returns true if centre + (fhch * fhs) is within limits
// Caution, different versions of the RH_RF22 support different max freq
// so YMMV
uint8_t si4432_setFrequency(float centre, float afcPullInRange)
{
    uint8_t fbsel = RH_RF22_SBSEL;
    uint8_t afclimiter;
    if (centre < 240.0 || centre > 960.0) // 930.0 for early silicon
	return false;
    if (centre >= 480.0)
    {
	if (afcPullInRange < 0.0 || afcPullInRange > 0.318750)
	    return false;
	centre /= 2;
	fbsel |= RH_RF22_HBSEL;
	afclimiter = afcPullInRange * 1000000.0 / 1250.0;
    }
    else
    {
	if (afcPullInRange < 0.0 || afcPullInRange > 0.159375)
	    return false;
	afclimiter = afcPullInRange * 1000000.0 / 625.0;
    }
    centre /= 10.0;
    float integerPart = floor(centre);
    float fractionalPart = centre - integerPart;

    uint8_t fb = (uint8_t)integerPart - 24; // Range 0 to 23
    fbsel |= fb;
    uint16_t fc = fractionalPart * 64000;
    SPIWriteByte(RH_RF22_REG_73_FREQUENCY_OFFSET1, 0);  // REVISIT
    SPIWriteByte(RH_RF22_REG_74_FREQUENCY_OFFSET2, 0);
    SPIWriteByte(RH_RF22_REG_75_FREQUENCY_BAND_SELECT, fbsel);
    SPIWriteByte(RH_RF22_REG_76_NOMINAL_CARRIER_FREQUENCY1, fc >> 8);
    SPIWriteByte(RH_RF22_REG_77_NOMINAL_CARRIER_FREQUENCY0, fc & 0xff);
    SPIWriteByte(RH_RF22_REG_2A_AFC_LIMITER, afclimiter);
    return !(si4432_statusRead() & RH_RF22_FREQERR);
}

// Step size in 10kHz increments
// Returns true if centre + (fhch * fhs) is within limits
uint8_t si4432_setFHStepSize(uint8_t fhs)
{
    SPIWriteByte(RH_RF22_REG_7A_FREQUENCY_HOPPING_STEP_SIZE, fhs);
    return !(si4432_statusRead() & RH_RF22_FREQERR);
}

// Adds fhch * fhs to centre frequency
// Returns true if centre + (fhch * fhs) is within limits
uint8_t si4432_setFHChannel(uint8_t fhch)
{
    SPIWriteByte(RH_RF22_REG_79_FREQUENCY_HOPPING_CHANNEL_SELECT, fhch);
    return !(si4432_statusRead() & RH_RF22_FREQERR);
}

uint8_t si4432_rssiRead()
{
    return SPIReadByte(RH_RF22_REG_26_RSSI);
}

uint8_t si4432_ezmacStatusRead()
{
    return SPIReadByte(RH_RF22_REG_31_EZMAC_STATUS);
}

void si4432_setOpMode(uint8_t mode)
{
    SPIWriteByte(RH_RF22_REG_07_OPERATING_MODE1, mode);
}

void si4432_setModeIdle()
{
    if (_mode != RHModeIdle)
    {
	si4432_setOpMode(_idleMode);
	_mode = RHModeIdle;
    }
}

uint8_t si4432_sleep()
{
    if (_mode != RHModeSleep)
    {
	si4432_setOpMode(0);
	_mode = RHModeSleep;
    }
    return true;
}

void si4432_setModeRx()
{
    if (_mode != RHModeRx)
    {
	si4432_setOpMode(_idleMode | RH_RF22_RXON);
	_mode = RHModeRx;
    }
}

void si4432_setModeTx()
{
    if (_mode != RHModeTx)
    {
	si4432_setOpMode(_idleMode | RH_RF22_TXON);
	// Hmmm, if you dont clear the RX FIFO here, then it appears that going
	// to transmit mode in the middle of a receive can corrupt the
	// RX FIFO
	si4432_resetRxFifo();
	_mode = RHModeTx;
    }
}

void si4432_setTxPower(uint8_t power)
{
    SPIWriteByte(RH_RF22_REG_6D_TX_POWER, power | RH_RF22_LNA_SW); // On RF23, LNA_SW must be set.
}

// Sets registers from a canned modem configuration structure
void si4432_setModemRegisters(const ModemConfig* config)
{
    SPIWriteByte(RH_RF22_REG_1C_IF_FILTER_BANDWIDTH,                    config->reg_1c);
    SPIWriteByte(RH_RF22_REG_1F_CLOCK_RECOVERY_GEARSHIFT_OVERRIDE,      config->reg_1f);
    SPIBurstWriteByte(RH_RF22_REG_20_CLOCK_RECOVERY_OVERSAMPLING_RATE, &config->reg_20, 6);
    SPIBurstWriteByte(RH_RF22_REG_2C_OOK_COUNTER_VALUE_1,              &config->reg_2c, 3);
    SPIWriteByte(RH_RF22_REG_58_CHARGE_PUMP_CURRENT_TRIMMING,           config->reg_58);
    SPIWriteByte(RH_RF22_REG_69_AGC_OVERRIDE1,                          config->reg_69);
    SPIBurstWriteByte(RH_RF22_REG_6E_TX_DATA_RATE1,                    &config->reg_6e, 5);
}

// Set one of the canned FSK Modem configs
// Returns true if its a valid choice
uint8_t si4432_setModemConfig(ModemConfigChoice index)
{
    if (index > (signed int)(sizeof(MODEM_CONFIG_TABLE) / sizeof(ModemConfig)))
        return false;

    ModemConfig cfg;
    memcpy(&cfg, &MODEM_CONFIG_TABLE[index], sizeof(ModemConfig));
    si4432_setModemRegisters(&cfg);

    return true;
}

// REVISIT: top bit is in Header Control 2 0x33
void si4432_setPreambleLength(uint8_t nibbles)
{
    SPIWriteByte(RH_RF22_REG_34_PREAMBLE_LENGTH, nibbles);
}

// Caution doesnt set sync word len in Header Control 2 0x33
void si4432_setSyncWords(const uint8_t* syncWords, uint8_t len)
{
    SPIBurstWriteByte(RH_RF22_REG_36_SYNC_WORD3, syncWords, len);
}

void si4432_clearRxBuf()
{
//    ATOMIC_BLOCK_START;
    _bufLen = 0;
    _rxBufValid = false;
//    ATOMIC_BLOCK_END;
}

uint8_t si4432_available()
{
    if (!_rxBufValid)
    {
	if (_mode == RHModeTx)
	    return false;
	si4432_setModeRx(); // Make sure we are receiving
    }
    return _rxBufValid;
}

uint8_t si4432_recv(uint8_t* buf, uint8_t* len)
{
    if (!si4432_available())
	return false;

    if (buf && len)
    {
	//ATOMIC_BLOCK_START;
	if (*len > _bufLen)
	    *len = _bufLen;
	memcpy(buf, _buf, *len);
	//ATOMIC_BLOCK_END;
    }
    si4432_clearRxBuf();
//    printBuffer("recv:", buf, *len);
    return true;
}

void si4432_clearTxBuf()
{
    //ATOMIC_BLOCK_START;
    _bufLen = 0;
    _txBufSentIndex = 0;
    //ATOMIC_BLOCK_END;
}

void si4432_startTransmit()
{
    si4432_sendNextFragment(); // Actually the first fragment
    SPIWriteByte(RH_RF22_REG_3E_PACKET_LENGTH, _bufLen); // Total length that will be sent
    si4432_setModeTx(); // Start the transmitter, turns off the receiver
}

// Restart the transmission of a packet that had a problem
void si4432_restartTransmit()
{
    _mode = RHModeIdle;
    _txBufSentIndex = 0;
//	    Serial.println("Restart");
    si4432_startTransmit();
}

uint8_t si4432_waitPacketSent()
{
    while (_mode == RHModeTx);
	//YIELD; // Wait for any previous transmit to finish
    return true;
}

uint8_t si4432_send(const uint8_t* data, uint8_t len)
{
    uint8_t ret = true;
    si4432_waitPacketSent();

   // if (!si4432_waitCAD())
	//return false;  // Check channel activity

    //ATOMIC_BLOCK_START;
    SPIWriteByte(RH_RF22_REG_3A_TRANSMIT_HEADER3, _txHeaderTo);
    SPIWriteByte(RH_RF22_REG_3B_TRANSMIT_HEADER2, _txHeaderFrom);
    SPIWriteByte(RH_RF22_REG_3C_TRANSMIT_HEADER1, _txHeaderId);
    SPIWriteByte(RH_RF22_REG_3D_TRANSMIT_HEADER0, _txHeaderFlags);
    if (!si4432_fillTxBuf(data, len))
	ret = false;
    else
	si4432_startTransmit();
    //ATOMIC_BLOCK_END;
//    printBuffer("send:", data, len);
    return ret;
}

uint8_t si4432_fillTxBuf(const uint8_t* data, uint8_t len)
{
  si4432_clearTxBuf();
  if (!len)
  return false;
  return si4432_appendTxBuf(data, len);
}

uint8_t si4432_appendTxBuf(const uint8_t* data, uint8_t len)
{
  if (((uint16_t)_bufLen + len) > RH_RF22_MAX_MESSAGE_LEN)
  return false;
  //ATOMIC_BLOCK_START;
  memcpy(_buf + _bufLen, data, len);
  _bufLen += len;
  //ATOMIC_BLOCK_END;
  //    printBuffer("txbuf:", _buf, _bufLen);
  return true;
}

// Assumption: there is currently <= RH_RF22_TXFFAEM_THRESHOLD bytes in the Tx FIFO
void si4432_sendNextFragment()
{
  if (_txBufSentIndex < _bufLen) {
    // Some left to send?
    uint8_t len = _bufLen - _txBufSentIndex;
    // But dont send too much
    if (len > (RH_RF22_FIFO_SIZE - RH_RF22_TXFFAEM_THRESHOLD - 1))
    len = (RH_RF22_FIFO_SIZE - RH_RF22_TXFFAEM_THRESHOLD - 1);
    SPIBurstWriteByte(RH_RF22_REG_7F_FIFO_ACCESS, _buf + _txBufSentIndex, len);
    //	printBuffer("frag:", _buf  + _txBufSentIndex, len);
    _txBufSentIndex += len;
  }
}

// Assumption: there are at least RH_RF22_RXFFAFULL_THRESHOLD in the RX FIFO
// That means it should only be called after a RXFFAFULL interrupt
void si4432_readNextFragment()
{
  if (((uint16_t)_bufLen + RH_RF22_RXFFAFULL_THRESHOLD) > RH_RF22_MAX_MESSAGE_LEN)
  return; // Hmmm receiver overflow. Should never occur

  // Read the RH_RF22_RXFFAFULL_THRESHOLD octets that should be there
  SPIBurstReadByte(RH_RF22_REG_7F_FIFO_ACCESS, _buf + _bufLen, RH_RF22_RXFFAFULL_THRESHOLD); //FIXME
  _bufLen += RH_RF22_RXFFAFULL_THRESHOLD;
}

// Clear the FIFOs
void si4432_resetFifos()
{
    SPIWriteByte(RH_RF22_REG_08_OPERATING_MODE2, RH_RF22_FFCLRRX | RH_RF22_FFCLRTX);
    SPIWriteByte(RH_RF22_REG_08_OPERATING_MODE2, 0);
}

// Clear the Rx FIFO
void si4432_resetRxFifo()
{
    SPIWriteByte(RH_RF22_REG_08_OPERATING_MODE2, RH_RF22_FFCLRRX);
    SPIWriteByte(RH_RF22_REG_08_OPERATING_MODE2, 0);
    _rxBufValid = false;
}

// CLear the TX FIFO
void si4432_resetTxFifo()
{
    SPIWriteByte(RH_RF22_REG_08_OPERATING_MODE2, RH_RF22_FFCLRTX);
    SPIWriteByte(RH_RF22_REG_08_OPERATING_MODE2, 0);
}

// Default implmentation does nothing. Override if you wish
void si4432_handleExternalInterrupt()
{
}

// Default implmentation does nothing. Override if you wish
void si4432_handleWakeupTimerInterrupt()
{
}

void si4432_setPromiscuous(uint8_t promiscuous)
{
    //RHSPIDriver::setPromiscuous(promiscuous);
    SPIWriteByte(RH_RF22_REG_43_HEADER_ENABLE3, promiscuous ? 0x00 : 0xff);
}

uint8_t si4432_setCRCPolynomial(CRCPolynomial polynomial)
{
  if (polynomial >= CRC_CCITT && polynomial <= CRC_Biacheva) {
    _polynomial = polynomial;
    return true;
  } else {
    return false;
  }
}

uint8_t si4432_maxMessageLength()
{
    return RH_RF22_MAX_MESSAGE_LEN;
}

void si4432_setThisAddress(uint8_t thisAddress)
{
    //RHSPIDriver::setThisAddress(thisAddress);
    SPIWriteByte(RH_RF22_REG_3F_CHECK_HEADER3, thisAddress);
}

uint32_t si4432_getLastPreambleTime()
{
    return _lastPreambleTime;
}

void si4432_setGpioReversed(uint8_t gpioReversed)
{
  if (gpioReversed) {
    SPIWriteByte(RH_RF22_REG_0B_GPIO_CONFIGURATION0, 0x15) ; // RX state
    SPIWriteByte(RH_RF22_REG_0C_GPIO_CONFIGURATION1, 0x12) ; // TX state
  } else {
    SPIWriteByte(RH_RF22_REG_0B_GPIO_CONFIGURATION0, 0x12) ; // TX state
    SPIWriteByte(RH_RF22_REG_0C_GPIO_CONFIGURATION1, 0x15) ; // RX state
  }
}
