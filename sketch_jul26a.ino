#include <math.h>
#include "GyverButton.h"
#define BTN1_PIN 8   // Пин для первой кнопки
#define BTN2_PIN 9   // Пин для второй кнопки
#define BTN3_PIN 10  // Пин для третьей кнопки

const int thermistorBoilerPin = A0;
const int thermistorSteamPin = A1;

const int cappuccinoSensorPin = 2;
const int flowmeterPin = 7;

const int pumpRelayPin = 3;
const int boilerRelayPin = 4;
const int valveRelayPin = 5;
const int steamRelayPin = 6;


const float referenceVoltage = 5.0;  // Опорное напряжение
float temperatureBoiler = 25;
float temperatureSteam = 25;

GButton espressoButtonPin(BTN1_PIN);
GButton cappuccinoButtonPin(BTN2_PIN);
GButton latteButtonPin(BTN3_PIN);

const unsigned long temperatureInterval = 1000;  // Интервал обновления температуры
unsigned long previousMillis = 0;                // Переменная для хранения времени последнего обновления
unsigned long currentMillis = 0;


void setup() {
  Serial.begin(9600);
  pinMode(boilerRelayPin, OUTPUT);
  pinMode(steamRelayPin, OUTPUT);
  pinMode(pumpRelayPin, OUTPUT);
  pinMode(valveRelayPin, OUTPUT);

  pinMode(cappuccinoSensorPin, INPUT_PULLUP);
  pinMode(flowmeterPin, INPUT_PULLUP);

  digitalWrite(boilerRelayPin, HIGH);
  digitalWrite(steamRelayPin, HIGH);
  digitalWrite(pumpRelayPin, HIGH);
  digitalWrite(valveRelayPin, HIGH);

  espressoButtonPin.setDebounce(50);       // Установка времени антидребезга
  espressoButtonPin.setClickTimeout(500);  // Установка таймаута между кликами
  espressoButtonPin.setTimeout(1000);      // Установка таймаута удержания
  cappuccinoButtonPin.setDebounce(50);
  cappuccinoButtonPin.setClickTimeout(500);
  cappuccinoButtonPin.setTimeout(1000);

  latteButtonPin.setDebounce(50);
  latteButtonPin.setClickTimeout(500);
  latteButtonPin.setTimeout(1000);
}

void loop() {

  currentMillis = millis();

  handleButton(espressoButtonPin, 1);    // Обработка первой кнопки
  handleButton(cappuccinoButtonPin, 2);  // Обработка второй кнопки
  handleButton(latteButtonPin, 3);       // Обработка третьей кнопки
  // Обновление температуры
  checkTemperature(currentMillis);
}

float readThermistor(int pin) {
  float betaCoefficient = 3950;
  float resistorNominal = 9800;
  int analogValue = analogRead(pin);
  float resistance = resistorNominal * ((referenceVoltage / (analogValue * (referenceVoltage / 1023.0))) - 1);
  float lnRt = resistance / 50000;
  lnRt = log(lnRt);
  float invT = (1.0 / 298.15) + (lnRt / betaCoefficient);
  float temperatureC = (1.0 / invT) - 273.15;
  return temperatureC;
}


void boileronoff() {
  if (temperatureBoiler >= 110) {
    digitalWrite(boilerRelayPin, HIGH);  // Выключение реле
  };
  if (temperatureBoiler <= 100) {
    digitalWrite(boilerRelayPin, LOW);  // Включение реле
  };
}

bool checkMilk() {
  bool stat = false;
  if (digitalRead(cappuccinoSensorPin) == LOW) {
    stat = true;
  } else {
    stat = false;
  }
  return stat;
}

bool flowmeter() {
  bool stat = false;
  unsigned long duration = pulseIn(flowmeterPin, LOW);
  if (duration > 0) {
    stat = true;
  } else {
    stat = false;
  }
  return stat;
}

