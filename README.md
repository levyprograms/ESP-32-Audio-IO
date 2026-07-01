# Interface de Áudio Wi-Fi Bidirecional com ESP32 🎧📡

Este projeto transforma um ESP32 padrão num sistema profissional de transmissão e recepção de áudio sem fios com qualidade de CD (44.1kHz/16-bits), utilizando o protocolo **VBAN** para integração em tempo real com **Voicemeeter** ou **OBS Studio**.

Desenvolvido para captura de mesas de som e retorno, superando as graves limitações de qualidade e latência do Bluetooth clássico. O dispositivo inclui um portal cativo web (Captive Portal) para configuração e um visor OLED integrado para monitoramento das métricas em tempo real (VU Meter).

## 🚀 Funcionalidades
* **Áudio Full-Duplex (Bidirecional)** simultâneo (I2S).
* Captura de áudio usando módulo ADC **PCM1808**.
* Reprodução de retorno/música usando módulo DAC **PCM5102A**.
* Interface de rede gráfica via web.
* Visor OLED integrado para acompanhamento do relógio NTP, IP, Força do Sinal Wi-Fi (RSSI) e Níveis de Volume de Áudio.
* Ligação e reinicialização automáticas, com armazenamento protegido não-volátil (NVS).

---

## 🛠️ Hardware Necessário

O código e o mapeamento de hardware foram desenvolvidos especificamente para as versões de breakout boards para facilitar a montagem (sem soldagem SMD).

1. **Placa de Desenvolvimento:** [ESP32 com Ecrã OLED 0.96" Integrado (I2C)](https://shopee.com.br/Placa-de-Desenvolvimento-ESP32-com-Display-OLED-de-0-96-Polegadas-M%C3%B3dulo-Sem-Fio-WiFi-BLE-e-Micro-USB-para-Arduino-i.315831373.43178345220)
2. **ADC (Entrada da Mesa):** [Placa Amplificadora Estéreo de Alta Resolução PCM1808 (24-Bits)](https://shopee.com.br/Placa-Amplificadora-ADC-Est%C3%A9reo-De-Alta-Resolu%C3%A7%C3%A3o-PCM1808-105dB-SNR-24-Bits-Para-Audi%C3%B3filos-i.426921557.28887729677)
3. **DAC (Saída para Mesa):** [Módulo Reprodutor Áudio PCM5102A Interface Digital I2S](https://shopee.com.br/Leitor-De-%C3%81udio-SHEENGD1-PCM5102A-I2S-M%C3%B3dulo-Reprodutor-I2S-Digital-Interface-PCM5102A-DAC-i.594020711.40078197333)
4. **Alimentação:** Carregador de qualidade superior (5V / 2A mínimo) para alimentar a porta Micro-USB do ESP32.
5. **Conectores / Complementos:**
   - 2x Conectores Jack P10 Fêmea de Painel (Para o *Line In*).
   - 1x Conector Jack P10 ou P2 Fêmea de Painel (Para o *Line Out*).
   - Fio Blindado de Áudio (para evitar ruídos do rádio Wi-Fi na fase analógica).
   - Placa de ensaio (Protoboard) e jumpers Dupont fêmea-macho.
   - *Recomendado:* 2 capacitores eletrolíticos (220µF) na linha 3.3V / GND.

---

## 🔌 Esquema de Ligações (Pinout)

[cite_start]O código foi rigorosamente desenhado para evitar conflito de barramentos entre o display OLED interno e os dois barramentos de áudio I2S utilizados. 

> **Aviso:** A placa ESP32 deve alimentar os módulos PCM de áudio exclusivamente no pino `3.3V`, todos devem partilhar a mesma ligação de Terra (`GND`).

### 1. Conexão do Ecrã OLED (Interno à placa recomendada)
| ESP32 Pin | Função I2C |
| :--- | :--- |
| **GPIO 21** | SDA (Data) |
| **GPIO 22** | SCL (Clock) |

### 2. Conexão da Entrada Analógica (PCM1808 - Line In)
Captura o áudio vindo da sua mesa de som e injeta digitalmente no ESP32.

| PCM1808 Pin | ESP32 Pin | Função I2S (Barramento 1 - Apenas RX) |
| :--- | :--- | :--- |
| **BCK** | **GPIO 14** | Bit Clock |
| **LRCK** | **GPIO 12** | Word Select (Left/Right Clock) |
| **DOUT** | **GPIO 13** | Digital Audio Data Input (Entra no ESP32) |
| **SCK** | **GND** | System Clock (Mantido no terra em modo Master/ESP) |

### 3. Conexão da Saída Analógica (PCM5102A - Line Out)
Recebe música digital do ESP32 e converte em analógico para enviar à mesa/fone.

| PCM5102A Pin | ESP32 Pin | Função I2S (Barramento 0 - Apenas TX) |
| :--- | :--- | :--- |
| **BCK** | **GPIO 26** | Bit Clock |
| **LCK (LRCK)** | **GPIO 25** | Word Select (Left/Right Clock) |
| **DIN** | **GPIO 18** | Digital Audio Data Output (Sai do ESP32) |
| **SCK** | **GND** | System Clock |

---

## ⚙️ Como Usar

### Instalação Física
1. Instale o firmware através do **Arduino IDE**.
2. Certifique-se de instalar as bibliotecas `Adafruit_GFX` e `Adafruit_SSD1306`.
3. Selecione o modelo padrão ESP32 Dev Module e faça o upload.

### Primeira Configuração
1. Assim que for ligado, o visor OLED indicará o estado: `MODO CONFIGURACAO`.
2. Aceda, no seu smartphone, à rede Wi-Fi criada: **`ESP32_Audio_Setup`**.
3. Abra o navegador e digite o endereço IP: **`192.168.4.1`**.
4. Irá abrir a nossa interface web. Selecione a rede do local, insira a palavra-passe e defina qual será o Endereço IP do seu Computador (onde corre o OBS / Voicemeeter).
5. Prima Guardar. O ESP32 reiniciará e ligar-se-á autonomamente à rede da Igreja ou Estúdio.

### Interação com o Voicemeeter
- No computador anfitrião, abra o **Voicemeeter** (Potato ou Banana).
- Ligue a funcionalidade **VBAN**.
- **Entrada (Som da Mesa para Live):** Em *Incoming Streams*, adicione um stream com o IP indicado no ecrã OLED e nomeie-o `Stream1`.
- **Saída (Música do PC para a Mesa):** Em *Outgoing Streams*, aponte para o IP do ESP32 na porta 6980.

---
*Desenvolvido em C++ / ESP-IDF nativo portado para Arduino Core.*