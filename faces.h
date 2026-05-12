#include "navigation.h"

class MainFace : public Face {
public:
  void show() override {
    // Atualizar display
    if (displayAvailable) {
      display.clearDisplay();
      display.setTextSize(1);

      if (rtcAvailable) {
        DateTime now = rtc.now();

        // Exibir data
        display.setCursor(0, 0);
        display.print(weekday(now.dayOfTheWeek()) + " ");
        if (now.day() < 10) display.print("0");
        display.print(now.day());
        display.print("/");
        if (now.month() < 10) display.print("0");
        display.print(now.month());
        display.print("/");
        display.print(now.year() - 2000);

        // Exibir hora
        display.setCursor(6, 34);
        display.setFont(&clockFont);
        if (now.hour() < 10) display.print("0");
        display.print(now.hour());
        display.print(":");
        if (now.minute() < 10) display.print("0");
        display.print(now.minute());
        display.setFont();

        // Exibir segundos
        display.setCursor(113, 34);
        if (now.second() < 10) display.print("0");
        display.println(now.second());

        // Exibir fase da lua
        display.setFont(&astralFont);
        display.setCursor(121, 0);
        display.print(moonPhase());
        display.setFont();
      }
      display.display();
    }
  }

  Face* handleUp(bool wasHold) override {
    if (!wasHold) return previousFace();
    return this;
  }

  Face* handleDown(bool wasHold) override {
    if (!wasHold) return nextFace();
    return this;
  }

  void action(bool wasHold) override {}
  void exit() override {}
};

class WiFiFace : public Face {
public:
  void show() override {
    // Atualizar display
    if (displayAvailable) {
      display.clearDisplay();

      display.setTextSize(5);
      display.setCursor(0, 0);
      display.println("WiFi");

      // Exibir IP
      display.setTextSize(1);
      if (rtcAvailable) {
        DateTime now = rtc.now();
        display.setCursor(0, 45);
        if (now.hour() < 10) display.print("0");
        display.print(now.hour());
        display.print(":");
        if (now.minute() < 10) display.print("0");
        display.print(now.minute());
        display.print(":");
        if (now.second() < 10) display.print("0");
        display.print(now.second());
      }
      display.setCursor(0, 55);
      if (WiFi.status() == WL_CONNECTED) {
        display.println(WiFi.localIP());
      } else {
        display.println("Desconectado");
      }

      display.display();
    }

    server.handleClient();
  }

  void action(bool wasHold) override {
    bool justConnected = false;
    if (WiFi.status() != WL_CONNECTED) {
      connectToWiFi();
      justConnected = true;
    }
    if (justConnected && WiFi.status() == WL_CONNECTED) {
      justConnected = false;
      server.begin();
      Serial.println("HTTPUpdateServer pronto! Acesse: http://" + WiFi.localIP().toString() + "/ota");
    }
  }

  void exit() override {
    if (WiFi.status() == WL_CONNECTED) {
      server.close();
      WiFi.disconnect(true);
    }
    WiFi.mode(WIFI_OFF);
  }
};

class TimerFace : public Face {
private:
  int minutes = 1;  // valor inicial do timer
  bool setting = false;
  bool running = false;

public:
  void show() override {
    if (displayAvailable) {
      display.clearDisplay();

      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("TIMER");

      if (running && timerActive) {
        // Mostrar tempo restante
        unsigned long remaining = (timerEnd - millis()) / 1000;
        int rMin = remaining / 60;
        int rSec = remaining % 60;

        display.setTextSize(3);
        display.setCursor(0, 30);
        if (rMin <= 0 && rSec <= 0) {
          display.print("ALARM");
        } else {
          if (rMin < 10) display.print("0");
          display.print(rMin);
          display.print(":");
          if (rSec < 10) display.print("0");
          display.print(rSec);
        }
      } else {
        // Mostrar valor configurado
        display.setTextSize(3);
        display.setCursor(0, 30);
        display.print(minutes >= 60 ? "01:" : "00:");
        if (minutes % 60 < 10) display.print("0");
        display.print(minutes % 60);
        // Mostrar hora final
        if (rtcAvailable) {
          DateTime later = rtc.now() + TimeSpan(0, 0, minutes, 0);
          display.setCursor(0, 55);
          display.setTextSize(1);
          if (later.hour() < 10) display.print("0");
          display.print(later.hour());
          display.print(":");
          if (later.minute() < 10) display.print("0");
          display.print(later.minute());
          display.print(":");
          if (later.second() < 10) display.print("0");
          display.print(later.second());
        }
      }

      display.display();
    }
  }


