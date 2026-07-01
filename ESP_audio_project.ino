/*
 * INTERFACE DE ÁUDIO ESP32 - Placa de Som para Live via VBAN/Wi-Fi
 * Hardware: ESP32 + OLED SSD1306 0.96" + PCM1808 (ADC) + PCM5102A (DAC)
 */

#include <WiFi.h>             // Biblioteca nativa para Wi-Fi
#include <WiFiUdp.h>          // Biblioteca para enviar e receber pacotes UDP
#include <WebServer.h>        // Biblioteca para o portal de configuração
#include <Preferences.h>      // Biblioteca para salvar Wi-Fi e IP na memória flash
#include <driver/i2s.h>       // Driver de áudio de alta qualidade do ESP32
#include <Adafruit_GFX.h>     // Biblioteca gráfica para o display OLED
#include <Adafruit_SSD1306.h> // Driver específico para o display OLED SSD1306
#include "time.h"             // Sincronização de data e hora via NTP

// ============================================================
// DEFINIÇÃO DE PINOS
// ============================================================
// Pinos I2C para o Display OLED
#define OLED_SDA  21    
#define OLED_SCL  22    

// Pinos I2S de Transmissão (TX) → Envia áudio para o PCM5102A (Saída de Linha)
#define I2S_TX_BCK   26   // Bit Clock  → Vai para o pino BCK do PCM5102A
#define I2S_TX_WS    25   // Word Select → Vai para o pino LCK (ou LRCK) do PCM5102A
#define I2S_TX_DOUT  18   // Data Out   → Vai para o pino DIN do PCM5102A

// Pinos I2S de Recepção (RX) → Recebe áudio da mesa de som pelo PCM1808 (Entrada)
#define I2S_RX_BCK   14   // Bit Clock  → Vai para o pino BCK do PCM1808
#define I2S_RX_WS    12   // Word Select → Vai para o pino LRCK do PCM1808
#define I2S_RX_DIN   13   // Data In    → Recebe do pino DOUT do PCM1808

// ============================================================
// CONFIGURAÇÃO DE ÁUDIO E REDE
// ============================================================
#define SAMPLE_RATE   44100      // Qualidade de CD (44.1 kHz)
#define BUFFER_SAMPLES 128       // Quantidade de amostras por pacote de áudio (reduz o atraso)
#define VBAN_PORT      6980      // Porta padrão do protocolo VBAN (Voicemeeter)
#define VBAN_HEADER_SZ 28        // Tamanho exato em bytes do cabeçalho VBAN

// ============================================================
// INICIALIZAÇÃO DE OBJETOS
// ============================================================
Adafruit_SSD1306 display(128, 64, &Wire, -1); // Instancia o OLED com tamanho 128x64
WebServer server(80);                         // Servidor web rodando na porta 80 (HTTP padrão)
Preferences preferences;                      // Acesso à memória não-volátil (tipo pen drive)
WiFiUDP udp;                                  // Instancia a comunicação UDP para o áudio

// ============================================================
// VARIÁVEIS GLOBAIS
// ============================================================
String ssid     = "";           // Guarda o nome da rede Wi-Fi
String password = "";           // Guarda a senha do Wi-Fi
String pc_ip    = "";           // Guarda o endereço IP do PC com o Voicemeeter
bool isSetupMode = false;       // Define se o ESP32 está no modo configuração ou live

int16_t buffer_in [BUFFER_SAMPLES * 2];   // Array (buffer) para guardar o áudio capturado da mesa
int16_t buffer_out[BUFFER_SAMPLES * 2];   // Array (buffer) para guardar o áudio vindo do PC
int vu_in  = 0;                           // Variável para a barra gráfica de volume de entrada
int vu_out = 0;                           // Variável para a barra gráfica de volume de saída

// Estrutura que monta o cabeçalho exigido pelo Voicemeeter (Protocolo VBAN)
struct vban_header_t {
  char     vban[4]        = {'V', 'B', 'A', 'N'}; // Assinatura obrigatória
  uint8_t  format_sr      = 0x12;                 // Define a taxa de 44100 Hz
  uint8_t  format_nbs     = 63;                   // Avisa que enviamos 64 pacotes
  uint8_t  format_nbc     = 1;                    // Avisa que é áudio Estéreo (2 canais)
  uint8_t  format_bit     = 1;                    // Avisa que a resolução é 16-bits
  char     stream_name[16]= "Stream1";            // NOME DO STREAM (Tem que ser igual no Voicemeeter)
  uint32_t frame_counter  = 0;                    // Contador para organizar os pacotes
} vban_header;

