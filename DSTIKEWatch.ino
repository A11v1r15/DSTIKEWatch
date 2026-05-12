#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include "RTClib.h"
#include "secrets.h"  // STASSID STAPSK OTA_PASSWORD
#include "faces.h"

Face* currentFace = new MainFace();

void setup() {
  // Inicializar pinos
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(RGB_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);

  digitalWrite(WHITE_LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

  // Inicializar comunicação serial
  Serial.begin(115200);
  Serial.println("Iniciando teste de componentes...");
  delay(1000);

  // Inicializar I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Inicializar LED WS2812B
  strip.begin();
  setRGBLed(255, 255, 255);

  // Inicializar display OLED
  display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  if (display.begin(SCREEN_I2C, true)) {
    displayAvailable = true;
    display.display();
    delay(1000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setContrast(0);
    Serial.println("Display OLED inicializado com sucesso!");
  } else {
    Serial.println("Falha ao inicializar o display OLED!");
    // Piscar LED em vermelho para indicar erro no display
    for (int i = 0; i < 5; i++) {
      setRGBLed(255, 0, 0);
      delay(200);
      setRGBLed(0, 0, 0);
      delay(200);
    }
  }

  // Inicializar RTC
  if (rtc.begin()) {
    rtcAvailable = true;
    Serial.println("RTC encontrado e inicializado!");
  } else {
    Serial.println("Não foi possível encontrar o RTC!");
  }

  // Initialize OTA updates
  initializeOTA();

  // Configurar o fuso horário para Brasil
  configTime(-3 * 3600, 0, "pool.ntp.org");  // UTC-3 (Brasília)

  httpUpdater.setup(&server, "/ota", "admin", OTA_PASSWORD);  // usuário/senha

  // Inicializar navegação
  currentFace  //  MainFace
    ->addNext(new CalendarFace())
    ->addNext(new TimerFace())
    ->addNext(new SnakeFace())
    ->addNext(new WiFiFace())
    ->addNext(new LedFace())
    ->addNext(currentFace);

  setRGBLed();
}

void loop() {
  // Sempre chame o handle do OTA primeiro
  ArduinoOTA.handle();

  // Se estiver em modo OTA, não execute o resto do código
  if (otaUpgrade) {
    return;
  }

  // Atualizar o tempo Unix
  time(&rawtime);

  // Alimentar o watchdog regularmente
  ESP.wdtFeed();

  currentFace->show();
  currentFace->update();

  // Tratar eventos de botões
  bool currentUp = !digitalRead(BUTTON_UP);
  bool currentDown = !digitalRead(BUTTON_DOWN);
  bool currentSelect = !digitalRead(BUTTON_SELECT);

  if (currentUp) {
    currentFace = currentFace->handleUp(currentUp && upPressed);
  } else if (currentDown) {
    currentFace = currentFace->handleDown(currentDown && downPressed);
  } else if (currentSelect) {
    currentFace->action(currentSelect && selectPressed);
  }

  upPressed = currentUp;
  downPressed = currentDown;
  selectPressed = currentSelect;


  // Atualizar timer global
  updateTimer();

  // Pequeno delay para evitar flickering
  delay(100);
}