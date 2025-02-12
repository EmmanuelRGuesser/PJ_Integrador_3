---
<img align="right" img alt="CI" src="https://github.com/user-attachments/assets/821fb21c-a6b7-41a6-97db-38415a316944" width="111" />

# Medidor de consumo elétrico residencial com ESP32
---

Este projeto tem como objetivo realizar o projeto e implementação de um dispositivo de monitoramento do consumo energético de tomadas residenciais integrado ao Home Assistant.

Dispositivos como esse permitem aos usuários monitorar e controlar o consumo de energia, ajudando a identificar aparelhos que consomem muita energia e incentivar hábitos mais sustentáveis. Isso resulta em uma redução significativa na fatura de energia e contribui para a conservação de recursos naturais.

Com o aumento das tarifas de energia elétrica, ser capaz de identificar os maiores consumidores de energia em casa pode gerar economia substancial. Um dispositivo que fornece dados detalhados permite aos consumidores tomar decisões informadas para reduzir o consumo e, consequentemente, os custos.

## Funcionalidades:

- Integração com o Home Assistant;
- Monitoramento da tensão, corrente, potência, frequência e fator de potência;
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
- Display LCD 20x4
- Case takachi WPL15-15-6

---
## Descrição dos componentes

### ESP32
O ESP32 é um microcontrolador de baixo custo e baixa potência, desenvolvido pela Espressif Systems. Ele integra Wi-Fi e Bluetooth, tornando-se uma escolha popular para projetos de Internet das Coisas (IoT), além disso a comunidade em torno do ESP32 é enorme, com muitos recursos, bibliotecas e tutoriais disponíveis. A Espressif, empresa desenvolvedora, também oferece um SDK robusto (ESP-IDF). 

### PZEM-004t
O PZEM-004T é um módulo que possui funcionalidades de Voltimetro, Amperímetro e Wattimetro. Os dados são transmitidos para o microcontrolador por meio de comunicação serial TTL. 

### Fonte Hi-link 5V
É um conversor step down que trabalha com uma tensão de alimentação AC-DC 127/220V para 5V, com uma potencia de 5W, o qual será suficiente para alimentar o microcontrolador e os periféricos contidos no circuito. Foi escolhido esse modelo de fonte pois é compacta, facil de encontrar no mercado e possui certificações que garantem a qualidade da tensão e corrente entregues na saída, dentro da potencia especificada.

### LEDs
O dispositivo conta com dois leds que dão um feedback do funcionamento do sistema. Um led vermelho indicará que o circuito está ligado e outro led verde indicará estado de conexão com o wifi.

---
## Home Assistant

O Home Assistant é uma plataforma de automação residencial de código aberto que permite controlar dispositivos e sistemas em sua casa de maneira centralizada e automatizada. Ele suporta uma ampla variedade de dispositivos e protocolos, facilitando a criação de cenários de automação e controle de tudo, desde luzes e termostatos até câmeras de segurança e sistemas de entretenimento.

#### Principais características do Home Assistant:
- Automação: Criação de regras de automação personalizadas com base em eventos, horários ou condições específicas.
- Integrações: Suporte a milhares de dispositivos e serviços, como Philips Hue, Google Assistant, Amazon Alexa, entre muitos outros.
- Controle Local: Embora possa ser integrado à nuvem, o Home Assistant é projetado para funcionar localmente, o que oferece maior privacidade e controle.
- Dashboard Personalizado: A plataforma oferece uma interface visual personalizável para que você possa monitorar e controlar dispositivos.

#### Integração com ESP32 no Home Assistant
O ESP32 é um microcontrolador com conectividade Wi-Fi e Bluetooth, amplamente utilizado em projetos de automação e IoT (Internet das Coisas). No contexto do Home Assistant, o ESP32 é frequentemente usado para criar dispositivos personalizados, como sensores e atuadores. A integração entre o ESP32 e o Home Assistant ocorre principalmente por meio do protocolo MQTT Moskitto.

Funcionamento da Integração:

1. Configurar a rede Wi-Fi e login do Home Assistant na pagina WEB gerada pelo ESP.
2. O Home Assistant recebe os dados por meio do protocolo MQTT Moskito.
3. Armazena os dados em forma de entidades, no bando de dados InfluxDB.
4. No banco de dados possui um dashboard o qual mostra todos os dados plotados para o gerenciamento.

