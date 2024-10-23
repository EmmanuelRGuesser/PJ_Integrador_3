---
<img align="right" img alt="CI" src="https://github.com/user-attachments/assets/821fb21c-a6b7-41a6-97db-38415a316944" width="111" />

# Medidor de consumo elétrico residencial com ESP32
---

Este projeto tem como objetivo realizar o projeto e implementação de um dispositivo de monitoramento do consumo energético de tomadas residenciais integrado ao Home Assistant.

Dispositivos como esse permitem aos usuários monitorar e controlar o consumo de energia, ajudando a identificar aparelhos que consomem muita energia e incentivar hábitos mais sustentáveis. Isso resulta em uma redução significativa na fatura de energia e contribui para a conservação de recursos naturais.

Com o aumento das tarifas de energia elétrica, ser capaz de identificar os maiores consumidores de energia em casa pode gerar economia substancial. Um dispositivo que fornece dados detalhados permite aos consumidores tomar decisões informadas para reduzir o consumo e, consequentemente, os custos.

## Funcionalidades:

- Integração com o Home Assistant;
- Monitoramento de tensão, corrente e potência;
- Visualiazação do histórico de consumo;
- Sistema de acionamento a distância;
- Sistema de proteção do dispositivo.  

## Dispositivo

O dispositivo funcionará entre as tomadas adquirindo os dados por meio do sensor PZEM-004T, abaixo temos uma representação dele:

<img alt="dispositivo" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/dispositivo%20de%20teste.jpg" width="400" />

Ele deve ser conectado a tomada para o fornecimento de energia, dentro da case possui uma fonte para alimentar o ciruito. Esse circuito é composto pelo microcontrolador(ESP32), sensor(PZEM-004t), relé de acionamento e leds para facil identificação do funcionamento, já a interface será integrada com o Home Assistant.

Para o projeto será utilizado os seguintes componentes:
- ESP32
- PZEM-004t
- Relé
- Fonte Hi-link 5V 3W
- 2 leds
- Case

## Descrição dos componentes

### ESP32

### PZEM-004t
O PZEM-004T é um módulo que possui funcionalisades de Voltimetro, Amperímetro e Wattimetro. Os dados são transmitidos para o microcontrolador por meio de comunicação serial TTL. 

### Fonte Hi-link 5V
É um conversor step down que trabalha com uma tensão de alimentação AC-DC 127/220V para 3.3V, com uma potencia de 3W, o qual será suficiente para alimentar o microcontrolador e os periféricos contidos no circuito. Foi escolhido esse modelo de fonte pois é compacta, facil de encontrar no mercado e possui certificações que garantem a qualidade da tensão e corrente entregues na saída, dentro da potencia especificada.


