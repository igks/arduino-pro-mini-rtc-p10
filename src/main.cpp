#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <DMD2.h>
#include <DS3231.h>
#include <fonts/SystemFont5x7.h>
#include <ezButton.h>

SoftDMD dmd(1, 1);
RTClib Calendar;
DS3231 RTC;
ezButton setButton(10);
ezButton incButton(12);

char weekDay[][7] = {"Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu", "Minggu"};
char Month[][4] = {"Jan", "Peb", "Mar", "Apr", "Mei", "Jun", "Jul", "Agu", "Sep", "Okt", "Nov", "Des"};
int Line[8][4] = {
    {15, 1, 15, 3},
    {17, 1, 15, 3},
    {17, 3, 15, 3},
    {17, 5, 15, 3},
    {15, 3, 15, 5},
    {13, 5, 15, 3},
    {13, 3, 15, 3},
    {13, 1, 15, 3}};

int Images[16][7] = {
    {-1, -1, 16, 19, 20, 21, -1},
    {13, 15, 18, 19, 20, -1, -1},
    {14, 15, 16, 17, 18, 19, -1},
    {9, 10, 11, -1, -1, -1, -1},
    {8, 12, 17, 18, 19, -1, -1},
    {7, -1, 13, 16, 20, -1, -1},
    {6, 13, 15, 21, -1, -1, -1},
    {13, 15, 22, -1, -1, -1, -1},
    {8, 13, 15, 22, -1, -1, -1},
    {9, 12, 15, 22, -1, -1, -1},
    {10, 11, 12, 14, 22, -1, -1},
    {-1, 13, 22, -1, -1, -1, -1},
    {6, 14, 22, -1, -1, -1, -1},
    {7, 14, 16, 21, -1, -1, -1},
    {8, 14, 17, 20, -1, -1, -1},
    {9, 10, 11, 12, 13, 18, 19}};

int messageStart = 32;
String Message = "";
int dotPeriod = 500;
int textPeriod = 150;
int clockPeriod = 1000;
int refreshPeriod = 1000;
unsigned long dotTime = 0;
unsigned long textTime = 0;
unsigned long clockTime = 0;
unsigned long refreshTime = 0;

int lineIndex = 0;

int newHour = 0;
int newMinute = 0;
int newDate = 0;
int day = 0;
int newMonth = 0;
int newYear = 0;
bool isDrawImage = false;
int imageCounter = 0;
int clockCounter = 0;

String getString(int value)
{
  if (value < 10)
  {
    return "0" + String(value);
  }
  else
  {
    return String(value);
  }
}

void updateTime()
{
  DateTime now = Calendar.now();
  day = RTC.getDoW();
  newHour = now.hour();
  newMinute = now.minute();
  newDate = now.day();
  newMonth = now.month();
  newYear = now.year();
}

void updateMessage()
{
  Message = String(weekDay[day - 1]) + "," + String(newDate) + "-" + String(Month[newMonth - 1]) + "-" + String(newYear);
}

void refresh()
{
  if ((unsigned long)(millis() - refreshTime > refreshPeriod))
  {
    refreshTime = millis();
    updateTime();
    updateMessage();

    if (isDrawImage == true)
    {
      clockCounter += 1;

      if (clockCounter >= 60)
      {
        clockCounter = 0;
        imageCounter = 0;
        dmd.clearScreen();
        isDrawImage = false;
      }
    }
    else
    {
      imageCounter += 1;

      if (imageCounter >= 300)
      {
        imageCounter = 0;
        clockCounter = 0;
        dmd.clearScreen();
        isDrawImage = true;
      }
    }
  }
}

void showMessage()
{
  if ((unsigned long)(millis() - textTime > textPeriod))
  {
    textTime = millis();
    dmd.drawString(messageStart, 8, Message);
    messageStart--;
    int messageLength = Message.length() * 5;
    if (messageStart <= -(messageLength + (messageLength / 5)))
    {
      messageStart = 32;
    }
  }
}

void showClock(int newHour, int newMinute)
{
  if ((unsigned long)(millis() - clockTime) > clockPeriod)
  {
    clockTime = millis();
    dmd.drawString(1, 0, String(getString(newHour)));
    dmd.drawString(19, 0, String(getString(newMinute)));
  }
}

void showLine()
{
  if ((unsigned long)(millis() - dotTime) > dotPeriod)
  {
    dotTime = millis();
    dmd.drawFilledBox(13, 0, 17, 5, GRAPHICS_OFF);
    dmd.drawLine(Line[lineIndex][0], Line[lineIndex][1], Line[lineIndex][2], Line[lineIndex][3]);
    lineIndex++;
    if (lineIndex > 7)
    {
      lineIndex = 0;
    }
  }
}

