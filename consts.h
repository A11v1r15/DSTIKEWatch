#include <WiFiUdp.h>
#include <NTPClient.h>
#include "certs.h"
#include "RussoOne18pt4b.h"
#include "MoonPhases7x7.h"
#include "SnakeII.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Configuração do RTC
RTC_DS3231 rtc;

// Configuração dos pinos
#define BUZZER_PIN 0
#define CON_TX 1
#define CON_RX 3
#define I2C_SCL 4
#define I2C_SDA 5
#define BUTTON_UP 12
#define BUTTON_DOWN 13
#define BUTTON_SELECT 14
#define RGB_LED_PIN 15
#define WHITE_LED_PIN 16

// Configuração do LED WS2812B
#define NUM_LEDS 1
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

// Configuração do display SH1106 1.3" OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configuração i2c
#define SCREEN_I2C 0x3C

X509List cert(cert_GTS_Root_R1);

// HTTPClient instance
HTTPClient http;
WiFiClient client;
std::unique_ptr<BearSSL::WiFiClientSecure> clientS(new BearSSL::WiFiClientSecure);
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// Tempo
time_t rawtime = 788896740;
struct tm ts;
char buf[80];

// Variáveis para controle
bool displayAvailable = false;
bool rtcAvailable = false;
bool otaUpgrade = false;
bool ntpInitialized = false;
unsigned long lastInteraction = 0;
bool displaySleeping = false;

// Tratar eventos de botões
bool upPressed;
bool downPressed;
bool selectPressed;
uint8_t mask = 0;

// bit -> significado
const uint8_t FLAG_UP      = 1 << 0; // bit 0
const uint8_t FLAG_DOWN    = 1 << 1; // bit 1
const uint8_t FLAG_LEFT    = 1 << 2; // bit 2
const uint8_t FLAG_RIGHT   = 1 << 3; // bit 3
const uint8_t FLAG_FORWARD = 1 << 4; // bit 4  (indica "sentido para a cabeça")
const uint8_t FLAG_BULGING = 1 << 5; // bit 5  (pedaço 'inchado' / boca aberta)

GFXfont clockFont = RussoOne18pt4b;
GFXfont astralFont = MoonPhases7x7;
GFXfont snakeFont = SnakeII;