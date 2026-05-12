#include "consts.h"
#include <Preferences.h>
Preferences preferences;


void setRGBLed(int r, int g, int b, int l) {
  strip.clear();
  strip.setBrightness(l);
  strip.setPixelColor(0, r, g, b);
  strip.show();
}

void setRGBLed(int r, int g, int b) {
  setRGBLed(r, g, b, 5);
}

void setRGBLed(int c, int l) {
  strip.clear();
  strip.setBrightness(l);
  strip.setPixelColor(0, c);
  strip.show();
}

void setRGBLed(int c) {
  setRGBLed(c, 5);
}

void setRGBLed() {
  strip.clear();
  strip.show();
}

int kToRGB(int kTemp) {
  int r;
  int g;
  int b;

  kTemp = kTemp / 100;

  if (kTemp <= 66) {
    r = 255;
  } else {
    r = kTemp - 60;
    r = 329.698727466 * pow(r, -0.1332047592);
    if (r < 0) {
      r = 0;
    }
    if (r > 255) {
      r = 255;
    }
  }

  if (kTemp <= 66) {
    g = kTemp;
    g = 99.4708025861 * log(g) - 161.1195681661;
    if (g < 0) {
      g = 0;
    }
    if (g > 255) {
      g = 255;
    }
  } else {
    g = kTemp - 60;
    g = 288.1221695283 * pow(g, -0.0755148492);
    if (g < 0) {
      g = 0;
    }
    if (g > 255) {
      g = 255;
    }
  }

  if (kTemp >= 66) {
    b = 255;
  } else {
    if (kTemp <= 19) {
      b = 0;
    } else {
      b = kTemp - 10;
      b = 138.5177312231 * log(b) - 305.0447927307;
      if (b < 0) {
        b = 0;
      }
      if (b > 255) {
        b = 255;
      }
    }
  }

  int rgb = r * 0x10000 + g * 0x100 + b;
  return rgb;
}

double julianDate(int y, int m, int d) {
  int mm, yy;
  double k1, k2, k3;
  double j;
  yy = y - int((12 - m) / 10);
  mm = m + 9;
  if (mm >= 12) {
    mm = mm - 12;
  }
  k1 = 365.25 * (yy + 4172);
  k2 = int((30.6001 * mm) + 0.5);
  k3 = int((((yy / 100) + 4) * 0.75) - 38);
  j = k1 + k2 + d + 59;
  j = j - k3;
  return j;
}

int moonPhase() {
  struct tm tts = *localtime(&rawtime);
  double jd = 0;
  double ed = 0;
  int b = 0;
  jd = julianDate(tts.tm_year + 1900, tts.tm_mon, tts.tm_mday);
  jd = int(jd - 2244116.75);  // start at Jan 1 1972
  jd /= 29.53;                // divide by the moon cycle
  b = jd;
  jd -= b;          // leaves the fractional part of jd
  ed = jd * 29.53;  // days elapsed this month
  b = jd * 8 + 0.5;
  b = b & 7;
  return b;
}

String weekday(int weekday) {
  switch (weekday) {
    case 0: return "Domingo";
    case 1: return "Segunda";
    case 2: return "Terca  ";
    case 3: return "Quarta ";
    case 4: return "Quinta ";
    case 5: return "Sexta  ";
    case 6: return "Sabado ";
    default: return"Que dia";
  }
}

// Função para traduzir os códigos de status WiFi
void printWiFiStatus(wl_status_t status) {
  Serial.print("\nStatus WiFi: ");
  switch (status) {
    case WL_IDLE_STATUS:
      Serial.println("WL_IDLE_STATUS - Em espera");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("WL_NO_SSID_AVAIL - Rede não encontrada");
      Serial.println("Verifique se o SSID está correto e se a rede está disponível");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("WL_SCAN_COMPLETED - Varredura de redes concluída");
      break;
    case WL_CONNECTED:
      Serial.println("WL_CONNECTED - Conectado");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("WL_CONNECT_FAILED - Falha na conexão");
      Serial.println("Verifique a senha da rede");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("WL_CONNECTION_LOST - Conexão perdida");
      break;
    case WL_DISCONNECTED:
      Serial.println("WL_DISCONNECTED - Desconectado");
      break;
    case WL_NO_SHIELD:
      Serial.println("WL_NO_SHIELD - Módulo WiFi não encontrado");
      break;
    default:
      Serial.print("Código desconhecido: ");
      Serial.println(status);
      break;
  }
}

// Adicione também esta função para verificar a força do sinal
void checkWiFiSignal() {
  if (WiFi.status() == WL_CONNECTED) {
    long rssi = WiFi.RSSI();
    Serial.print("Força do sinal: ");
    Serial.print(rssi);
    Serial.println(" dBm");

    if (rssi > -50) {
      Serial.println("Sinal excelente");
    } else if (rssi > -60) {
      Serial.println("Sinal muito bom");
    } else if (rssi > -70) {
      Serial.println("Sinal bom");
    } else if (rssi > -80) {
      Serial.println("Sinal fraco");
    } else {
      Serial.println("Sinal muito fraco");
    }
  }
}