void drawImage()
{
  for (int x = 0; x < 16; x++)
  {
    for (int y = 0; y < 7; y++)
    {
      dmd.setPixel(Images[x][y], x);
    }
  }
}

void initialSetTime()
{
  // YY = Year (ex. 20 for 2020)");
  // MM = Month (ex. 04 for April)");
  // DD = Day of month (ex. 09 for 9th)");
  // w  = Day of week from 1 to 7, (ex. 2 for Tuesday)");
  // hh = hours in 24h format (ex. 09 for 9AM or 21 for 9PM)");
  // mm = minutes (ex. 02)");
  // ss = seconds (ex. 42)");

  RTC.setYear(23);
  RTC.setMonth(01);
  RTC.setDate(31);
  RTC.setDoW(2);
  RTC.setHour(20);
  RTC.setMinute(21);
  RTC.setSecond(00);
  delay(2000);
}

int setMode = 0;
int hourBuff = 0;
int minBuff = 0;
int dayBuff = 0;
int dateBuff = 1;
int monthBuff = 1;
int yearBuff = 20;

void setHour()
{
  dmd.drawString(1, 0, "Hour");
  dmd.drawString(1, 8, String(hourBuff));
  if (incButton.isReleased())
  {
    if (hourBuff < 23)
    {
      hourBuff++;
    }
    else
    {
      hourBuff = 0;
    }
  }
}

void setMin()
{
  dmd.drawString(1, 0, "Minute");
  dmd.drawString(1, 8, String(minBuff));
  if (incButton.isReleased())
  {
    if (minBuff < 59)
    {
      minBuff++;
    }
    else
    {
      minBuff = 0;
    }
  }
}

void setDay()
{
  dmd.drawString(1, 0, "Day");
  dmd.drawString(1, 8, String(weekDay[dayBuff - 1]));
  if (incButton.isReleased())
  {
    if (dayBuff < 7)
    {
      dayBuff++;
    }
    else
    {
      dayBuff = 0;
    }
  }
}

void setDate()
{
  dmd.drawString(1, 0, "Date");
  dmd.drawString(1, 8, String(dateBuff));
  if (incButton.isReleased())
  {
    if (dateBuff < 31)
    {
      dateBuff++;
    }
    else
    {
      dateBuff = 1;
    }
  }
}

void setMonth()
{
  dmd.drawString(1, 0, "Month");
  dmd.drawString(1, 8, String(Month[monthBuff - 1]));
  if (incButton.isReleased())
  {
    if (monthBuff < 12)
    {
      monthBuff++;
    }
    else
    {
      monthBuff = 0;
    }
  }
}

void setYear()
{
  dmd.drawString(1, 0, "Year");
  dmd.drawString(1, 8, String(yearBuff));
  if (incButton.isReleased())
  {
    yearBuff++;
  }
}

void setTimeFinish()
{
  dmd.drawString(1, 0, "Finish");
  RTC.setYear(yearBuff);
  RTC.setMonth(monthBuff);
  RTC.setDate(dateBuff);
  RTC.setDoW(dayBuff);
  RTC.setHour(hourBuff);
  RTC.setMinute(minBuff);
  RTC.setSecond(00);
  setMode = 0;
  dmd.clearScreen();
  delay(1000);
}

void setup()
{
  dmd.setBrightness(70);
  dmd.selectFont(SystemFont5x7);
  dmd.begin();
  Wire.begin();
  setButton.setDebounceTime(50);
  incButton.setDebounceTime(50);

  // initialSetTime();

  updateTime();
  updateMessage();
}

void loop()
{
  setButton.loop();
  incButton.loop();

  if (setButton.isReleased())
  {
    setMode++;
    dmd.clearScreen();
  }

  if (setMode == 0)
  {
    refresh();

    if (isDrawImage == true)
    {
      drawImage();
    }
    else
    {
      showMessage();
      showClock(newHour, newMinute);
      showLine();
    }
  }
  else
  {
    switch (setMode)
    {
    case 1:
      setYear();
      break;
    case 2:
      setMonth();
      break;
    case 3:
      setDate();
      break;
    case 4:
      setDay();
      break;
    case 5:
      setHour();
      break;
    case 6:
      setMin();
      break;
    case 7:
      setTimeFinish();
      break;
    default:
      break;
    }
  }
}