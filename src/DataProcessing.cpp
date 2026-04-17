#include <DataProcessing.h>

float computeResist(float volt_ads) {
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

  // Antes da medição, existe um amplificador diferencial 
  // com ganho 10 (INA143).
  float aux = ((volt_ads - Vref)/Ganho_diferencial + Vn)/Vref;
  
  float resistence = R4*aux/(1 - aux);

  // Fatores de calibragem
  constexpr float A = 0.9182602607f;
  constexpr float B = 9.974427365f;

  return (resistence - B)/A;
}


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


void writeData(
    HardwareSerial& serial1,
    int16_t value, 
    float voltage, 
    float resistence, 
    float tempeture) {

  serial1.print("Valor: "); 
  serial1.println(value); 

  serial1.print("Tensão: "); 
  serial1.print(voltage); 
  serial1.println(" V");

  serial1.print("Resistencia: "); 
  serial1.print(resistence); 
  serial1.println(" Ohm");

  serial1.print("Temperatura: "); 
  serial1.print(tempeture); 
  serial1.println(" C");
}

void comuputeData(int16_t valor,
                  float volt_ads,
                  HardwareSerial& serial1) {
    float resistence = computeResist(volt_ads); 
    float tempeture = CVD_inverse(resistence); 

    writeData(serial1, valor, volt_ads, resistence, tempeture);
}