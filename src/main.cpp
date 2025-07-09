#include <Arduino.h>
#include <SPI.h>
#include "ADS1292R.h"
#include "ecgResAlgorithm.h"

volatile uint8_t globalHeartRate = 0;
volatile uint8_t globalRespirationRate = 0;

#define SPI_SCK   18
#define SPI_MISO  19
#define SPI_MOSI  23
#define SPI_CS    5

#define ADS1292_DRDY_PIN   2
#define ADS1292_CS_PIN     5
#define ADS1292_START_PIN  4
#define ADS1292_PWDN_PIN   15

SPIClass *spi = &SPI;

int16_t ecgWaveBuff, ecgFilterout;
int16_t resWaveBuff, respFilterout;

ads1292r ADS1292R;
ecg_respiration_algorithm ECG_RESPIRATION_ALGORITHM;

void readADS1292R_ID();

void setup() {
  delay(2000);

  Serial.begin(57600);
  while (!Serial); 
  spi->begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);

  pinMode(ADS1292_DRDY_PIN, INPUT);
  pinMode(ADS1292_CS_PIN, OUTPUT);
  pinMode(ADS1292_START_PIN, OUTPUT);
  pinMode(ADS1292_PWDN_PIN, OUTPUT);

  ADS1292R.ads1292Init(ADS1292_CS_PIN, ADS1292_PWDN_PIN, ADS1292_START_PIN);

  Serial.println("ADS1292R initialized successfully.");
  readADS1292R_ID();
}

void loop() {
  ads1292OutputValues ecgRespirationValues;

  bool ret = ADS1292R.getAds1292EcgAndRespirationSamples(ADS1292_DRDY_PIN, ADS1292_CS_PIN, &ecgRespirationValues);

  if (ret) {
    ecgWaveBuff = (int16_t)(ecgRespirationValues.sDaqVals[1] >> 8);
    resWaveBuff = (int16_t)(ecgRespirationValues.sresultTempResp >> 8);

    if (!ecgRespirationValues.leadoffDetected) {
      ECG_RESPIRATION_ALGORITHM.ECG_ProcessCurrSample(&ecgWaveBuff, &ecgFilterout);
      ECG_RESPIRATION_ALGORITHM.QRS_Algorithm_Interface(ecgFilterout, &globalHeartRate);

      respFilterout = ECG_RESPIRATION_ALGORITHM.Resp_ProcessCurrSample(resWaveBuff);
      ECG_RESPIRATION_ALGORITHM.RESP_Algorithm_Interface(respFilterout, &globalRespirationRate);

      Serial.print("ECG:");
      Serial.print(ecgFilterout);
      Serial.print(",RESP:");
      Serial.print(respFilterout);
      Serial.print(",HR:");
      Serial.print(globalHeartRate);
      Serial.print(",RR:");
      Serial.println(globalRespirationRate);
    } else {
      Serial.println("Lead-off detected!");
    }
  }
}

void readADS1292R_ID() {

  digitalWrite(ADS1292_CS_PIN, LOW);
  delayMicroseconds(2);

  spi->transfer(RREG | ADS1292_REG_ID); 
  spi->transfer(0x00);          
  uint8_t chip_id = spi->transfer(0x00);  
  digitalWrite(ADS1292_CS_PIN, HIGH);

  Serial.print("ADS1292R Chip ID: 0x");
  Serial.println(chip_id, HEX);

  if (chip_id == 0x73) {
    Serial.println("Chip  (0x73)");
  } else {
    Serial.println("Not match");
  }
}