// ============================================================
// PÁGINAS WEB (HTML)
// ============================================================
// Página inicial (quando o Wi-Fi não está configurado)
const char* html_setup = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Configuração de Áudio</title>
<style>
  body{font-family:Arial;padding:20px;background:#222;color:#fff;}
  select,input{width:100%;padding:10px;margin:5px 0 15px;border-radius:5px;border:none;}
  button{background:#007BFF;color:#fff;padding:15px;border:none;width:100%;
         font-size:16px;border-radius:5px;cursor:pointer;}
</style></head>
<body><h2>Configurar ESP32 Áudio</h2>
<form action="/save" method="POST">
  <label>Selecione o Wi-Fi:</label>
  <select name="ssid" required>
    <option value="">Escaneando redes...</option>
    %OPTIONS%
  </select>
  <label>Senha do Wi-Fi:</label>
  <input type="password" name="pass" placeholder="Deixe em branco se for rede aberta">
  <label>IP do PC (Voicemeeter / VBAN):</label>
  <input type="text" name="pcip" placeholder="Ex: 192.168.1.10" required>
  <button type="submit">Salvar e Reiniciar</button>
</form></body></html>
)rawliteral";

// Página de Monitoramento (Dashboard) acessível pelo IP na rede local
const char* html_dashboard = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Dashboard</title><meta http-equiv="refresh" content="3">
<style>
  body{font-family:Arial;text-align:center;background:#111;color:#0f0;}
  .box{border:1px solid #0f0;padding:20px;margin:10px;border-radius:10px;}
</style></head>
<body><h2>Status da Live</h2>
<div class="box"><h3>IP: %IP%</h3></div>
<div class="box"><h3>Rede: %SSID% | Sinal: %RSSI% dBm</h3></div>
<div class="box"><h3>Data/Hora: %TIME%</h3></div>
<div class="box"><h3>Line-In (Mesa): %VU_IN% %</h3></div>
<div class="box"><h3>Line-Out (PC→ESP): %VU_OUT% %</h3></div>
<a href="/reset" style="color:red;">Resetar Configurações</a>
</body></html>
)rawliteral";

// ============================================================
// FUNÇÕES DO SERVIDOR WEB
// ============================================================
void handleRoot() {
  if (isSetupMode) {
    // Se não tem internet salva, mostra a tela de conectar o Wi-Fi
    String html = html_setup;
    String options = "";
    int n = WiFi.scanNetworks(); // Escaneia roteadores ao redor
    if (n == 0) {
      options = "<option value=''>Nenhuma rede encontrada</option>";
    } else {
      for (int i = 0; i < n; i++) { // Cria lista de Wi-Fis no dropdown HTML
        options += "<option value='" + WiFi.SSID(i) + "'>"
                +  WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
      }
    }
    html.replace("%OPTIONS%", options); // Troca o marcador %OPTIONS% pelas redes reais
    server.send(200, "text/html", html);
  } else {
    // Se está em modo Live, mostra o painel de métricas
    String html = html_dashboard;
    html.replace("%IP%",   WiFi.localIP().toString());
    html.replace("%SSID%", ssid);
    html.replace("%RSSI%", String(WiFi.RSSI()));
    
    // Configura e puxa as informações de Data e Hora
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[32];
      sprintf(timeStr, "%02d:%02d:%02d - %02d/%02d/%04d",
              timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
              timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      html.replace("%TIME%", String(timeStr));
    } else {
      html.replace("%TIME%", "Sincronizando...");
    }

    html.replace("%VU_IN%",  String(vu_in));
    html.replace("%VU_OUT%", String(vu_out));
    server.send(200, "text/html", html); // Envia o HTML pronto para o navegador
  }
}

// Salva os dados inseridos na página web dentro da memória Flash
void handleSave() {
  preferences.begin("audio_cfg", false);
  preferences.putString("ssid", server.arg("ssid"));
  preferences.putString("pass", server.arg("pass"));
  preferences.putString("pcip", server.arg("pcip"));
  preferences.end();
  server.send(200, "text/html", "<h2>Salvo! Reiniciando...</h2>");
  delay(2000);
  ESP.restart(); // Reinicia o dispositivo para aplicar as mudanças
}

// Apaga totalmente a memória e reseta o Wi-Fi
void handleReset() {
  preferences.begin("audio_cfg", false);
  preferences.clear();
  preferences.end();
  server.send(200, "text/html", "<h2>Resetado! Reiniciando...</h2>");
  delay(2000);
  ESP.restart();
}

// Reinicia o aparelho em caso de queda de Wi-Fi SEM apagar a senha salva
void restartOnly() {
  delay(2000);
  ESP.restart();
}

// ============================================================
// FUNÇÕES DE INICIALIZAÇÃO DE ÁUDIO I2S
// ============================================================
// Configura o I2S para Envio (Toca o som do PC no DAC PCM5102A)
void initI2S_TX() {
  i2s_config_t cfg = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT, // Estéreo
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 64,
    .use_apll             = true,   // Ativa relógio de precisão (evita chiado audiófilo)
    .tx_desc_auto_clear   = true,   // Limpa restos de áudio para evitar estalos
    .fixed_mclk           = 0
  };
  i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);

  // Mapeia os pinos da saída de áudio
  i2s_pin_config_t pins = {
    .bck_io_num   = I2S_TX_BCK,
    .ws_io_num    = I2S_TX_WS,
    .data_out_num = I2S_TX_DOUT,
    .data_in_num  = I2S_PIN_NO_CHANGE // Ignora o pino de entrada neste canal
  };
  i2s_set_pin(I2S_NUM_0, &pins);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