void checkTemperature(unsigned long currentMillis) {
  if (currentMillis - previousMillis >= temperatureInterval) {
    previousMillis = currentMillis;

    temperatureBoiler = readThermistor(thermistorBoilerPin);
    temperatureSteam = readThermistor(thermistorSteamPin);
    /*
    Serial.print("Температура бойлера: ");
    Serial.println(temperatureBoiler);*/
    boileronoff(); /*
    Serial.print("Температура стимера: ");
    Serial.println(temperatureSteam);*/
    heatSteamer();
  }
}

void handleButton(GButton &button, int buttonNumber) {
  button.tick();  // Вызов обязательной функции для обработки кнопки

  if (button.isSingle()) {
    handleSingleClick(buttonNumber);
  }
  if (button.isDouble()) {
    handleDoubleClick(buttonNumber);
  }
  if (button.isHolded()) {
    handleLongPress(buttonNumber);
  }
}

void handleSingleClick(int buttonNumber) {
  // Вызов функции для одного короткого нажатия
  switch (buttonNumber) {
    case 1: function1(); break;
    case 2: function2(); break;
    case 3: function3(); break;
  }
}

void handleDoubleClick(int buttonNumber) {
  // Вызов функции для двух коротких нажатий
  switch (buttonNumber) {
    case 1: function4(); break;
    case 2: function5(); break;
    case 3: function6(); break;
  }
}

void handleLongPress(int buttonNumber) {
  // Вызов функции для долгого нажатия
  switch (buttonNumber) {
    case 1: function7(); break;
    case 2: function8(); break;
    case 3: function9(); break;
  }
}

bool checkBoilerTemperature() {
  if (temperatureBoiler < 100) {
    Serial.println("Ошибка: температура бойлера слишком низкая.");
    return false;
  }
  return true;
}

bool checkWater() {
  digitalWrite(pumpRelayPin, LOW);  // Включение насоса
  delay(1000);                      // Небольшая задержка для запуска потока воды

  if (!flowmeter()) {
    Serial.println("Ошибка: нет воды в баке.");
    digitalWrite(pumpRelayPin, HIGH);  // Выключение насоса
    return false;
  }
  return true;
}


// Вспомогательная функция для управления парогенератором
void heatSteamer() {
  if (temperatureSteam < 130) {
    Serial.println("Нагрев парогенератора...");
    digitalWrite(steamRelayPin, LOW);  // Включение нагрева парогенератора
    while (temperatureSteam < 150) {
      delay(1000);  // Задержка для проверки температуры
      temperatureSteam = readThermistor(thermistorSteamPin);
      Serial.print("Температура парогенератора: ");
      Serial.println(temperatureSteam);
    }
    digitalWrite(steamRelayPin, HIGH);  // Выключение нагрева парогенератора
  }
}

void makeCoffee(int pumpDelay) {
  digitalWrite(pumpRelayPin, LOW);   // Включение насоса
  delay(pumpDelay);                  // Время приготовления
  digitalWrite(pumpRelayPin, HIGH);  // Выключение насоса
}

// Вспомогательная функция для приготовления молочной пены
void makeMilkFoam(int foamDelay) {
  Serial.println("Приготовление молочной пены...");
  digitalWrite(valveRelayPin, LOW);   // Открытие клапана
  digitalWrite(pumpRelayPin, LOW);    // Включение насоса
  delay(foamDelay);                   // Время приготовления молочной пены
  digitalWrite(pumpRelayPin, HIGH);   // Выключение насоса
  digitalWrite(valveRelayPin, HIGH);  // Закрытие клапана
}


void function1() {
  if (!checkBoilerTemperature() || !checkWater()) return;

  // Закрытие клапана
  digitalWrite(valveRelayPin, HIGH);  // Закрытие клапана

  // Продолжение приготовления эспрессо
  delay(500);         // Небольшая задержка перед началом приготовления
  makeCoffee(23000);  // Время приготовления эспрессо

  Serial.println("Эспрессо готово!");
}


