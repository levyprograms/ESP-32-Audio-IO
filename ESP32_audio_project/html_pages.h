#ifndef HTML_PAGES_H
#define HTML_PAGES_H

#define VERSAO_LIB 1.0

// ============================================================
// DECLARAÇÃO DE VARIÁVEIS EXTERNAS (Vêm do ficheiro principal)
// ============================================================
extern bool isSetupMode;
extern OperationMode currentMode; // Correção: alterado de 'int' para 'OperationMode'
extern int connected_apps;
extern int vu_in;
extern int vu_out;
extern WebServer webServer;
extern Preferences preferences;


// ============================================================
// PÁGINAS WEB (HTML)
// ============================================================

const char* html_setup = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Harpyja.Tech - Setup</title>
  <link rel="icon" type="image/png" href="https://raw.githubusercontent.com/HarpyjaTech/ESP-32-Audio-IO/BT_version/ESP32_audio_project/logo.png">
  <style>
    :root {
      --bg-color: #000000;
      --card-bg: #121212;
      --primary: #c38b38;
      --primary-hover: #a3722b;
      --text-main: #ffffff;
      --text-muted: #888888;
      --input-bg: #1e1e1e;
      --border-color: #333333;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
      background-color: var(--bg-color);
      color: var(--text-main);
      display: flex; justify-content: center; align-items: center;
      min-height: 100vh; padding: 20px;
    }
    .card {
      background: var(--card-bg); width: 100%; max-width: 400px;
      padding: 40px 30px; border-radius: 12px;
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.8);
      border: 1px solid var(--border-color); text-align: center;
    }
    .logo-container { margin-bottom: 25px; background: rgba(0, 0, 0, 0)}
    .logo-container img { max-width: 220px; height: auto; }
    .header { margin-bottom: 30px; }
    .header p { color: var(--text-muted); font-size: 15px; letter-spacing: 1px; text-transform: uppercase;}
    .input-group { margin-bottom: 20px; text-align: left; }
    label {
      display: block; font-size: 12px; font-weight: 600; color: var(--text-muted);
      margin-bottom: 8px; text-transform: uppercase; letter-spacing: 0.5px;
    }
    select, input {
      width: 100%; padding: 14px; background: var(--input-bg); color: var(--text-main);
      border: 1px solid var(--border-color); border-radius: 8px; font-size: 16px;
      transition: all 0.3s ease; appearance: none; -webkit-appearance: none;
    }
    select {
      background-image: url("data:image/svg+xml;charset=US-ASCII");
      background-repeat: no-repeat; background-position: right 14px top 50%; background-size: 12px auto;
    }
    select:focus, input:focus { outline: none; border-color: var(--primary); box-shadow: 0 0 0 2px rgba(195, 139, 56, 0.3); }
    button {
      width: 100%; padding: 14px; margin-top: 10px;
      background: linear-gradient(135deg, var(--primary), var(--primary-hover));
      color: white; font-size: 16px; font-weight: bold; border: none; border-radius: 8px; cursor: pointer;
      transition: transform 0.2s, box-shadow 0.2s;
    }
    button:hover { transform: translateY(-2px); box-shadow: 0 6px 15px rgba(195, 139, 56, 0.4); }
  </style>
</head>
<body>
  <div class="card">
    <div class="logo-container">
      <!-- COLE O BASE64 DA IMAGEM AQUI -->
      <img src="https://raw.githubusercontent.com/HarpyjaTech/ESP-32-Audio-IO/BT_version/ESP32_audio_project/logo.png" alt="Harpyja.Tech">
    </div>
    <div class="header">
      <p>Configuração de Rede</p>
    </div>
    <form action="/save" method="POST">
      <div class="input-group">
        <label>Wi-Fi Local</label>
        <select name="ssid" required>
          <option value="" disabled selected>Escaneando redes...</option>
          %OPTIONS%
        </select>
      </div>
      <div class="input-group">
        <label>Senha do Wi-Fi</label>
        <input type="password" name="pass" placeholder="Deixe em branco se for rede aberta">
      </div>
      <button type="submit">Salvar e Iniciar Servidor</button>
    </form>
  </div>
</body>
</html>
)rawliteral";