  void action(bool wasHold) override {
    if (!wasHold) {
      if (!setting && !running) {
        // Primeiro clique → entrar no modo de configuração
        setting = true;
      } else if (setting && !running) {
        // Segundo clique → iniciar timer
        startTimer(minutes * 60000UL);
        running = true;
        setting = false;
      } else if (running) {
        // Terceiro clique → cancelar timer
        stopTimer();
        running = false;
      }
    }
    if (setting) {
      setRGBLed(0, 0, 255);
    } else {
      setRGBLed();
    }
  }

  Face* handleUp(bool wasHold) override {
    if (setting && !running) {
      minutes++;
      if (minutes > 90) minutes = 1;
      return this;
    }
    return previousFace();
  }

  Face* handleDown(bool wasHold) override {
    if (setting && !running) {
      minutes--;
      if (minutes < 1) minutes = 90;
      return this;
    }
    return nextFace();
  }

  void exit() override {
    display.setTextSize(1);
    setRGBLed();
  }
};

class LedFace : public Face {
private:
  bool led = false;

public:
  void show() override {
    if (displayAvailable) {
      display.clearDisplay();

      display.setTextSize(6);
      display.setCursor(0, 0);

      if (led) {
        display.println("ON");
      } else {
        display.println("OFF");
      }

      display.display();
    }
  }

  void action(bool wasHold) override {
    if (!wasHold) led = !led;
    digitalWrite(WHITE_LED_PIN, !led);
    setRGBLed(0xFFFFFF, led ? 255 : 0);
  }

  void exit() override {
    led = false;
    digitalWrite(WHITE_LED_PIN, HIGH);
    setRGBLed();
  }
};

class CalendarFace : public Face {
private:
  bool navigating = false;  // se true, estou "dentro" do calendário
  int monthOffset = 0;      // deslocamento em relação ao mês atual

  // nomes dos meses em português
  const char* months[12] = {
    "Janeiro", "Fevereiro", "Marco", "Abril", "Maio", "Junho",
    "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"
  };

  // calcular número de dias do mês (simples, pode usar RTC para ano bissexto real)
  int daysInMonth(int month, int year) {
    switch (month) {
      case 1: return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
      case 3:
      case 5:
      case 8:
      case 10: return 30;
      default: return 31;
    }
  }

public:
  void show() override {
    if (!displayAvailable) return;
    display.clearDisplay();

    // Data base
    DateTime now = rtc.now();
    int year = now.year();
    int month = now.month() - 1;  // DateTime retorna 1–12
    int day = now.day();

    // aplicar offset de navegação
    month += monthOffset;
    while (month < 0) {
      month += 12;
      year--;
    }
    while (month > 11) {
      month -= 12;
      year++;
    }

    // Cabeçalho: mês e ano
    display.setTextSize(1);
    display.setCursor(0, 0);
    if (navigating) {
      display.fillRect(0, 0, 128, 8, SH110X_WHITE);
      display.setTextColor(SH110X_BLACK);
    }
    display.println(String(months[month]) + " " + String(year));
    display.setTextColor(SH110X_WHITE);  // resetar

    // Cabeçalho dias da semana
    const char* weekDays[7] = { "D", "S", "T", "Q", "Q", "S", "S" };
    for (int i = 0; i < 7; i++) {
      display.setCursor(i * 18 + 5, 9);
      display.print(weekDays[i]);
    }

    // calcular primeiro dia do mês (0 = domingo)
    DateTime firstDay(year, month + 1, 1, 0, 0, 0);
    int startDay = firstDay.dayOfTheWeek();

    // desenhar os dias
    int totalDays = daysInMonth(month, year);
    int x = 0, y = 20;
    for (int d = 1; d <= totalDays; d++) {
      int col = (startDay + d - 1) % 7;
      int row = (startDay + d - 1) / 7;
      x = col * 18;
      y = 20 + row * 9;
      int offsetX = (d < 10) ? 5 : 2;

      if (col == 0 || isHoliday(d, month + 1)) {
        // domingo: inverter cores
        if (d == now.day() && month == now.month() - 1) {
          display.fillRect(x, y, 16, 8, SH110X_BLACK);
          display.fillRect(x + 1, y + 1, 14, 6, SH110X_WHITE);
        } else {
          display.fillRect(x, y, 16, 8, SH110X_WHITE);
        }
        display.setTextColor(SH110X_BLACK);
        display.setCursor(x + offsetX, y);
        display.print(d);
        display.setTextColor(SH110X_WHITE);  // resetar
      } else if (d == now.day() && month == now.month() - 1) {
        display.fillRect(x, y, 16, 8, SH110X_WHITE);
        display.fillRect(x + 1, y + 1, 14, 6, SH110X_BLACK);
        display.setCursor(x + offsetX, y);
        display.print(d);
      } else {
        display.setCursor(x + offsetX, y);
        display.print(d);
      }
    }

    display.display();
  }