// Configura o I2S para Recepção (Captura o som da mesa via ADC PCM1808)
void initI2S_RX() {
  i2s_config_t cfg = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 64,
    .use_apll             = true,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };
  i2s_driver_install(I2S_NUM_1, &cfg, 0, NULL);

  // Mapeia os pinos da entrada de áudio
  i2s_pin_config_t pins = {
    .bck_io_num   = I2S_RX_BCK,
    .ws_io_num    = I2S_RX_WS,
    .data_out_num = I2S_PIN_NO_CHANGE, // Ignora o pino de saída neste canal
    .data_in_num  = I2S_RX_DIN
  };
  i2s_set_pin(I2S_NUM_1, &pins);
}

// ============================================================
// ATUALIZAÇÃO DO DISPLAY OLED
// ============================================================
void updateOLED() {
  display.clearDisplay();
  
  // Linha 0: Exibe IP abreviado e Relógio
  display.setCursor(0, 0);
  String ipStr = WiFi.localIP().toString();
  display.print(ipStr.length() > 13 ? ipStr.substring(0, 13) : ipStr);

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    display.setCursor(80, 0);
    display.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  }

  // Linha 1: Nome da Rede e Força do Sinal (RSSI)
  display.setCursor(0, 10);
  String ssidShort = ssid.length() > 10 ? ssid.substring(0, 10) : ssid;
  display.print(ssidShort);
  display.print(" | "); display.print(WiFi.RSSI()); display.print("dB");
  
  // Gráfico da barra de volume: Entrada (Áudio da Mesa)
  display.setCursor(0, 25); display.print("IN: ");
  display.drawRect(28, 25, 98, 8, WHITE); // Borda da barra
  display.fillRect(28, 25, map(vu_in, 0, 100, 0, 98), 8, WHITE); // Preenchimento da barra

  // Gráfico da barra de volume: Saída (Áudio vindo do PC)
  display.setCursor(0, 40); display.print("OUT:");
  display.drawRect(28, 40, 98, 8, WHITE); // Borda da barra
  display.fillRect(28, 40, map(vu_out, 0, 100, 0, 98), 8, WHITE); // Preenchimento da barra

  display.display(); // Envia os desenhos para o hardware da tela
}

