#include <Wire.h>

// ================ Power IC IP5306 ===================
// IP5306 is a fully-integrated multi-function power management SoC
// Documentatio:
// http://www.injoinic.com/wwwroot/uploads/files/20200221/0405f23c247a34d3990ae100c8b20a27.pdf

// !!!!!!   ATTENTION  !!!!!!
// IP5306 cannot be switched on automatically when powered only by a battery - no VIN
// the button should be used to turn it on

#define IP5306_ADDR 0x75

// Registers
#define IP5306_SYS_CTL0 0x00
#define IP5306_SYS_CTL1 0x01
#define IP5306_SYS_CTL2 0x02
#define IP5306_CHARGER_CTL0 0x20
#define IP5306_CHARGER_CTL1 0x21
#define IP5306_CHARGER_CTL2 0x22
#define IP5306_DIG_CTL0 0x24
#define IP5306_REG_READ0 0x70
#define IP5306_REG_READ1 0x71
#define IP5306_REG_READ2 0x72
#define IP5306_REG_READ3 0x77

//- REG_CTL0
#define BOOST_ENABLE_BIT 0x20
#define CHARGE_OUT_BIT 0x10
#define BOOT_ON_LOAD_BIT 0x04
#define BOOST_OUT_BIT 0x02
#define BOOST_BUTTON_EN_BIT 0x01

//- REG_CTL1
#define BOOST_SET_BIT 0x80
#define WLED_SET_BIT 0x40
#define SHORT_BOOST_BIT 0x20
#define VIN_ENABLE_BIT 0x04

//- REG_CTL2
#define SHUTDOWNTIME_MASK 0x0c
#define SHUTDOWNTIME_64S 0x0c
#define SHUTDOWNTIME_32S 0x04
#define SHUTDOWNTIME_16S 0x08
#define SHUTDOWNTIME_8S 0x00

//- REG_READ0
#define CHARGE_ENABLE_BIT 0x08

//- REG_READ1
#define CHARGE_FULL_BIT 0x08

//- REG_READ2
#define LIGHT_LOAD_BIT 0x20
#define LOWPOWER_SHUTDOWN_BIT 0x01

//! Error Code
#define IP5306_PASS 0
#define IP5306_FAIL -1
#define IP5306_INVALID -2
#define IP5306_NOT_INIT -3

uint8_t read_byte(uint8_t _reg)
{
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(_reg);
    Wire.endTransmission();
    Wire.requestFrom(IP5306_ADDR, 1);
    return Wire.read();
}

bool get_charge_enable_state()
{
    return read_byte(IP5306_REG_READ0) & CHARGE_ENABLE_BIT;
}

bool get_charge_full_state()
{
    return read_byte(IP5306_REG_READ1) & CHARGE_FULL_BIT;
}

uint8_t get_sysctl0()
{
    return read_byte(IP5306_SYS_CTL0);
}

uint8_t get_sysctl1()
{
    return read_byte(IP5306_SYS_CTL1);
}

uint8_t get_sysctl2()
{
    return read_byte(IP5306_SYS_CTL2);
}

bool set_ip5306()
{
    bool res = Wire.begin();
    int ctl0 = 0b00110110;
    int ctl1 = 0b00000101;
    int ctl2 = 0b00000100;

    // ctl0 = 55;  // 0b00110111
    // ctl1 = 29;  // 0b00011101
    // ctl2 = 100; // 0b01100100

    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_SYS_CTL0);
    Wire.write(ctl0);
    res = Wire.endTransmission() == 0;
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_SYS_CTL1);
    Wire.write(ctl1);
    res &= Wire.endTransmission() == 0;
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_SYS_CTL2);
    Wire.write(ctl2);
    res &= Wire.endTransmission() == 0;
    // SYS_CTL0:
    // 7:6 - reserved - 00
    // 5 - boost enable - 0 - automatic shutdown under light load
    // 4 - charger enable - 1 - stop charging when fully charged
    // 3 - reserved - 0
    // 2 - auto power on enable, when load is plugged - 1
    // 1 - boost output normally open - 1
    // 0 - enable button shutdown - 0
    // default 0b00110101 was set

    // SYS_CTL1:
    // 7 - turn off boost signal selection - 0
    // 6 - Switch WLED flashlight control signal selection - 0
    // 5 - Short press the switch boost - 0
    // 4:3 - reserved - 00
    // 2 - After VIN is unplugged, whether to turn on Boost - 0
    // 1 - reserved
    // 0 - Batlow 3.0V low power shutdown enable - 1
    // default 0b00011101 was set

    // SYS_CTL2:
    // 7:5 - reserved - 000
    // 4 - KEY long press time setting - 0 (2s)
    // 3:2 - Light load shutdown time setting - 01 (32s)
    // 0:1 - reserved - 00

    return res;
}