# Atualizações do andamento do projeto

- [x] Conexão WIFI
- [x] Conexão MQTT com o Home Assistant
- [x] Leitura de tensão, corrente, potência e outros
- [x] Banco de dados
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

Para salvar os dados será utilizado o File editor para criar as entidades que serão atribuidas aos dados que são recebidos via mqtt e o InfluxDB que será nosso banco de dados, para isso você deve instalá-los na loja de add-ons. 

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
```

Já no InfluxDB para criar um database você de ir em *InfluxDB Admin -> Database -> Create Database* nomeie ele e está feito. Ainda na mesma seção, crie um usuário e de permissão para leitura e escrita no database criado anteriormente.

Agora no novamente com o File Editor edite o novamente o arquivo */homeassistant/configuration.yaml* adicianando as informações de database e user criados, por exemplo:
```
influxdb:
    database: sensor
    username: admin
    password: admin123
```

Caso você queira visualizar os dados, você pode criar um dashboard em *Dashboard -> Create Dashboard*. Após criar uma célula, nomeie o gráfico, selecione o banco de dados e a informação desejada, por exemplo a corrente é 
 "A", após isso confimre.

<img alt="dashboard1" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/create%20dashboard%20test.png"/>

Criando outros gráficos voce terá um resultado semelhante a este:

<img alt="dashboard2" src="https://github.com/EmmanuelRGuesser/PJ_Integrador_3/blob/main/imgs/dashboard%20test.png"/>



