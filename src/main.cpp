#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <Temperature_LM75_Derived.h>

// #define USE_ONBOARD_TEMP_SENSOR
#define USE_EXTERNAL_ADC_WITH_TIMER
// #define USE_EXTERNAL_ADC_WITH_ISR
// #define USE_INTERNAL_ADC

#define BUTTON1 PB3
#define BUTTON2 PB4
#define BUTTON3 PB5
#define LED1 PB13
#define LED2 PB14
#define LED3 PB15
#define ISO_OUTPUT PA12
#define ISO_INPUT PA11

// Internal ADC pins
#define SDA_PIN PB9
#define SCL_PIN PB8
//#define RDY_PIN PA15

#define READ_TEMP_PERIOD_SECONDS 10
#define READ_ADC_PERIOD_MICROSECONDS 1000

// Function prototype
void readTempIsr();
void readAdcIsr();
void NewDataReadyISR();

TwoWire wire_tempeture(PB9, PB8);
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Generic_LM75 temperature(&wire_tempeture);
HardwareSerial serial1(PA10, PA9);

#ifdef USE_EXTERNAL_ADC_WITH_ISR
bool new_data = false;
#endif

void setup() {
  Serial.begin(9600);
  
  // Print Header
  serial1.println("...............................");
  serial1.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  serial1.println("ADC Range: +/- 0.512 V");

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
     ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  // Button Digital outputs
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  pinMode(ISO_INPUT, OUTPUT);

  // LED Digital outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(ISO_OUTPUT, OUTPUT);

  if (!ads.begin(ADS1X15_ADDRESS, &wire_tempeture)) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  #ifdef USE_ONBOARD_TEMP_SENSOR
    HardwareTimer *timerTemp = new HardwareTimer(TIM1);
    timerTemp->setMode(1, TIMER_OUTPUT_DISABLED, NC);
    timerTemp->setOverflow(READ_TEMP_PERIOD_SECONDS * 1000000U, MICROSEC_FORMAT);
    timerTemp->attachInterrupt(readTempIsr);
    timerTemp->resume();
  #endif

  #ifdef USE_EXTERNAL_ADC_WITH_TIMER
    HardwareTimer *timerAdc = new HardwareTimer(TIM2);
    timerAdc->setMode(1, TIMER_OUTPUT_DISABLED, NC);
    timerAdc->setOverflow(READ_ADC_PERIOD_MICROSECONDS * 1000U, MICROSEC_FORMAT);
    timerAdc->attachInterrupt(readAdcIsr);
    timerAdc->resume();
  #endif

  #ifdef USE_EXTERNAL_ADC_WITH_ISR
    pinMode(RDY_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(RDY_PIN), NewDataReadyISR, RISING);
    
    ads.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, true);
  #endif
}

// put your main code here, to run repeatedly:
void loop() {
  // control LED
  digitalWrite(LED1, HIGH);
  digitalWrite(LED3, LOW);
  delay(1000);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED1, LOW);
  delay(1000);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED2, LOW);
  delay(1000);

  #ifdef USE_EXTERNAL_ADC_WITH_ISR
  if(!new_data) {
    return;
  } 

  int16_t results = ads.getLastConversionResults();

  Serial.print("Differential: "); Serial.print(results); Serial.print("("); Serial.print(ads.computeVolts(results)); Serial.println("V)");

  new_data = false;

  delay(1000); // Evitar muitos dados no terminal
  #endif
}

// ISR to read on board temperature sensor
void readTempIsr() {
  serial1.print("Temperature = ");
  serial1.print(temperature.readTemperatureC());
  serial1.println(" C");
}

// ISR to read ADC measurement
void readAdcIsr() {
  #ifdef USE_EXTERNAL_ADC_WITH_TIMER
  int16_t adcResult;

  const float adcScaling =  0.015625F; /* ADS1115  @ +/- 0.512 V gain (16-bit results) */

  adcResult = ads.readADC_Differential_0_1();

  Serial.print("Differential: "); 
  Serial.print(adcResult); 
  Serial.print("("); 
  Serial.print(adcResult * adcScaling); Serial.println("mV)");

  #endif

  #ifdef USE_INTERNAL_ADC
  adcResult = analogRead(PA2);

  serial1.print("PA2: "); 
  serial1.println(adcResult); 
  #endif
}

void NewDataReadyISR(){
  #ifdef USE_EXTERNAL_ADC_WITH_ISR
  bool new_data = true;
  #endif
}