//////////////////////////////////////////////////////////////////////////////////////////
//
//   Arduino Library for ADS1292R Shield/Breakout
//
//   Copyright (c) 2017 ProtoCentral
//   Heartrate and respiration computation based on original code from Texas Instruments
//
//   This is a simple example to plot ECG through arduino serial plotter.
//
//   This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//  
//
//  |ads1292r pin label| Arduino Connection   |Pin Function      |
//  |----------------- |:--------------------:|-----------------:|
//  | VDD              | +3V3                  |  Supply voltage  |
//  | PWDN/RESET       | D7                   |  Reset           |
//  | START            | A5                   |  Start Input     |
//  | DRDY             | D2                   |  Data Ready Outpt|
//  | CS               | D10                  |  Chip Select     |
//  | MOSI             | D11                  |  Slave In        |
//  | MISO             | D12                  |  Slave Out       |
//  | SCK              | D13                  |  Serial Clock    |
//  | GND              | Gnd                  |  Gnd             |
//
/////////////////////////////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include "ADS1292R.h"
#include "ecgResAlgorithm.h"
#include <SPI.h>

volatile uint8_t globalHeartRate = 0;
volatile uint8_t globalRespirationRate = 0;


#define ESP32
// ESP32 SPI default pins (VSPI): SCK=18, MISO=19, MOSI=23, SS=5
// You can change these if needed, or use HSPI (SCK=14, MISO=12, MOSI=13, SS=15)
#ifdef ESP32
  #define ADS1292_DRDY_PIN  2
  #define ADS1292_CS_PIN    5   // VSPI SS default
  #define ADS1292_START_PIN 4   // Choose a free GPIO
  #define ADS1292_PWDN_PIN  15  // Choose a free GPIO
  SPIClass *spi = &SPI;
#else
  const int ADS1292_DRDY_PIN = 2;
  const int ADS1292_CS_PIN = 10;
  const int ADS1292_START_PIN = A5;
  const int ADS1292_PWDN_PIN = 7;
#endif

int16_t ecgWaveBuff, ecgFilterout;
int16_t resWaveBuff, respFilterout;

ads1292r ADS1292R;
ecg_respiration_algorithm ECG_RESPIRATION_ALGORITHM;

void setup()
{
  delay(2000);

#ifdef ESP32
  // VSPI default: SCK=18, MISO=19, MOSI=23, SS=5
  spi->begin(); // Use default VSPI pins or specify pins: spi->begin(SCK, MISO, MOSI, SS)
#else
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  //CPOL = 0, CPHA = 1
  SPI.setDataMode(SPI_MODE1);
  // Selecting 1Mhz clock for SPI
  SPI.setClockDivider(SPI_CLOCK_DIV16);
#endif

  pinMode(ADS1292_DRDY_PIN, INPUT);
  pinMode(ADS1292_CS_PIN, OUTPUT);
  pinMode(ADS1292_START_PIN, OUTPUT);
  pinMode(ADS1292_PWDN_PIN, OUTPUT);

  Serial.begin(57600);

#ifdef ESP32
  ADS1292R.ads1292Init(ADS1292_CS_PIN, ADS1292_PWDN_PIN, ADS1292_START_PIN);
#else
  ADS1292R.ads1292Init(ADS1292_CS_PIN, ADS1292_PWDN_PIN, ADS1292_START_PIN);
#endif

  Serial.println("Initialization is done");
}

void loop()
{
  ads1292OutputValues ecgRespirationValues;

#ifdef ESP32
  bool ret = ADS1292R.getAds1292EcgAndRespirationSamples(ADS1292_DRDY_PIN, ADS1292_CS_PIN, &ecgRespirationValues);
#else
  boolean ret = ADS1292R.getAds1292EcgAndRespirationSamples(ADS1292_DRDY_PIN, ADS1292_CS_PIN, &ecgRespirationValues);
#endif

  if (ret == true)
  {
    ecgWaveBuff = (int16_t)(ecgRespirationValues.sDaqVals[1] >> 8);  // ignore the lower 8 bits out of 24bits
    resWaveBuff = (int16_t)(ecgRespirationValues.sresultTempResp >> 8);

    if (ecgRespirationValues.leadoffDetected == false)
    {
      ECG_RESPIRATION_ALGORITHM.ECG_ProcessCurrSample(&ecgWaveBuff, &ecgFilterout);   // filter out the line noise @40Hz cutoff 161 order
      ECG_RESPIRATION_ALGORITHM.QRS_Algorithm_Interface(ecgFilterout, &globalHeartRate); // calculate
      //respFilterout = ECG_RESPIRATION_ALGORITHM.Resp_ProcessCurrSample(resWaveBuff);
      //ECG_RESPIRATION_ALGORITHM.RESP_Algorithm_Interface(respFilterout, &globalRespirationRate);

    }
    else
    {
      ecgFilterout = 0;
      respFilterout = 0;
    }
    //if you want ecg data comment second line and if you want res datas on plotter comment first line and uncomment second line
    Serial.println(ecgFilterout);
    //Serial.println(ecgWaveBuff);
    //Serial.println(resWaveBuff);
  }
}
