#include <LiquidCrystal_I2C.h>
#include <Adafruit_BMP280.h>
#include "EEPROMAnything.h"

LiquidCrystal_I2C disp(0x27, 20, 4);
Adafruit_BMP280 bmp;
int interval = 20;
bool menu = 0;
int btn[] = {8, 9, 10};
int sel = 0, prev = -1;
long curr_time, prev_time;
float slope;
typedef struct sensorData {
  float pres;
  float temp;
} sensorData;

sensorData savedData[10];

byte customChar[8][8] {
  { 0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x1f
  },
  { 0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x1f,
    0x1f
  },
  { 0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x1f,
    0x1f,
    0x1f
  },
  { 0x00,
    0x00,
    0x00,
    0x00,
    0x1f,
    0x1f,
    0x1f,
    0x1f
  },
  { 0x00,
    0x00,
    0x00,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f
  },
  { 0x00,
    0x00,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f
  },
  { 0x00,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f
  },
  { 0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f,
    0x1f
  }
};


void writeLast(float pres, float temp, sensorData arr[10]);
sensorData minVals(sensorData arr[10]);
sensorData maxVals(sensorData arr[10]);

void setup() {
  curr_time = millis();
  prev_time = millis();
  for (int i = 0; i < 3; i++) {
    pinMode(btn[i], INPUT);
  }
  Serial3.begin(9600);               // the GPRS baud rate
  Serial.begin(9600);             // the Serial port of Arduino baud rate.
  disp.init();
  disp.backlight();
  disp.setCursor(6, 1);
  disp.print("Lyolik");
  disp.setCursor(7, 2);
  disp.print("team");
  delay(3000);
  disp.clear();
  disp.setCursor(6, 1);
  disp.print("Frcstr");
  disp.setCursor(5, 2);
  disp.print("v 0.0001");
  delay(3000);

  while (!bmp.begin()) {
    Serial.println("Cannot load barometer, trying again...");
    disp.setCursor(3, 3);
    disp.print("err:bmp280 404");
    delay(1000);
  }

  Serial.print("Device started");
  EEPROM_readAnything(0, savedData);
  EEPROM_readAnything(10, interval);
  for (int i = 0; i < 8; i++) {
    disp.createChar(i + 1, customChar[i]);
  }
  for (int i = 0; i < 10; i++) {
  Serial.print("[");
  Serial.print(savedData[i].pres);
  Serial.print(" ");
  Serial.print(savedData[i].temp);
  Serial.print("]");
  }
  Serial.println();
}

void checkIfRead() {
  curr_time = millis();
  if (curr_time - prev_time >= interval * 60000) {
    readSensor();
    prev_time = millis();
  }
}

void loop() {
  checkIfRead();

  if (menu == 0 ) {
    printMenu();
    menu = 1;
  }

  if (prev == -1) {
    printCursor(sel, prev);
    prev = sel;
  }

  int btnUp = digitalRead(btn[0]);
  int btnDwn = digitalRead(btn[1]);
  int btnSel = digitalRead(btn[2]);

  if (btnUp == HIGH && sel <= 3  && sel > 0) {
    Serial.println("UP");
    prev = sel;
    sel--;
    printCursor(sel , prev);
    delay(1000);
  }
  else if (btnDwn == HIGH && sel >= 0 && sel < 3) {
    Serial.println("DOWN");
    prev = sel;
    sel++;
    printCursor(sel, prev);
    delay(1000);
  }
  else if (btnSel == HIGH && menu) {
    loadAnim();
    switch (sel) {
      case 0:
        printPres(savedData);
        while (digitalRead(btn[2]) != HIGH) {
          checkIfRead();
        }
        Serial.println("STOP");
        break;
      case 1:
        printTemp(savedData);
        while (digitalRead(btn[2]) != HIGH) {
          checkIfRead();
        }
        Serial.println("STOP");
        break;
      case 2:
        disp.clear();
        disp.setCursor(2, 1);
        disp.print("Sending report...");
        sendReport();
        break;
      case 3:
        disp.clear();
        settings();
        break;
    }
    menu = 0;
    prev = -1;
    loadAnim();
  }
}

