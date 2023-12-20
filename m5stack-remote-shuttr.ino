#include <M5Stack.h>
#include <FastLED.h>
#include <utility/M5Timer.h>
#define debug

// ポート設定 (M5GO Bottom Module)
// シャッター信号
// PORT.B(GPIO26,36)は36が出力に使用できないためPORT.C(GPIO16,17)を使用する
#define SIGNAL_FOCUS 17
#define SIGNAL_SHUTTER 16
// LED Bar(GPIO15)
#define LED_PIN_DATA 15

// LED設定
#define LED_COLOR_STANDBY CRGB::White
#define LED_COLOR_FOCUS CRGB::Green
#define LED_COLOR_SHUTTER CRGB::Red
#define LED_COLOR_OFF CRGB::Black
#define NUM_LEDS 10
#define ON true
#define OFF false

// 設定モード定義
const int MODE_FIRE = 0;
const int MODE_SHUTTER = 1;
const int MODE_FOCUS = 2;
int mode = MODE_FIRE;
// タイマーの実行開始フラグ
boolean isTimerStarted = false;
// シャッターインターバル時間
unsigned int shutterIntervalTime = 2;
// フォーカシング時間
unsigned int focusingTime = 1;
// LEDアレイ
CRGB leds[NUM_LEDS];

// 値の描画座標
const int pos_fire_x = 10;
const int pos_fire_y = 5;
const int pos_shutter_x = 100;
const int pos_shutter_y = 40;
const int pos_focus_x = 100;
const int pos_focus_y = 130;

// タイマー
M5Timer timer;
int timerId = -1;

// フォーカスボタン(シャッターボタン半押し)
void setFocusButton(boolean focusButton) {
  if (focusButton == ON) {
    fill_solid(leds, NUM_LEDS, LED_COLOR_FOCUS);
    FastLED.show();
    M5.Lcd.fillRect(0, 298, 32, 32, BLUE);
    digitalWrite(SIGNAL_FOCUS, HIGH);
  } else {
    fill_solid(leds, NUM_LEDS, LED_COLOR_OFF);
    FastLED.show();
    M5.Lcd.fillRect(0, 298, 32, 32, BLACK);
    digitalWrite(SIGNAL_FOCUS, LOW);
  }
}

// シャッターボタン
void setShutterButton(boolean shutterButton) {
  if (shutterButton == ON) {
    fill_solid(leds, NUM_LEDS, LED_COLOR_SHUTTER);
    FastLED.show();
    M5.Lcd.fillRect(0, 298, 32, 32, RED);
    digitalWrite(SIGNAL_SHUTTER, HIGH);
  } else {
    fill_solid(leds, NUM_LEDS, LED_COLOR_OFF);
    FastLED.show();
    M5.Lcd.fillRect(0, 298, 32, 32, BLACK);
    digitalWrite(SIGNAL_SHUTTER, LOW);
  }
}

// Fire shutter
void shutter() {
  setFocusButton(ON);
  for (int i = 1; i <= focusingTime * 10000; i++) {
    delayMicroseconds(100);
  }
  setShutterButton(ON);
  for (int i = 1; i <= 100; i++) {
    delayMicroseconds(100);
  }
  setShutterButton(OFF);
  setFocusButton(OFF);
}

void setup() {
  M5.begin();
  Serial.begin(115200);

  // GPIO設定
  pinMode(SIGNAL_FOCUS, OUTPUT);
  pinMode(SIGNAL_SHUTTER, OUTPUT);

  // LED初期設定
  FastLED.addLeds<NEOPIXEL, LED_PIN_DATA>(leds, NUM_LEDS);  // GRB ordering is assumed
  fill_solid(leds, NUM_LEDS, LED_COLOR_STANDBY);
  FastLED.show();

  // LCD初期設定
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);

  // 初期描画
  M5.Lcd.drawRightString("SEC", pos_shutter_x + 210, pos_shutter_y + 60, 2);
  M5.Lcd.drawRightString("SEC", pos_focus_x + 210, pos_focus_y + 60, 2);
  draw_mode();
  draw_value();
}

void loop() {
  M5.update();

  // タイマー実行
  if (isTimerStarted) {
    timer.enable(timerId);
    timer.run();
  } else {
    timer.deleteTimer(timerId);
    timerId = -1;
  }

  if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 200)) {
    // モードボタン
    if (!isTimerStarted) {
      mode = (mode + 1) % 3;
    }
    draw_mode();
    draw_value();
  } else if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(1000, 200)) {
    // -ボタン
    switch (mode) {
      case MODE_FIRE:
        if (!isTimerStarted) {
          // タイマー開始フラグを設定
          isTimerStarted = true;
          timerId = timer.setInterval(shutterIntervalTime * 1000, shutter);  // ms指定のため1000倍する
          fill_solid(leds, NUM_LEDS, LED_COLOR_OFF);
          FastLED.show();
        } else {
          isTimerStarted = false;
          fill_solid(leds, NUM_LEDS, LED_COLOR_STANDBY);
          FastLED.show();
        }
        break;
      case MODE_SHUTTER:
        // シャッターインターバル時間の最小値は1
        if (shutterIntervalTime > 1) shutterIntervalTime--;
        // フォーカシング時間の最大値はシャッターインターバル時間-1
        if (focusingTime == shutterIntervalTime) focusingTime = shutterIntervalTime - 1;
        break;
      case MODE_FOCUS:
        // フォーカシング時間の最小値は0
        if (focusingTime > 0) focusingTime--;
        break;
    }
    draw_mode();
    draw_value();
  } else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 200)) {
    // +ボタン
    switch (mode) {
      case MODE_FIRE:
        if (!isTimerStarted) {
          // タイマー開始フラグを設定
          isTimerStarted = true;
          timerId = timer.setInterval(shutterIntervalTime * 1000, shutter);  // ms指定のため1000倍する
          fill_solid(leds, NUM_LEDS, LED_COLOR_OFF);
          FastLED.show();
        } else {
          isTimerStarted = false;
          fill_solid(leds, NUM_LEDS, LED_COLOR_STANDBY);
          FastLED.show();
        }
        break;
      case MODE_SHUTTER:
        // シャッターインターバル時間の最大値は1
        if (shutterIntervalTime < 999) shutterIntervalTime++;
        break;
      case MODE_FOCUS:
        // フォーカシング時間の最大値はシャッターインターバル時間-1
        if (focusingTime < shutterIntervalTime - 1) focusingTime++;
        break;
    }
    draw_mode();
    draw_value();
  }
}