void initializeOTA() {
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
    display.clearDisplay();
    display.setTextSize(7);
    display.setCursor(0, 0);
    display.print("OTA");
    display.display();
    otaUpgrade = true;
  });
  ArduinoOTA.onEnd([]() {
    setRGBLed();

    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    setRGBLed(255 - (progress / (total / 255.0)), (progress / (total / 255.0)), 0);
    Serial.printf("Progresso: %u%%\r", (progress / (total / 100)));
    display.clearDisplay();
    display.setTextSize(7);
    display.setCursor(0, 0);
    display.println("OTA");
    display.setTextSize(1);
    display.print((float)progress / (float)total * 100);
    display.print("%");
    display.display();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    setRGBLed(255, 0, 0, 255);
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
    setRGBLed();
  });
  ArduinoOTA.begin();
}

bool syncTimeWithNTP() {
  if (!ntpInitialized || WiFi.status() != WL_CONNECTED) {
    Serial.println("NTP não disponível - WiFi desconectado ou não inicializado");
    return false;
  }

  Serial.println("Sincronizando com NTP...");

  if (timeClient.forceUpdate()) {
    unsigned long epochTime = timeClient.getEpochTime();

    if (rtcAvailable) {
      rtc.adjust(DateTime(epochTime));
      Serial.println("RTC sincronizado com NTP!");

      // Atualizar a variável rawtime
      rawtime = epochTime;

      // Feedback de sincronização bem-sucedida
      setRGBLed(0, 255, 0, 30);  // Verde
      delay(500);
      setRGBLed(0, 0, 0);

      return true;
    } else {
      Serial.println("RTC não disponível para sincronização");
      return false;
    }
  } else {
    Serial.println("Falha na sincronização NTP!");

    // Feedback de erro
    setRGBLed(255, 0, 0, 30);  // Vermelho
    delay(500);
    setRGBLed(0, 0, 0);

    return false;
  }
}

void connectToWiFi() {
  Serial.print("Conectando ao WiFi ");
  Serial.println(STASSID);

  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP-A11V1R15");
  WiFi.begin(STASSID, STAPSK);

  // Tentativas de conexão com timeout
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;

    // Feedback visual com LED durante a tentativa de conexão
    setRGBLed(0, 0, 255);  // Azul para indicar tentativa de conexão
    delay(100);
    setRGBLed(0, 0, 0);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado com sucesso!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Inicializar cliente NTP se não foi inicializado
    if (!ntpInitialized) {
      timeClient.begin();
      timeClient.setTimeOffset(-3 * 3600);  // UTC-3 para Brasil
      ntpInitialized = true;
      Serial.println("Cliente NTP inicializado");
    }

    if (rtc.now().year() < 2025) {
      syncTimeWithNTP();
    }

    // LED verde para indicar conexão bem-sucedida
    setRGBLed(0, 255, 0);
    delay(1000);
    setRGBLed(0, 0, 0);

    // Configurações de segurança
    clientS->setFingerprint(fingerprint_upload_video_google_com);
    clientS->setTrustAnchors(&cert);
  } else {
    Serial.println("\nFalha na conexão WiFi!");
    printWiFiStatus(WiFi.status());

    // LED vermelho piscante para indicar falha
    for (int i = 0; i < 5; i++) {
      setRGBLed(255, 0, 0);
      delay(300);
      setRGBLed(0, 0, 0);
      delay(300);
    }
    Serial.println("Continuando sem WiFi...");
    // Não reinicia - continua operação sem WiFi
  }
}

unsigned long timerEnd = 0;
bool timerActive = false;

void startTimer(unsigned long durationMs) {
  timerEnd = millis() + durationMs;
  timerActive = true;
}

void stopTimer() {
  timerActive = false;
}

struct AlarmStep {
	uint16_t freq;      // frequência da nota
	uint16_t duration;  // duração da nota em ms
	uint8_t r, g, b;    // cor do LED
	uint16_t pause;     // pausa após a nota em ms
};

AlarmStep alarmTune[] = {
	{4435, 100, 255,   0,   0, 150}, // C#8 - vermelho
	{4435, 100,   0, 255,   0, 150}, // C#8 - verde
	{3951, 100,  84,  33,  96,  25}, // B7  - roxo
	{1318, 100,   0,   0, 255, 150}, // E6  - azul
	{5274, 100, 255, 255,   0, 150}, // E8  - amarelo
	{4435, 100, 198,  56, 107,   0}  // C#8 - magenta
};

void playAlarm() {
	for (auto &step : alarmTune) {
		tone(BUZZER_PIN, step.freq, step.duration);
		setRGBLed(step.r, step.g, step.b);
		delay(step.duration);
		setRGBLed();
		delay(step.pause);
	}
}

void updateTimer() {
  if (timerActive && millis() >= timerEnd) {
    timerActive = false;
    // Disparar alarme
    playAlarm();
  }
}

struct Holiday {
  int day;
  int month;
};

Holiday holidays[] = {
  {1, 1},   // Confraternização Universal
  {21, 4},  // Tiradentes
  {1, 5},   // Dia do Trabalho
  {7, 9},   // Independência
  {12, 10}, // Nossa Senhora Aparecida
  {2, 11},  // Finados
  {15, 11}, // Proclamação da República
  {20, 11}, // Consciência Negra
  {25, 12}, // Natal
  
  {19, 3},  // São José
  {25, 3},  // Carta Magna do Ceará

  {29, 6},  // São Pedro
  {29, 9},  // Aniversário de Camocim
  
  {28, 10}, // Servidor Público
};

bool isHoliday(int day, int month) {
  for (auto &h : holidays) {
    if (h.day == day && h.month == month) {
      return true;
    }
  }
  return false;
}

#define setTimezone(tz) setenv("TZ", tz, 1); tzset()