void function2() {
  if (!checkBoilerTemperature() || !checkWater() || !checkMilk()) return;

  // Закрытие клапана


  digitalWrite(valveRelayPin, HIGH);  // Закрытие клапана

  // Приготовление капучино
  Serial.println("Приготовление капучино...");

  makeCoffee(23000);  // Время приготовления эспрессо

  delay(2000);  // Ожидание, например, 2 секунды

  // Нагрев парогенератора и приготовление молочной пены
  heatSteamer();
  makeMilkFoam(10000);  // Время приготовления молочной пены

  Serial.println("Капучино готово!");
}

// Функция 3
void function3() {
  if (!checkBoilerTemperature() || !checkWater() || !checkMilk()) return;

  // Закрытие клапана
  digitalWrite(valveRelayPin, HIGH);  // Закрытие клапана

  // Приготовление латте
  Serial.println("Приготовление латте...");

  makeCoffee(23000);  // Время приготовления эспрессо

  delay(2000);  // Ожидание, например, 2 секунды

  // Нагрев парогенератора и приготовление молочной пены
  heatSteamer();
  makeMilkFoam(7500);  // Время приготовления молочной пены

  Serial.println("Латте готово!");
}


// Функция 4
void function4() {
  if (!checkBoilerTemperature() || !checkWater()) return;

  // Закрытие клапана
  digitalWrite(valveRelayPin, HIGH);  // Закрытие клапана

  // Продолжение приготовления эспрессо
  delay(500);         // Небольшая задержка перед началом приготовления
  makeCoffee(46000);  // Время приготовления эспрессо

  Serial.println("Эспрессо готово!");
}


// Функция 5
void function5() {
  if (!checkBoilerTemperature() || !checkWater() || !checkMilk()) return;

  // Закрытие клапана
  digitalWrite(valveRelayPin, HIGH);  // Закрытие клапана

  // Приготовление капучино
  Serial.println("Приготовление капучино...");

  makeCoffee(46000);  // Время приготовления эспрессо

  delay(2000);  // Ожидание, например, 2 секунды

  // Нагрев парогенератора и приготовление молочной пены
  heatSteamer();
  makeMilkFoam(20000);  // Время приготовления молочной пены

  Serial.println("Капучино готово!");
}

// Функция 6
void function6() {
  if (!checkBoilerTemperature() || !checkWater() || !checkMilk()) return;

  // Закрытие клапана
  digitalWrite(valveRelayPin, HIGH);  // Закрытие клапана

  // Приготовление латте
  Serial.println("Приготовление латте...");

  makeCoffee(46000);  // Время приготовления эспрессо

  delay(2000);  // Ожидание, например, 2 секунды

  // Нагрев парогенератора и приготовление молочной пены
  heatSteamer();
  makeMilkFoam(15000);  // Время приготовления молочной пены

  Serial.println("Латте готово!");
}

// Функция 7
void function7() {
  // Выдача кипятка
  digitalWrite(pumpRelayPin, LOW);   // Включение насоса
  delay(20000);                      // Время выдачи кипятка
  digitalWrite(pumpRelayPin, HIGH);  // Выключение насоса
}

// Функция 8
void function8() {
  if (!checkBoilerTemperature() || !checkWater() || !checkMilk()) return;

  // Закрытие клапана
  digitalWrite(valveRelayPin, HIGH);  // Закрытие клапана
  Serial.println("Приготовление пены...");

  // Нагрев парогенератора и приготовление молочной пены
  heatSteamer();
  makeMilkFoam(10000);  // Время приготовления молочной пены

  Serial.println("Пена готова!");
}

// Функция 9 //промвыка
void function9() {
  if (!checkBoilerTemperature() || temperatureSteam < 100) {
    Serial.println("Ошибка: температура бойлера или стимера слишком низкая.");
    return;  // Прекращение выполнения функции, если температура слишком низкая
  }

  // Выдача кипятка
  digitalWrite(pumpRelayPin, LOW);  // Включение насоса
  delay(20000);                     // Время выдачи кипятка

  // Открытие клапана и выдача воды
  digitalWrite(valveRelayPin, LOW);   // Открытие клапана
  delay(20000);                       // Время выдачи воды
  digitalWrite(pumpRelayPin, HIGH);   // Выключение насоса
  digitalWrite(valveRelayPin, HIGH);  // Закрытие клапана
}