// ============================================================
// SETUP (Configurações iniciais ao ligar)
// ============================================================
void setup() {
  Serial.begin(115200);

  // Inicializa o OLED via comunicação I2C
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    for (;;);
  }
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay(); // Evita tela chuviscada ao ligar
  display.display();

  // Carrega as senhas de Wi-Fi guardadas da memória interna
  preferences.begin("audio_cfg", true);
  ssid     = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");
  pc_ip    = preferences.getString("pcip", "");
  preferences.end();
  
  if (ssid == "") {
    // ── MODO CONFIGURAÇÃO (Access Point) ─────────────────────
    // O ESP32 vira um "Roteador" para você se conectar nele
    isSetupMode = true;
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("ESP32_Audio_Setup");

    display.clearDisplay();
    display.setCursor(0, 0);  display.println("MODO CONFIGURACAO");
    display.setCursor(0, 16); display.println("Wi-Fi: ESP32_Audio_Setup");
    display.setCursor(0, 32); display.println("IP:    192.168.4.1");
    display.display();
  } else {
    // ── MODO LIVE (Station) ──────────────────────────────────
    // O ESP32 tenta conectar no Wi-Fi da Igreja/Estúdio
    WiFi.begin(ssid.c_str(), password.c_str());

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Conectando Wi-Fi...");
    display.display();

    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 20) {
      delay(500); // Tenta por 10 segundos
      tries++;
    }

    // Caso de falha (Se a internet caiu ou mudaram a senha)
    if (WiFi.status() != WL_CONNECTED) {
      display.clearDisplay();
      display.setCursor(0, 0); display.println("Wi-Fi falhou!");
      display.setCursor(0, 16); display.println("Reiniciando...");
      display.display();
      restartOnly(); // Reinicia sem apagar a senha e tenta novamente
    }

    // Configura fuso horário (Brasília UTC-3) para o relógio OLED
    configTime(-10800, 0, "pool.ntp.br");
    udp.begin(VBAN_PORT); // Abre as portas UDP para o áudio

    initI2S_TX(); // Prepara o PCM5102A (Saída de áudio)
    initI2S_RX(); // Prepara o PCM1808 (Entrada de áudio)

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Online!");
    display.println(WiFi.localIP().toString());
    display.display();
  }

  // Rotas que o usuário acessa pelo navegador
  server.on("/",      handleRoot);
  server.on("/save",  HTTP_POST, handleSave);
  server.on("/reset", handleReset);
  server.begin(); // Ativa as páginas de internet
  Serial.println("Servidor web iniciado.");
}

// ============================================================
// LOOP PRINCIPAL (Gera o fluxo de áudio contínuo)
// ============================================================
void loop() {
  server.handleClient(); // Mantém o servidor web ouvindo o navegador

  if (!isSetupMode) { // Só trabalha o áudio se o Wi-Fi estiver ok
    size_t bytes_read, bytes_written;

    // 1. CAPTURA E ENVIO (Mesa de som → ESP32 → OBS/Voicemeeter)
    // Lê o áudio cru (PCM) entrando pelo PCM1808
    i2s_read(I2S_NUM_1, buffer_in, sizeof(buffer_in), &bytes_read, portMAX_DELAY);
    vu_in = map(abs(buffer_in[0]), 0, 25000, 0, 100); // Calcula volume para o OLED

    vban_header.frame_counter++; // Sinaliza para o PC a ordem do pacote
    udp.beginPacket(pc_ip.c_str(), VBAN_PORT); // Abre conexão com o IP do computador
    udp.write((uint8_t*)&vban_header, sizeof(vban_header)); // Envia o cabeçalho VBAN
    udp.write((uint8_t*)buffer_in, bytes_read); // Anexa o áudio gravado e despacha
    udp.endPacket();
    
    // 2. RECEPÇÃO E REPRODUÇÃO (PC/Música → ESP32 → Mesa de som)
    int pktSize = udp.parsePacket(); // Verifica se chegou som do computador
    if (pktSize > VBAN_HEADER_SZ) {
      uint8_t hdr_discard[VBAN_HEADER_SZ];
      
      // Lê o cabeçalho recebido do PC e descarta (isolamento do payload)
      udp.read(hdr_discard, VBAN_HEADER_SZ);
      // Extrai apenas a música (Payload)
      int payloadBytes = udp.read((uint8_t*)buffer_out, sizeof(buffer_out));
      
      // Envia os dados digitais para o chip PCM5102A tocar
      i2s_write(I2S_NUM_0, buffer_out, payloadBytes, &bytes_written, 0);
      vu_out = map(abs(buffer_out[0]), 0, 25000, 0, 100); // Calcula volume
    } else {
      vu_out = 0; // Zera barra gráfica se não houver música
    }

    // 3. Atualiza os gráficos do OLED
    static unsigned long lastOLED = 0;
    if (millis() - lastOLED > 1000) { // Trava o ecrã a 1 FPS para não travar o som
      updateOLED();
      lastOLED = millis();
    }
  }
}