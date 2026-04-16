#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <Temperature_LM75_Derived.h>

// #define USE_ONBOARD_TEMP_SENSOR
// #define USE_EXTERNAL_ADC_WITH_TIMER
#define USE_EXTERNAL_ADC_WITH_ISR
// #define USE_INTERNAL_ADC

#define BUTTON1 PB3
#define BUTTON2 PB4
#define BUTTON3 PB5
#define LED1 PB13
#define LED2 PB14
#define LED3 PB15
#define ISO_OUTPUT PA12
#define ISO_INPUT PA11

// External ADC pins
#define RDY_PIN PA15

#define READ_TEMP_PERIOD_SECONDS 10
#define READ_ADC_PERIOD_MICROSECONDS 1000

// Function prototype
void readTempIsr();
void readAdcIsr();
float CVD_inverse(float R);

TwoWire wire(PB9, PB8); /* Fios do I2C */
Adafruit_ADS1115 ads;
Generic_LM75 temperature(&wire);
HardwareSerial serial1(PA10, PA9);

uint32_t tempo_anterior = millis();
uint8_t  estado_LEDS = 0; /* Controla o estado dos LEDS */

#ifdef USE_EXTERNAL_ADC_WITH_ISR
volatile bool new_data = false;
#endif

#ifdef USE_EXTERNAL_ADC_WITH_TIMER
volatile bool new_data = false;
#endif

void setup() {
  serial1.begin(9600);
  
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

  if (!ads.begin(73U, &wire)) {
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
    attachInterrupt(digitalPinToInterrupt(RDY_PIN), readAdcIsr, RISING);
    
    ads.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, true);
  #endif

  // Tensão inicial dos LEDs
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, LOW);

  
}

// put your main code here, to run repeatedly:
void loop() {
  // control LED
  uint32_t tempo_atual = millis();

  if(tempo_atual - tempo_anterior > 1000) {
    switch (estado_LEDS)
    {
    case 0:
      digitalWrite(LED1, HIGH);
      digitalWrite(LED3, LOW);

      estado_LEDS = 1;

      break;
    case 1:
      digitalWrite(LED2, HIGH);
      digitalWrite(LED1, LOW);

      estado_LEDS = 2;

      break;

    case 2:
      digitalWrite(LED3, HIGH);
      digitalWrite(LED2, LOW);

      estado_LEDS = 0;

      break;
    
    default:
      estado_LEDS = 0;

      break;
    }
  }

  #ifdef USE_EXTERNAL_ADC_WITH_ISR
  if(!new_data) {
    return;
  } 

  int16_t results = ads.getLastConversionResults();
  float volt_ads = ads.computeVolts(results);

  constexpr uint16_t R4  = 1130;
  constexpr uint16_t R5  = 11300;
  constexpr uint16_t R13 = 1210;

  constexpr float Vref = 2.5f;
  constexpr float Ganho_diferencial = 10.0f;

  // Vp = Vref * R/(R + R4);
  constexpr float Vn = Vref * R13/(R13 + R5);

  // Volt_ads = Ganho_diferencial*(Vp - Vn)
  // Vp = Volt_ads/Ganho_diferencial + Vn
  // R/(R + R4) = (Volt_ads/Ganho_diferencial + Vn)/Vref == aux

  float aux = (volt_ads/Ganho_diferencial + Vn)/Vref;
  float resistencia = R4*aux/(1 - aux);

  // Por interpolação linear 
  float tempeture = CVD_inverse(resistencia); 

  serial1.print("Valor: "); 
  serial1.println(results); 

  serial1.print("Tensão: "); 
  serial1.print(volt_ads); 
  serial1.println(" mV");

  serial1.print("Resistencia: "); 
  serial1.print(resistencia); 
  serial1.println(" Ohm");

  serial1.print("Temperatura: "); 
  serial1.print(tempeture); 
  serial1.println(" °C");

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
  new_data = true;
  #endif

  #ifdef USE_EXTERNAL_ADC_WITH_ISR
  new_data = true;
  #endif

  #ifdef USE_INTERNAL_ADC
  adcResult = analogRead(PA2);

  serial1.print("PA2: "); 
  serial1.println(adcResult); 
  #endif
}

// Função inversa da equação de Callendar-Van Dusen(CVD)
float CVD_inverse(float R) {
  constexpr float cnt_A = 3.9083e-3f;
  constexpr float cnt_B = -5.775e-7f;
  constexpr float cnt_C = -4.183e-12f;
  
  constexpr uint16_t R0  = 100;

  constexpr float a = cnt_B * R0; 
  constexpr float b = cnt_A * R0;
  
  float c = R0 - R;

  return (-b + sqrt(b*b - 4*a*c))/(2*a);
}