const char* html_dashboard = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Harpyja.Tech - Dashboard</title>
  <link rel="icon" type="image/png" href="https://raw.githubusercontent.com/HarpyjaTech/ESP-32-Audio-IO/BT_version/ESP32_audio_project/logo.png">
  <meta http-equiv="refresh" content="3">
  <style>
    :root {
      --bg-color: #000000;
      --card-bg: #121212;
      --primary: #c38b38;
      --danger: #dc3545;
      --text-main: #ffffff;
      --text-muted: #888888;
      --box-bg: #1e1e1e;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
      background-color: var(--bg-color); color: var(--text-main);
      display: flex; justify-content: center; align-items: center;
      min-height: 100vh; padding: 20px;
    }
    .card {
      background: var(--card-bg); width: 100%; max-width: 450px;
      padding: 30px; border-radius: 12px; box-shadow: 0 10px 30px rgba(0, 0, 0, 0.8);
      text-align: center; border: 1px solid #333;
    }
    .logo-container { margin-bottom: 20px; background: rgba(0, 0, 0, 0)}
    .logo-container img { max-width: 220px; height: auto; display: inline-block; }
    .status-badge {
      display: inline-block; background: rgba(0, 0, 0, 0); color: var(--primary);
      padding: 6px 18px; border-radius: 20px; font-size: 13px; font-weight: bold;
      letter-spacing: 1px; margin-bottom: 25px; border: 1px solid rgba(195, 139, 56, 0.4);
    }
    .grid { display: grid; grid-template-columns: 1fr; gap: 12px; margin-bottom: 20px; }
    .box {
      background: var(--box-bg); padding: 15px; border-radius: 8px; text-align: left;
      border-left: 4px solid var(--primary);
    }
    .box-title { font-size: 11px; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: 5px; }
    .box-value { font-size: 16px; font-weight: bold; }
    
    /* VU METERS */
    .vu-container { background: var(--box-bg); padding: 15px; border-radius: 8px; margin-bottom: 12px; text-align: left; }
    .vu-header { display: flex; justify-content: space-between; font-size: 12px; color: var(--text-muted); text-transform: uppercase; font-weight: bold; margin-bottom: 8px; }
    .vu-bar-bg { width: 100%; height: 12px; background: #000; border-radius: 6px; overflow: hidden; }
    .vu-bar-fill { height: 100%; background: linear-gradient(90deg, #28a745 0%, #ffc107 75%, #dc3545 100%); transition: width 0.3s ease; }
    
    .btn-reset {
      display: block; width: 100%; padding: 14px; margin-top: 20px; background: transparent;
      color: var(--danger); font-size: 15px; font-weight: bold; text-decoration: none;
      border: 1px solid var(--danger); border-radius: 8px; transition: all 0.3s ease;
    }
    .btn-reset:hover { background: var(--danger); color: #fff; box-shadow: 0 4px 15px rgba(220, 53, 69, 0.4); }
  </style>
</head>
<body>
  <div class="card">
    <div class="logo-container">
      <!-- COLE O BASE64 DA IMAGEM AQUI -->
      <img src="https://raw.githubusercontent.com/HarpyjaTech/ESP-32-Audio-IO/BT_version/ESP32_audio_project/logo.png" alt="Harpyja.Tech">
    </div>

    <div class="status-badge">MODO %MODO%</div>
    
    <div class="grid">
      <div class="box">
        <div class="box-title">IP Servidor (Porta 7000)</div>
        <div class="box-value">%IP%</div>
      </div>
      <div class="box">
        <div class="box-title">Apps Conectados</div>
        <div class="box-value">%CLIENTS% / 4</div>
      </div>
      <div class="box">
        <div class="box-title">Data / Hora</div>
        <div class="box-value">%TIME%</div>
      </div>
    </div>

    <div class="vu-container">
      <div class="vu-header"><span>Line-In (Mesa)</span><span>%VU_IN%%</span></div>
      <div class="vu-bar-bg"><div class="vu-bar-fill" style="width: %VU_IN%%;"></div></div>
    </div>

    <div class="vu-container">
      <div class="vu-header"><span>Line-Out (Saída)</span><span>%VU_OUT%%</span></div>
      <div class="vu-bar-bg"><div class="vu-bar-fill" style="width: %VU_OUT%%;"></div></div>
    </div>

    <a href="/reset" class="btn-reset">Resetar Wi-Fi</a>
  </div>
</body>
</html>
)rawliteral";


// ============================================================
// FUNÇÕES DO SERVIDOR WEB
// ============================================================
void handleRoot() {
  if (isSetupMode) {
    String html = html_setup;
    String options = "";
    int n = WiFi.scanNetworks();
    if (n == 0) {
      options = "<option value=''>Nenhuma rede encontrada</option>";
    } else {
      for (int i = 0; i < n; i++) {
        options += "<option value='" + WiFi.SSID(i) + "'>"
                +  WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
      }
    }
    html.replace("%OPTIONS%", options);
    webServer.send(200, "text/html", html);
  } else {
    String html = html_dashboard;

    String modoStr;
    // Correção: Agora comparamos com os nomes do enum em vez de números soltos
    if (currentMode == MODE_SERVER) modoStr = "SERVIDOR";
    else if (currentMode == MODE_BLUETOOTH) modoStr = "BLUETOOTH";
    else modoStr = "AUXILIAR";
    html.replace("%MODO%", modoStr);

    html.replace("%IP%",   WiFi.localIP().toString());
    html.replace("%CLIENTS%", String(connected_apps));

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[32];
      sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      html.replace("%TIME%", String(timeStr));
    } else {
      html.replace("%TIME%", "Sincronizando...");
    }

    html.replace("%VU_IN%",  String(vu_in));
    html.replace("%VU_OUT%", String(vu_out));
    webServer.send(200, "text/html", html);
  }
}

void handleSave() {
  preferences.begin("audio_cfg", false);
  preferences.putString("ssid", webServer.arg("ssid"));
  preferences.putString("pass", webServer.arg("pass"));
  preferences.end();
  webServer.send(200, "text/html", "<h2>Salvo! Reiniciando Servidor...</h2>");
  delay(2000);
  ESP.restart();
}

void handleReset() {
  preferences.begin("audio_cfg", false);
  preferences.clear();
  preferences.end();
  webServer.send(200, "text/html", "<h2>Resetado! Reiniciando...</h2>");
  delay(2000);
  ESP.restart();
}

#endif // HTML_PAGES_H