
#include <Wire.h>

// Hardware I2C using PB9 (SDA), PB8 (SCL)
TwoWire rtcWire(PB8, PB9);


//#include <SoftWire.h>

//SoftWire rtcWire(PB8, PB9, SOFT_STANDARD);

#define MCP7940N_ADDRESS 0x6F
#define MFP_PIN PA1

// UART pins
#define UART_TX_PIN PA2
#define UART_RX_PIN PA3

uint8_t seconds, minutes, hours, day, date, month, year;//
char dateTimeStr[64];


bool mcp7940n_write_register(uint8_t reg, uint8_t value) {
  rtcWire.beginTransmission(MCP7940N_ADDRESS);
  rtcWire.write(reg);
  rtcWire.write(value);
  return rtcWire.endTransmission() == 0;
}

bool mcp7940n_read_register(uint8_t reg, uint8_t *value) {
  rtcWire.beginTransmission(MCP7940N_ADDRESS);
  rtcWire.write(reg);
  if (rtcWire.endTransmission(false) != 0) return false;
  if (rtcWire.requestFrom(MCP7940N_ADDRESS, 1) != 1) return false;
  *value = rtcWire.read();
  return true;
}

bool mcp7940n_read_time() {
  uint8_t value;
  if (!mcp7940n_read_register(0x00, &value)) return false;
  seconds = (value & 0x0F) + ((value & 0x70) >> 4) * 10;

  if (!mcp7940n_read_register(0x01, &value)) return false;
  minutes = (value & 0x0F) + ((value & 0x70) >> 4) * 10;

  if (!mcp7940n_read_register(0x02, &value)) return false;
  hours = (value & 0x0F) + ((value & 0x30) >> 4) * 10;

  if (!mcp7940n_read_register(0x03, &value)) return false;
  day = value & 0x07;

  if (!mcp7940n_read_register(0x04, &value)) return false;
  date = (value & 0x0F) + ((value & 0x30) >> 4) * 10;

  if (!mcp7940n_read_register(0x05, &value)) return false;
  month = (value & 0x0F) + ((value & 0x10) >> 4) * 10;

  if (!mcp7940n_read_register(0x06, &value)) return false;
  year = (value & 0x0F) + ((value & 0xF0) >> 4) * 10;

  return true;
}

bool mcp7940n_set_time(uint8_t y, uint8_t m, uint8_t d, uint8_t h, uint8_t min, uint8_t sec) {
  uint8_t sec_bcd = ((sec / 10) << 4) | (sec % 10);
  uint8_t min_bcd = ((min / 10) << 4) | (min % 10);
  uint8_t hour_bcd = ((h / 10) << 4) | (h % 10);
  uint8_t date_bcd = ((d / 10) << 4) | (d % 10);
  uint8_t month_bcd = ((m / 10) << 4) | (m % 10);
  uint8_t year_bcd = ((y / 10) << 4) | (y % 10);
  uint8_t wkday = 1; // Monday

  sec_bcd |= 0x80; // Start oscillator

  return
    mcp7940n_write_register(0x00, sec_bcd) &&
    mcp7940n_write_register(0x01, min_bcd) &&
    mcp7940n_write_register(0x02, hour_bcd) &&
    mcp7940n_write_register(0x03, wkday | 0x08) &&
    mcp7940n_write_register(0x04, date_bcd) &&
    mcp7940n_write_register(0x05, month_bcd) &&
    mcp7940n_write_register(0x06, year_bcd);
}

bool mcp7940n_init() {
  //pinMode(MFP_PIN, INPUT);
  //rtcWire.begin();
  uint8_t dummy;
  return mcp7940n_read_register(0x03, &dummy);
}

void setup() {
  Serial.begin(115200);
    Serial2.begin(9600);
  delay(1000);
  Serial2.println("Starting hardware I2C RTC");
//rtcWire.setClock(100000);
delay(500);
  rtcWire.begin();
Serial2.println("L1");
  if (mcp7940n_init()) {
    Serial.println("RTC OK");
    Serial2.println("RTC OK");
    if (mcp7940n_set_time(23, 4, 9, 14, 30, 0)) {
      Serial.println("Time set");
    }
  } else {
    Serial.println("RTC FAIL");
  }
}

void loop() {
  if (mcp7940n_read_time()) {
    sprintf(dateTimeStr, "20%02d/%02d/%02d %02d:%02d:%02d",
            year, month, date, hours, minutes, seconds);
    Serial.println(dateTimeStr);
    Serial2.print("RTC: ");
    Serial2.println(dateTimeStr);
  } else {
    Serial.println("RTC read error");
    Serial2.println("RTC read error");
  }
  Serial2.println("UART heartbeat");
  delay(1000);
}
