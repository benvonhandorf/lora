#include <heltec.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

/*
 * BMP280 - Temperature and Pressure - 0x76
 * AHT20 - Temperatuer and Humidity - 0x38
 * INA219 - Volage and current - 0x40
 */

// Heltec module changes the I2C library pins after display initialization, so define our own
TwoWire i2c = TwoWire(1);

Adafruit_INA219 ina219;
Adafruit_AHTX0 aht20;

Adafruit_BMP280 bmp280(&i2c);
Adafruit_Sensor *bmp280_temp = bmp280.getTemperatureSensor();
Adafruit_Sensor *bmp280_pressure = bmp280.getPressureSensor();

#define LORA_FREQUENCY_HZ 915.5E6
#define LORA_DESTINATION 0x01
#define LORA_NODE 0x02
#define LORA_MESSAGE_ID 0x01
#define LORA_FLAGS 0x00
#define LORA_BANDWIDTH 125000
#define LORA_CODING 5
#define LORA_SF 7
#define LORA_TX_POWER_DBM 10

#define MEASUREMENT_FREQUENCY_MS 10000

#define DISPLAY_MEASUREMENTS 0

bool displayOn = false;

void sendLoRaFloat(float value) {
  LoRa.write(*(((uint8_t *) &value) + 0));
  LoRa.write(*(((uint8_t *) &value) + 1));
  LoRa.write(*(((uint8_t *) &value) + 2));
  LoRa.write(*(((uint8_t *) &value) + 3));
}

void sendLoRaLong(long value) {
  LoRa.write(*(((uint8_t *) &value) + 0));
  LoRa.write(*(((uint8_t *) &value) + 1));
  LoRa.write(*(((uint8_t *) &value) + 2));
  LoRa.write(*(((uint8_t *) &value) + 3));
}

void sendLoRaMessage(float voltage_V, float current_mA, float humidity_PCT, float temp_C, float pressure_hPa, float temp_2_C) {
  
  LoRa.begin(LORA_FREQUENCY_HZ, true);

  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setCodingRate4(LORA_CODING);
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.enableCrc();
  LoRa.setTxPower(LORA_TX_POWER_DBM, RF_PACONFIG_PASELECT_PABOOST);
  
  LoRa.beginPacket();

  //Header: To, From, Id, Flags
  LoRa.write(LORA_DESTINATION);
  LoRa.write(LORA_NODE);
  LoRa.write(LORA_MESSAGE_ID);
  LoRa.write(LORA_FLAGS);

  sendLoRaFloat(voltage_V);
  sendLoRaFloat(current_mA);
  sendLoRaFloat(humidity_PCT);
  sendLoRaFloat(temp_C);
  sendLoRaFloat(pressure_hPa);
  sendLoRaFloat(temp_2_C);

  LoRa.endPacket();

  delay(1000);

  LoRa.end();
}

void displayStatus(float voltage_V, float current_mA, float humidity_PCT, float temp_C, float pressure_hPa, float temp_2_C, long last_reading) {

  Serial.print("Bus Voltage:   "); Serial.print(voltage_V); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Humidity:      "); Serial.print(humidity_PCT); Serial.println("  %");
  Serial.print("AHT20 Temp:    "); Serial.print(temp_C); Serial.println("  C");
  Serial.print("Pressure:      "); Serial.print(pressure_hPa); Serial.println("  hPa");
  Serial.print("BMP280 Temp:   "); Serial.print(temp_2_C); Serial.println("  C");
  Serial.print("Last reading:  "); Serial.println(last_reading);
  Serial.println("");

#if DISPLAY_MEASUREMENTS

  if(!displayOn) {
    Heltec.display->displayOn();
    Heltec.display->init();
    Heltec.display->flipScreenVertically();  
    Heltec.display->setFont(ArialMT_Plain_10);

    displayOn = true;
  } else {
    Heltec.display->clear();
  }

  int y = 0;
  
  Heltec.display->drawString(0, y, "I:");
  Heltec.display->drawString(15, y, String(current_mA, 2));
  Heltec.display->drawString(45, y, "mA");
  Heltec.display->drawString(64, y, "| V:");
  Heltec.display->drawString(90, y, String(voltage_V, 2));
  Heltec.display->drawString(115, y, "V");

  y += 12;

  Heltec.display->drawString(0, y, "H:");
  Heltec.display->drawString(15, y, String(humidity_PCT, 2));
  Heltec.display->drawString(45, y, "%");
  Heltec.display->drawString(64, y, "| T:");
  Heltec.display->drawString(90, y, String(temp_C, 2));
  Heltec.display->drawString(115, y, "C");

  y += 12;

  Heltec.display->drawString(0, y, "P:");
  Heltec.display->drawString(15, y, String(pressure_hPa, 2));
  Heltec.display->drawString(45, y, "hPa");
  Heltec.display->drawString(64, y, "| T:");
  Heltec.display->drawString(90, y, String(temp_2_C, 2));
  Heltec.display->drawString(115, y, "C");
  
  Heltec.display->display();

#endif
 
}

void halt() {
  while(1) delay(100);
}

void handleError(String error){
  Serial.println(error);
  
  if(!displayOn) {
    Heltec.display->displayOn();
    Heltec.display->init();
    Heltec.display->flipScreenVertically();  
    Heltec.display->setFont(ArialMT_Plain_10);

    displayOn = true;
  } else {
    Heltec.display->clear();
  }
  
  Heltec.display->drawString(0, 0, "Error:");
  Heltec.display->drawString(0, 12, error);
  Heltec.display->display();
}

void setup() {
  //Begin with display and LoRa disabled, serial and PA boost enabled.
  Heltec.begin(false, //Display enabled
    false, //LoRa enabled
    true, //Serial enabled
    true /*PABOOST Enable*/, 
    LORA_FREQUENCY_HZ /*long BAND*/);

  while(!Serial) {
    delay(10);
  }

  i2c.begin(21, 22);

  if(!ina219.begin(&i2c)) {
    handleError("INA219 initialization failed");
    halt();
  }
  
  ina219.setCalibration_16V_400mA();

  if(!aht20.begin(&i2c)) {
    handleError("AHT20 initialization failed");
    halt();
  }

  if(!bmp280.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
    handleError("BMP280 initialization failed: " + String(bmp280.sensorID(), 16)) ;
    halt();
  }

  Serial.println("BMP280 sensor Id: " + String(bmp280.sensorID(), 16)) ;

  Serial.println("Initialization complete");

}

void sensorsOn() {
  ina219.powerSave(false);
}

void sensorsOff() {
  ina219.powerSave(true);
}

void loop() {
  // put your main code here, to run repeatedly:


  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  
  sensors_event_t humidity, temp;
  if(!aht20.getEvent(&humidity, &temp)) {
    handleError("Error reading AHT20");
    halt();
  }

  sensors_event_t bmp280_temp_reading, bmp280_pressure_reading;

  bmp280_temp->getEvent(&bmp280_temp_reading);
  bmp280_pressure->getEvent(&bmp280_pressure_reading);

  displayStatus(busvoltage, current_mA, humidity.relative_humidity, temp.temperature, bmp280_pressure_reading.pressure, bmp280_temp_reading.temperature, humidity.timestamp);

  sendLoRaMessage(busvoltage, current_mA, humidity.relative_humidity, temp.temperature, bmp280_pressure_reading.pressure, bmp280_temp_reading.temperature);

  delay(MEASUREMENT_FREQUENCY_MS);
}