---

## Funcionamento do Monitor de Energia

Para o funcionamento do dispositivo devemos conectar a rede Wi-FI, isso é feito através do aplicativo ESP BLE Provising que está disponível na Play Store e App Store, leia o [código QR do link](https://espressif.github.io/esp-jumpstart/qrcode.html?data=%7B%22ver%22:%22v1%22,%22name%22:%22Monitor%20de%20energia%22,%22pop%22:%22abcd1234%22,%22transport%22:%22ble%22%7D) ou procure por monitor de energia e use a senha “abc1234”, com isso você pode ver as redes disponíveis e fazer a conexão, caso necessário reconfigurar a rede, somente pressione o botão ao lado da entrada de alimentação por 3 segundos que seu dispositivo irá reiniciar e basta seguir os passos anteriores. 

Agora é possível acessar a página de configurações através do Web Server gerado pela ESP32, acesse [monitor-energia.local](http://monitor-energia.local/), através dela você deve configurar o serviço MQTT para realizar a comunicação de dados entre o monitor de energia e o Home Assistant, use sua conta e senha do seu perfil do HA. Você também pode configurar os alertas, visualizar o consumo acumulado, além de ligar e desligar o dispositivo conectado através da página.

O sistema de alertas funciona como uma proteção que irá atuar desligando o dispositivo conectado ao monitor, a proteção age sobre tensão mínima e tensão, corrente e potência máxima, sempre que um alerta é disparado um aviso é exibido na tela, para voltar ao funcionamento basta clicar no botão ligar. Caso você queira não utilizar os alertas, somente deixe o item com 0. 
Na página de consumo acumulado é exibido quantidade de energia consumida pelo dispositivo conectado, com isso você pode estimar o valor gasto com ele, caso você queira trocar o dispositivo monitorado ou no que for necessário, você pode zerar o contador clicando em zerar energia.

Com isso, já é possível ver na tela LCD do monitor dados como tensão, corrente, potência e energia consumida, além dos avisos do dispositivos e informaçãoes já descritas acima, isso sem a necessidade de integrar com o Home Assistant , porém para conseguir monitorar os dados ao decorrer do tempo em um gráfico, você deve seguir os passos abaixo.

---
### Instalação do Home Assistant
Como o Home Assistant funciona localmente é necessário fazer a instalação em um computador, máquina virtual ou um Raspberry Pi por exemplo. 

Para acessar o processo de instalação [click aqui](https://www.home-assistant.io/installation/).

Com o Home Assistant instalado vamos adicionar o Moskitto MQTT

---
### Guia para Adicionar MQTT Mosquitto no Home Assistant
Instalar o Add-on:

Acesse Configurações → Add-ons, Backup & Supervisor
Vá em Loja de Add-ons e instale o Mosquitto broker
Configurar o Mosquitto:

No add-on, clique em Configuração e edite conforme necessário
Se precisar de autenticação, crie usuários em Configurações → Pessoas e Acesso
Habilitar e Iniciar:

Ative Iniciar na inicialização e Assistente de Rede
Clique em Iniciar
Verificar Funcionamento:

Vá em Configurações → Dispositivos e Serviços
Adicione uma integração MQTT, com isso no menu de configurações já deve ser possível se inscrever em algum tópico para receber os pacotes via MQTT.
No momento têm-se o tópico *sensor* e os sub tópicos *voltage, current, power, energy, frequency e pf*, para ver a tensão por exemplo, deve-se se inscrever no *sensor/voltage*, como resultado você deve ver algumo semelhante a isto:

<img alt="mqtt_msg" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/mqqt_msg.png" width="600" /> 

Para salvar os dados será utilizado o File editor para criar as entidades que serão atribuidas aos dados que são recebidos via mqtt.

Com o File Editor aditaremos o arquivo */homeassistant/configuration.yaml*, basta adicionar o seguinte código para criar as entidades:
```
mqtt:    
    sensor:
        - name: voltage
          state_topic: "sensor/voltage"
          unit_of_measurement: V

        - name: current
          state_topic: "sensor/current"
          unit_of_measurement: A

        - name: power
          state_topic: "sensor/power"
          unit_of_measurement: W

        - name: energy
          state_topic: "sensor/energy"
          unit_of_measurement: Wh

        - name: frequency
          state_topic: "sensor/frequency"
          unit_of_measurement: Hz
          
        - name: fp
          state_topic: "sensor/pf"
          unit_of_measurement: fp
```
Agora vamos criar o banco de dados com o InfluxDB, para armazenar os dados e os manipular.

---
### Adicionando o InfluxDB no Home Assistant
Instalar o Add-on:

Acesse Configurações → Add-ons, Backup & Supervisor
Instale o InfluxDB
Criar um Banco de Dados e Usuário:

No terminal do InfluxDB, execute:
```
CREATE DATABASE home_assistant;
CREATE USER ha_user WITH PASSWORD 'senha' WITH ALL PRIVILEGES;
```
(Opcional) Restringir permissões:
```
GRANT ALL ON home_assistant TO ha_user;
```
Configurar no Home Assistant:
Adicione no configuration.yaml:
```
influxdb:
  host: a0d7b954-influxdb
  port: 8086
  database: home_assistant
  username: ha_user
  password: senha
  max_retries: 3
  default_measurement: state
```
Reinicie o Home Assistant

Para Verificar Funcionamento:

Acesse Desenvolvedor → Ferramentas de Estado para ver os dados sendo enviados

Pronto! Agora o MQTT e o InfluxDB estão integrados ao Home Assistant. 🚀

---
### Gerar o Dashboard e os gráficos no InfluxDB
1. Acessar o InfluxDB
2. No Home Assistant, vá para Configurações → Add-ons, Backup & Supervisor
Abra o InfluxDB e clique em Abrir Web UI
3. Criar um Dashboard
No menu lateral, vá em Dashboards
Clique em + Create Dashboard
Dê um nome ao dashboard e clique em Create
4. Criar um Gráfico
No dashboard criado, clique em + Create Cell
Escolha o tipo de visualização (Linha, Barras, Gauge, etc.)
Selecione Configure Query e escolha o banco de dados home_assistant
5. Construir a Query
   
OBS: O código das query estão no arquivo [Código_dos_gráficos_influxDB.md](https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/C%C3%B3digo_dos_gr%C3%A1ficos_influxDB.md)

6. Utilizando os códigos, você pode personalizar os gráficos como desejar.

---

### Teste do Monitor de Energia

Para o teste vamos monitorar o carregamento de um notebook, após realizar as configuração de conexão do monitor e do Home Assistant conforme o passo a passo acima, podemos monitorar o consumo através da tela LCD no monitor ou pelo dashboard no Home Assistant.

<img alt="monitor" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/monitor%20de%20energia.jpg" width="400" /> <img alt="dashboard" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/dashboard.png" width="700" /> 

Como visto acima podemos monitorar os parâmetros elétricos do dispositivo conectado ao monitor. Vamos testar o sistema de proteção, como a potência consumida é entre 15W - 20 W então para provocar um alerta iremos limitar em 10W, para isso entramos no página web do monitor de energia → configurar alertas e colocamos Max Power com 10W.

<img alt="tela principal" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/menu%20de%20configuracao.png" width="350" /> <img alt="tela de alertas" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/tela%20de%20alertas.png" width="300" /> <img alt="alerta de potencia" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/alerta%20de%20potencia.jpg" width="350" />

Como vemos acima o monitor entra em modo proteção e desliga a carga, para voltar a funcionar é necessário ligar novamente no menu principal.

O consumo acumulado pode ser acessado pelo menu e la você poderá estimar o valor gasto pela caraga conectada, somente digite o valor pago por kWh e calcule. Caso você quiser zerar o contador nesta página também é possível fazer isto.

<img alt="tela energia consumida" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/tela%20de%20energia%20consumida.png" width="350" />

---

## Considerações finais

O desenvolvimento do projeto atingiu seus objetivos propostos, resultando em um dispositivo funcional capaz de monitorar o consumo elétrico de forma eficiente. A integração do hardware e software permitiu a coleta e visualização dos dados em tempo real, proporcionando um produto prático para automação residencial. Para projetos futuros, considera-se a possibilidade de utilizar um banco de dados mais robusto, um software mais avançado para a criação do dashboard e a adoção de componentes SMD para um design mais compacto, eficiente e profissional, reduzindo o tamanho da case e aprimorando a estética e usabilidade do dispositivo.