// 値描画
void draw_value() {
  M5.Lcd.fillRect(pos_shutter_x, pos_shutter_y, 160, 240 - pos_shutter_y - 30, BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  // 右寄せで描画
  M5.Lcd.setTextDatum(2);
  M5.Lcd.drawNumber(shutterIntervalTime, pos_shutter_x + 160, pos_shutter_y, 8);
  M5.Lcd.drawNumber(focusingTime, pos_focus_x + 160, pos_focus_y, 8);
  M5.Lcd.setTextDatum(0);
}

// モード描画
void draw_mode() {
  switch (mode) {
    case MODE_FIRE:
      // モードボタン
      if (isTimerStarted) {
        // 動作中
        M5.Lcd.fillRect(0, 0, 320, 32, RED);
        M5.Lcd.setTextColor(WHITE, RED);
        M5.Lcd.drawString("RUNNING", pos_fire_x, pos_fire_y, 4);
        M5.Lcd.fillRect(0, 215, 320, 30, BLACK);
        M5.Lcd.setTextColor(DARKGREY, BLACK);
        M5.Lcd.drawString("MODE", 26, 215, 4);
        M5.Lcd.setTextColor(RED, BLACK);
        M5.Lcd.drawString("STOP", 124, 215, 4);
        M5.Lcd.drawString("STOP", 220, 215, 4);
      } else {
        // 停止中
        M5.Lcd.fillRect(0, 0, 320, 32, BLACK);
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.drawString("STANDBY", pos_fire_x, pos_fire_y, 4);
        M5.Lcd.fillRect(0, 215, 320, 30, BLACK);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.drawString("MODE", 26, 215, 4);
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.drawString("START", 120, 215, 4);
        M5.Lcd.drawString("START", 216, 215, 4);
      }
      // 非アクティブ
      M5.Lcd.setTextColor(WHITE, BLACK);
      M5.Lcd.drawString("SHUTTER", pos_shutter_x - 90, pos_shutter_y, 2);
      M5.Lcd.drawString("INTERVAL", pos_shutter_x - 90, pos_shutter_y + 20, 2);
      M5.Lcd.drawString("FOCUSING", pos_focus_x - 90, pos_focus_y, 2);
      M5.Lcd.drawString("TIME", pos_focus_x - 90, pos_focus_y + 20, 2);
      break;
    case MODE_SHUTTER:
      // アクティブ
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.drawString("SHUTTER", pos_shutter_x - 90, pos_shutter_y, 2);
      M5.Lcd.drawString("INTERVAL", pos_shutter_x - 90, pos_shutter_y + 20, 2);
      // 非アクティブ
      M5.Lcd.fillRect(0, 0, 320, 32, BLACK);
      M5.Lcd.setTextColor(WHITE, BLACK);
      M5.Lcd.drawString("STANDBY", pos_fire_x, pos_fire_y, 4);
      M5.Lcd.drawString("FOCUSING", pos_focus_x - 90, pos_focus_y, 2);
      M5.Lcd.drawString("TIME", pos_focus_x - 90, pos_focus_y + 20, 2);
      M5.Lcd.fillRect(0, 215, 320, 30, BLACK);
      M5.Lcd.drawString("MODE", 26, 215, 4);
      M5.Lcd.drawString("-", 154, 215, 4);
      M5.Lcd.drawString("+", 246, 215, 4);
      break;
    case MODE_FOCUS:
      // アクティブ
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.drawString("FOCUSING", pos_focus_x - 90, pos_focus_y, 2);
      M5.Lcd.drawString("TIME", pos_focus_x - 90, pos_focus_y + 20, 2);
      // 非アクティブ
      M5.Lcd.fillRect(0, 0, 320, 32, BLACK);
      M5.Lcd.setTextColor(WHITE, BLACK);
      M5.Lcd.drawString("STANDBY", pos_fire_x, pos_fire_y, 4);
      M5.Lcd.drawString("SHUTTER", pos_shutter_x - 90, pos_shutter_y, 2);
      M5.Lcd.drawString("INTERVAL", pos_shutter_x - 90, pos_shutter_y + 20, 2);
      M5.Lcd.fillRect(0, 215, 320, 30, BLACK);
      M5.Lcd.drawString("MODE", 26, 215, 4);
      M5.Lcd.drawString("-", 154, 215, 4);
      M5.Lcd.drawString("+", 246, 215, 4);
      break;
  }
}
