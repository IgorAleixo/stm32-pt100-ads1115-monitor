#pragma once
#include <Arduino.h>

/**
 * @brief Calcula a resistência do sensor PT-100 a partir da tensão medida
 * @param volt_ads Tensão em volts obtida do conversor AD
 * @return float Valor da resistência em ohms (Ω)
 * @details Como o sensor PT-100 é parte de uma ponte Wheatstone, a resistência é 
 *          calculada com base na tensão medida e no ganho do amplificador diferencial
 *          utilizado (ex: INA143). A função considera os valores de referência e os 
 *          fatores de calibragem para fornecer uma leitura precisa da resistência.
 */
float computeResist(float volt_ads);

/**
 * @brief Calcula a temperatura inversa da equação de Callendar-Van Dusen
 * @param R Valor da resistência em ohms do sensor PT-100
 * @return float Temperatura em graus Celsius (°C)
 * @details Implementa a função inversa da equação CVD para resistores Pt100
 *          válida para faixa de temperatura de 0°C a 850°C. A equação é 
 *          baseada nos coeficientes de temperatura do Pt100 no padrão IEC-60751.
 */
float CVD_inverse(float R);

/**
 * @brief Escreve os dados processados no terminal serial formatado
 * @param serial1 Referência para a porta serial de comunicação
 * @param value Valor bruto do conversor analógico-digital
 * @param voltage Tensão medida em volts
 * @param resistence Resistência calculada em ohms (Ω)
 * @param temperature Temperatura final convertida em graus Celsius (°C)
 * @return void
 * @details Formata e transmite os dados via serial para monitoramento em tempo real
 */
void writeData(HardwareSerial& serial1,
               int16_t value, 
               float voltage, 
               float resistence, 
               float temperature
);

/**
 * @file DataProcessing.h
 * @brief Módulo de processamento de dados para leitura e conversão de sensores PT-100
 * @details Fornece funções para computar resistência, temperatura e exibir dados processados
 */

/**
 * @brief Processa dados brutos do sensor e exibe no terminal serial
 * @param valor Valor bruto lido do conversor AD (12/16 bits)
 * @param volt_ads Tensão em volts lida do conversor analógico-digital
 * @param serial1 Referência para a porta serial de comunicação (Serial1)
 * @return void
 * @note Função coordenadora que integra leitura, conversão e exibição de dados
 */
void comuputeData(int16_t valor,
                  float volt_ads,
                  HardwareSerial& serial1);