void clearLine(int row) {
  for (int i = 0; i < 20; i++) {
    disp.setCursor(i, row);
    disp.print(" ");
  }
}


void settings() {
  disp.clear();
  disp.setCursor(1, 1);
  disp.print("Set time interval");
  disp.setCursor(5, 2);
  disp.print(interval);
  disp.print(" min");
  while (1) {
    checkIfRead();
    if (digitalRead(btn[0]) == HIGH) {
      if (interval + 10 > 60) {
        disp.setCursor(1, 3);
        disp.print("max value reached");
      }
      else {
        interval += 10;
        clearLine(2);
        clearLine(3);
        disp.setCursor(5, 2);
        disp.print(interval);
        disp.print(" min");
      }
    } else if (digitalRead(btn[1]) == HIGH) {
      if (interval - 10 < 10) {
        disp.setCursor(1, 3);
        disp.print("min value reached");
      }
      else {
        interval -= 10;
        clearLine(2);
        clearLine(3);
        disp.setCursor(5, 2);
        disp.print(interval);
        disp.print(" min");
      }
    } else if (digitalRead(btn[2]) == HIGH) {
      disp.clear();
      disp.setCursor(0, 1);
      disp.print("Value set to ");
      disp.print(interval);
      EEPROM_writeAnything(10, interval);
      disp.print(" min");
      delay(3000);
      return;
    }
    delay(300);
  }
}

void loadAnim() {
  disp.clear();
  disp.setCursor(5, 1);
  disp.print("Loading...");
  for (int i = 1; i <= 8; i++) {
    disp.setCursor(4 + i, 2);
    disp.write(i);
    delay(200);
  }
}

void printMenu() {
  disp.clear();
  disp.setCursor(1, 0);
  disp.print("Pres graph");
  disp.setCursor(1, 1);
  disp.print("Temp graph");
  disp.setCursor(1, 2);
  disp.print("Send report");
  disp.setCursor(1, 3);
  disp.print("Settings");
}

void printCursor(int selected, int prev) {
  if (selected != -1) {
    disp.setCursor(0, prev);
    disp.print(" ");
  }
  disp.setCursor(0, selected);
  disp.print("+");
}

void printTemp(sensorData savedData[10]) {
  disp.clear();
  sensorData min = minVals(savedData);
  sensorData max = maxVals(savedData);
  sensorData averData = avg(savedData);
  int minTemp = min.temp * 100;
  minTemp %= 100;
  int maxTemp = max.temp * 100;
  maxTemp %= 100;
  Serial.println("Temp for graph");
  for (int i = 0; i < 10; i++) {
    int temp = (savedData[i].temp * 100);
    temp = temp % 100;
    Serial.print(temp);
    Serial.print(" ");
    drawCol(i, temp, minTemp , maxTemp);
  }
  disp.setCursor(10, 0);
  disp.print("Temp C");
  disp.setCursor(10, 1);
  disp.print("min:");
  disp.print(min.temp);
  disp.setCursor(10, 2);
  disp.print("max:");
  disp.print(max.temp);
  disp.setCursor(10, 3);
  disp.print("avg:");
  disp.print(averData.temp);
}

void printPres(sensorData savedData[10]) {
  disp.clear();
  sensorData min = minVals(savedData);
  sensorData max = maxVals(savedData);
  sensorData averData = avg(savedData);

  for (int i = 0; i < 10; i++) {
    drawCol(i, (int) savedData[i].pres % 1000, (int) min.pres % 1000 , (int) max.pres % 1000);
  }
  for (int i = 0; i < 10; i++) {
    Serial.print(savedData[i].pres);
    Serial.print(" ");
  }
  Serial.println();
  disp.setCursor(10, 0);
  disp.print("Pres kPa");
  disp.setCursor(10, 1);
  disp.print("min:");
  disp.print(min.pres / 1000);
  disp.setCursor(10, 2);
  disp.print("max:");
  disp.print(max.pres / 1000);
  disp.setCursor(10, 3);
  disp.print("avg:");
  disp.print(averData.pres / 1000);
}