  void action(bool wasHold) override {
    if (!wasHold) navigating = !navigating;  // alterna entre navegar e sair
  }

  Face* handleUp(bool wasHold) override {
    if (navigating) {
      monthOffset--;  // mês anterior
      return this;
    }
    return previousFace();
  }

  Face* handleDown(bool wasHold) override {
    if (navigating) {
      monthOffset++;  // próximo mês
      return this;
    }
    return nextFace();
  }

  void enter() override {
    tone(BUZZER_PIN, 600, 100);
    navigating = false;
    monthOffset = 0;
  }

  void exit() override {
    navigating = false;
    monthOffset = 0;
  }
};

class SnakeFace : public Face {
private:
  static const int GRID_W = 22;
  static const int GRID_H = 12;
  static const int MAX_LEN = GRID_W * GRID_H;

  struct Point {
    int x, y;
  };

  Point snake[MAX_LEN];
  int snakeLen;
  Point food;
  int dir;  // 0=up,1=right,2=down,3=left
  bool running = false;
  bool paused = false;
  unsigned long lastMove = 0;
  int score = 0;
  int hScore;

public:
  SnakeFace() {
    resetGame();
  }

  void resetGame() {
    hScore = max(score, hScore);
    preferences.putInt("hScore", hScore);
    snakeLen = 7;
    for (int i = 0; i < 7; i++) {
      snake[i] = { GRID_W / 2 - i, GRID_H / 2 };
    }
    dir = 1;
    score = 0;
    spawnFood();
    running = false;
    paused = false;
  }

  void spawnFood() {
    bool valid = false;
    while (!valid) {
      food.x = random(0, GRID_W);
      food.y = random(0, GRID_H);
      valid = true;
      for (int i = 0; i < snakeLen; i++) {
        if (snake[i].x == food.x && snake[i].y == food.y) {
          valid = false;
          break;
        }
      }
    }
  }

  uint8_t makeMaskFromSegment(int i) {
    Point cur = snake[i];
    Point prev = { -999, -999 }, next = { -999, -999 };
    if (i > 0) prev = snake[i - 1];
    if (i < snakeLen - 1) next = snake[i + 1];

    bool up = ((prev.y == cur.y - 1 && prev.x == cur.x) || (next.y == cur.y - 1 && next.x == cur.x));
    bool down = ((prev.y == cur.y + 1 && prev.x == cur.x) || (next.y == cur.y + 1 && next.x == cur.x));
    bool left = ((prev.x == cur.x - 1 && prev.y == cur.y) || (next.x == cur.x - 1 && next.y == cur.y));
    bool right = ((prev.x == cur.x + 1 && prev.y == cur.y) || (next.x == cur.x + 1 && next.y == cur.y));

    bool forward;
    if (i == 0) forward = true;                                   // cabeça -> sempre forward
    else if (i == snakeLen - 1) forward = false;                  // cauda -> never forward
    else forward = (prev.x - cur.x == 1 || prev.y - cur.y == 1);  // segmentos interm.: tratar como "apontando p/ cabeça"

    bool bulging = false;  // só cabeça (ou adapte para propagar)

    uint8_t mask = 0;
    if (up) mask |= FLAG_UP;
    if (down) mask |= FLAG_DOWN;
    if (left) mask |= FLAG_LEFT;
    if (right) mask |= FLAG_RIGHT;
    if (forward) mask |= FLAG_FORWARD;
    if (bulging) mask |= FLAG_BULGING;
    return mask;
  }

