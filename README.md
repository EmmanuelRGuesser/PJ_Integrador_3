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

Ele deve ser conectado a tomada para o fornecimento de energia, dentro da case possui uma fonte para alimentar o ciruito. Esse circuito é composto pelo microcontrolador(ESP32), sensor(PZEM-004t), relé de acionamento e leds para fácil identificação do funcionamento, já a interface será integrada com o Home Assistant.

Para o projeto será utilizado os seguintes componentes:
- ESP32
- PZEM-004t
- Relé
- Fonte Hi-link 5V 3W
- 2 leds
- Case

## Descrição dos componentes

### ESP32
O ESP32 é um microcontrolador de baixo custo e baixa potência, desenvolvido pela Espressif Systems. Ele integra Wi-Fi e Bluetooth, tornando-se uma escolha popular para projetos de Internet das Coisas (IoT), além disso a comunidade em torno do ESP32 é enorme, com muitos recursos, bibliotecas e tutoriais disponíveis. A Espressif, empresa desenvolvedora, também oferece um SDK robusto (ESP-IDF). 

### PZEM-004t
O PZEM-004T é um módulo que possui funcionalidades de Voltimetro, Amperímetro e Wattimetro. Os dados são transmitidos para o microcontrolador por meio de comunicação serial TTL. 

### Fonte Hi-link 5V
É um conversor step down que trabalha com uma tensão de alimentação AC-DC 127/220V para 3.3V, com uma potencia de 3W, o qual será suficiente para alimentar o microcontrolador e os periféricos contidos no circuito. Foi escolhido esse modelo de fonte pois é compacta, facil de encontrar no mercado e possui certificações que garantem a qualidade da tensão e corrente entregues na saída, dentro da potencia especificada.

### LEDs
O dispositivo conta com dois leds que dão um feedback do funcionamento do sistema. Um led vermelho indicará que o circuito está ligado e outro led verde indicará estado de conexão com o wifi.

## Home Assistant

O Home Assistant é uma plataforma de automação residencial de código aberto que permite controlar dispositivos e sistemas em sua casa de maneira centralizada e automatizada. Ele suporta uma ampla variedade de dispositivos e protocolos, facilitando a criação de cenários de automação e controle de tudo, desde luzes e termostatos até câmeras de segurança e sistemas de entretenimento.

#### Principais características do Home Assistant:
- Automação: Criação de regras de automação personalizadas com base em eventos, horários ou condições específicas.
- Integrações: Suporte a milhares de dispositivos e serviços, como Philips Hue, Google Assistant, Amazon Alexa, entre muitos outros.
- Controle Local: Embora possa ser integrado à nuvem, o Home Assistant é projetado para funcionar localmente, o que oferece maior privacidade e controle.
- Dashboard Personalizado: A plataforma oferece uma interface visual personalizável para que você possa monitorar e controlar dispositivos.

#### Integração com ESP32 no Home Assistant
O ESP32 é um microcontrolador com conectividade Wi-Fi e Bluetooth, amplamente utilizado em projetos de automação e IoT (Internet das Coisas). No contexto do Home Assistant, o ESP32 é frequentemente usado para criar dispositivos personalizados, como sensores e atuadores. A integração entre o ESP32 e o Home Assistant ocorre principalmente por meio de firmwares especializados, como o ESPHome.

Funcionamento da Integração:

1. ESPHome: O ESPHome é um firmware que transforma o ESP32 (e outros microcontroladores) em dispositivos IoT que podem ser facilmente integrados ao Home Assistant. O ESPHome permite configurar sensores, relés, LEDs e outros componentes conectados ao ESP32 por meio de arquivos YAML, que são então compilados e enviados para o dispositivo.

2. Conexão via Wi-Fi: Uma vez que o firmware ESPHome é instalado no ESP32, ele pode se conectar à rede Wi-Fi e se comunicar com o Home Assistant usando o protocolo MQTT ou diretamente pela API do ESPHome.

3. Integração no Home Assistant:

    - No Home Assistant, você pode adicionar o ESP32 como um dispositivo usando a integração do ESPHome.
    - O ESP32, com seu firmware configurado, aparece como um dispositivo no painel do Home Assistant, e os sensores ou atuadores conectados ao ESP32 podem ser usados para criar automações.
  
### Instalação
Como o Home Assistant funciona localmente é necessário fazer a instalação em um computador, máquina virtual ou um Raspberry Pi por exemplo. 

Para acessar o processo de instalação [click aqui](https://www.home-assistant.io/installation/).