sensorData avg(sensorData arr[10]) {
  float avgPres = 0;
  float avgTemp = 0.0;
  Serial.println("Average:");
  for (int i = 0; i < 10; i++) {
    Serial.print(arr[i].pres);
    Serial.print(" avg:");
    avgPres += arr[i].pres;
    avgTemp += arr[i].temp;
    Serial.println(avgPres);
  }
  Serial.println("avgPres");
  Serial.println(avgPres);
  avgPres /= 10;
  avgTemp /= 10;
  sensorData res = {avgPres, avgTemp};
  return res;
}

void drawCol(int col, int val, int max, int min) {
  int mapped = map(val, min, max, 32, 1);
  int bars = mapped / 8;
  int rem = mapped % 8;

  for ( int i = 3; i >= 4 - bars ; i-- ) {
    disp.setCursor(col, i);
    disp.write(8);
  }
  if (rem != 0) {
    disp.setCursor(col, 3 - bars);
    disp.write(rem);
  }
}

void readSensor() {
  Serial.println();
  float pres = bmp.readPressure();
  float temp =  bmp.readTemperature();
  for (int i = 0; i < 9; i++) {
    delay(300);
    pres += bmp.readPressure();
    temp += bmp.readTemperature();
  }
  pres /= 10.0;
  temp /= 10.0;
  writeLast(pres, temp, savedData);
  Serial.println(pres);
  Serial.println(temp);
  EEPROM_writeAnything(0, savedData);

  int back = 60 / interval;
  slope = savedData[9].pres - savedData[9 - back].pres;
}

void writeLast(float pres, float temp, sensorData arr[10]) {
  for ( int i = 0; i < 9; i++) {
    arr[i] = arr[i + 1];
  }
  arr[9].pres = pres;
  arr[9].temp = temp;
}

sensorData minVals(sensorData arr[10]) {
  float minPres = arr[0].pres;
  float minTemp = arr[0].temp;
  for (int i = 0; i < 10; i++) {
    if (minPres > arr[i].pres) minPres = arr[i].pres;
    if (minTemp > arr[i].temp) minTemp = arr[i].temp;
  }
  sensorData res = {minPres, minTemp};
  return res;
}

sensorData maxVals(sensorData arr[10]) {
  float maxPres = arr[0].pres;
  float maxTemp = arr[0].temp;
  for (int i = 0; i < 10; i++) {
    if (maxPres < arr[i].pres) maxPres = arr[i].pres;
    if (maxTemp < arr[i].temp) maxTemp = arr[i].temp;
  }
  sensorData res = {maxPres, maxTemp};
  return res;
}

void sendReport() {
  sensorData average = avg(savedData);
  float pres = bmp.readPressure();
  float temp =  bmp.readTemperature();
  for (int i = 0; i < 9; i++) {
    delay(300);
    pres += bmp.readPressure();
    temp += bmp.readTemperature();
  }
  pres /= 10.0;
  temp /= 10.0;
  String msg = "Report from Frkstr\nAvegare ";
  //msg += interval;
  msg += "/\nTemp:";
  msg += average.temp;
  msg += " C\nPres:";
  msg += average.pres / 1000.0;
  msg += "kPa\nCurrent\nTemp:";
  msg += temp;
  msg += " C\nPres:";
  msg += pres / 1000.0;
  msg += "kPa\nProbability of rain:";
  msg += (slope / 300.0) * 100;
  msg += "%";
  Serial3.println(msg);
}
