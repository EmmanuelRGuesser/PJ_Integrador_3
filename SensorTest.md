# Atualizações do andamento do projeto

- [x] Conexão WIFI
- [x] Conexão MQTT com o Home Assistant
- [x] Leitura de tensão, corrente, potência e outros
- [ ] Banco de dados
- [ ] Dashboard
- [ ] Alarmes e sistema de proteção
- [ ] Busca automática do broker ip

## Manual de utilização

No arquivo *config.h* você deve colocar os dados da rede wifi, o usuário e senha da sua conta do home assistant e URI do broker MQTT.

Já com o Home Assistant instalado, baixe o Mosquitto Broker na loja de add-ons e adicione no menu de integrações, com isso no menu de configurações já deve ser possível se inscrever em algum tópico para receber os pacotes via MQTT.
No momento têm-se o tópico *sensor* e os sub tópicos *voltage, current, power, energy, frequency e pf*, para ver a tensão por exemplo, deve-se se inscrever no *sensor/voltage*, como resultado você deve ver algumo semelhante a isto:

<img alt="mqtt_msg" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/mqqt_msg.png" width="600" />

Para o teste foram feita as conexões seguindo o datasheet do PZEM-004T:

<img alt="teste" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/montagem.jpg" width="600" />
