# Monitor de temperatura por PT-100 (STM32)

Este projeto foi desenvolvido para a disciplina de Lab. de Eletrônica Embarcada. O objetivo é realizar a leitura precisa de temperatura com um sensor PT-100 em configuração de ponte de Wheatstone, utilizando a alta resolução do módulo ADS1115 para contornar as limitações do ADC nativo do STM32.

## Tecnologias e Componentes
* MCU: STM32F103C8T6
* ADC: ADS1115 (16-bit via I2C)
* Sensor: PT-100 (RTD)
* Framework: Arduino Framework
* Comunicação: I2C para o sensor e UART para debug

## Funcionalidades
* Leitura de dados brutos do ADS1115 via I2C.
* Conversão precisa de Resistência -> Temperatura (Callendar-Van Dusen).
* Implementação de média móvel para filtragem de ruído.
* Interface Serial/LCD para exibição.

## Pinagem
|Componente| Pino| Função|
|:---|:---|:---|
|ADS1115 SCL|PB8 STM32|I2C Clock|
|ADS1115 SDA|PB9 STM32|I2C Data|
|Sensor PT-100|A0 ADS1115|RTD measument|


## Créditos
Este trabalho é uma derivação e evolução do material de laboratório disponibilizado pelo **Prof. Tomás P. Corrêa**, disponível no repositório:
🔗 [tpcorrea/LabDevBoard](https://github.com/tpcorrea/LabDevBoard)

As modificações principais incluem na implematação da leitura por interrupção do ADS1115 e nas funções que calculam a resistência e temperatura do PT-100.