  void show() override {
    if (!displayAvailable) return;
    display.clearDisplay();

    // Desenha barreira
    display.fillRect(0, 0, (GRID_W + 2) * 4 + 3, (GRID_H + 1) * 4 + 3, SH110X_WHITE);
    display.fillRect(1, 1, (GRID_W + 2) * 4 + 1, (GRID_H + 1) * 4 + 1, SH110X_BLACK);

    // desenhar cobra
    display.setFont(&snakeFont);
    for (int i = 0; i < snakeLen; i++) {
      display.setCursor((snake[i].x + 1) * 4 + 3, (snake[i].y + 1) * 4 + 3);
      display.write(65 + makeMaskFromSegment(i));
    }

    // desenhar comida
    display.setCursor((food.x + 1) * 4 + 3, (food.y + 1) * 4 + 3);
    display.write(65 + FLAG_BULGING);

    // placar
    display.setFont();
    display.setTextSize(1);
    display.setCursor(101, 0);
    display.print("PTS");
    display.setCursor(101, 10);
    display.print(score);
    display.setCursor(101, 20);
    display.print("HIGH");
    display.setCursor(101, 30);
    display.print(hScore);

    display.display();
  }

  void
  action(bool wasHold) override {
    if (!wasHold) {
      if (!running) {
        running = true;
        paused = false;
      } else if (paused) {
        paused = false;
      } else {
        paused = true;
      }
    }
  }

  Face* handleUp(bool wasHold) override {
    if (running && !paused) {
      if (!wasHold) dir = (dir + 3) % 4;  // esquerda
      return this;
    }
    return previousFace();
  }

  Face* handleDown(bool wasHold) override {
    if (running && !paused) {
      if (!wasHold) dir = (dir + 1) % 4;  // direita
      return this;
    }
    return nextFace();
  }

  void update() override {
    if (!running || paused) return;

    if (millis() - lastMove > 500) {
      lastMove = millis();

      Point head = snake[0];
      switch (dir) {
        case 0: head.y--; break;
        case 1: head.x++; break;
        case 2: head.y++; break;
        case 3: head.x--; break;
      }

      // colisão com parede
      if (head.x < 0 || head.x >= GRID_W || head.y < 0 || head.y >= GRID_H) {
        setRGBLed(255, 0, 0);
        for (int f = 600; f > 200; f -= 100) {
          tone(BUZZER_PIN, f, 100);
          delay(120);
        }
        setRGBLed(0);
        resetGame();
        return;
      }

      // colisão consigo mesma
      for (int i = 0; i < snakeLen; i++) {
        if (snake[i].x == head.x && snake[i].y == head.y) {
          setRGBLed(255, 0, 0);
          for (int f = 600; f > 200; f -= 100) {
            tone(BUZZER_PIN, f, 100);
            delay(120);
          }
          setRGBLed(0);
          resetGame();
          return;
        }
      }

      // mover corpo
      for (int i = snakeLen; i > 0; i--) {
        snake[i] = snake[i - 1];
      }
      snake[0] = head;

      if (head.x == food.x && head.y == food.y) {
        tone(BUZZER_PIN, 440, 100);
        snakeLen++;
        score++;
        spawnFood();
      } else if (snakeLen < MAX_LEN) {
        // nada, só mantém o tamanho
      }
    }
  }

  void enter() override {
    tone(BUZZER_PIN, 600, 100);
    preferences.begin("snake", false);
    hScore = preferences.getInt("hScore", 0);
  }

  void exit() override {
    preferences.end();
  }
};