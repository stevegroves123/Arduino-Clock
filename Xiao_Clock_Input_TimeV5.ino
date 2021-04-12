/*
   Clock with alarm, analog or digital, light and display off
    Steven Groves
    April 2021
*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <RTCZero.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
RTCZero rtc;

unsigned long previousMillis = 0;

uint8_t h = 0;
uint8_t m = 0;
uint8_t s = 0;
uint8_t dy = 1;
uint8_t mth = 1;
uint8_t yr = 20;
uint8_t event;
uint8_t alarmh = 0;
uint8_t alarmm = 0;


bool exitHours = false;
bool exitMins = false;
bool exitDay = false;
bool exitMonth = false;
bool exitYear = false;
bool alarmActive = false;
bool flashDisplay = false;
bool digitalActive = false;

const long interval = 250;
// START OF ANALOG CLOCK
const int MIDDLE_Y = u8g2.getDisplayHeight() / 2;
const int MIDDLE_X = u8g2.getDisplayWidth() / 2;
/* Constants for the clock face */
int CLOCK_RADIUS;
int POS_12_X, POS_12_Y;
int POS_3_X, POS_3_Y;
int POS_6_X, POS_6_Y;
int POS_9_X, POS_9_Y;
int S_LENGTH;
int M_LENGTH;
int H_LENGTH;

unsigned long lastDraw = 0;

void initClockVariables()
{
  // Calculate constants for clock face component positions:

  CLOCK_RADIUS = min(MIDDLE_X, MIDDLE_Y) - 1;

  POS_12_X = MIDDLE_X - 6;
  POS_12_Y = MIDDLE_Y - CLOCK_RADIUS + 12;
  POS_3_X  = MIDDLE_X + CLOCK_RADIUS - 10;
  POS_3_Y  = MIDDLE_Y + 5;
  POS_6_X  = MIDDLE_X - 2;
  POS_6_Y  = MIDDLE_Y + CLOCK_RADIUS - 2;
  POS_9_X  = MIDDLE_X - CLOCK_RADIUS + 4;
  POS_9_Y  = MIDDLE_Y + 5;

  // Calculate clock arm lengths
  S_LENGTH = CLOCK_RADIUS - 2;
  M_LENGTH = S_LENGTH * 0.75;
  H_LENGTH = S_LENGTH * 0.5;
}

void drawArms(int h, int m, int s)
{
  double midHours;  // this will be used to slightly adjust the hour hand
  static int hx, hy, mx, my, sx, sy;

  // Adjust time to shift display 90 degrees ccw
  // this will turn the clock the same direction as text:

  h -= 3;
  m -= 15;
  s -= 15;
  if (h <= 0)
    h += 12;
  if (m < 0)
    m += 60;
  if (s < 0)
    s += 60;
  // Calculate and draw new lines:
  s = map(s, 0, 60, 0, 360);  // map the 0-60, to "360 degrees"
  sx = S_LENGTH * cos(PI * ((float)s) / 180);  // woo trig!
  sy = S_LENGTH * sin(PI * ((float)s) / 180);  // woo trig!
  // draw the second hand:
  u8g2.drawLine(MIDDLE_X, MIDDLE_Y, MIDDLE_X + sx, MIDDLE_Y + sy);

  m = map(m, 0, 60, 0, 360);  // map the 0-60, to "360 degrees"
  mx = M_LENGTH * cos(PI * ((float)m) / 180);  // woo trig!
  my = M_LENGTH * sin(PI * ((float)m) / 180);  // woo trig!
  // draw the minute hand
  u8g2.drawLine(MIDDLE_X, MIDDLE_Y, MIDDLE_X + mx, MIDDLE_Y + my);

  midHours = int(rtc.getMinutes()) / 12; // midHours is used to set the hours hand to middling levels between whole hours
  h *= 5;  // Get hours and midhours to the same scale
  h += midHours;  // add hours and midhours
  h = map(h, 0, 60, 0, 360);  // map the 0-60, to "360 degrees"
  hx = H_LENGTH * cos(PI * ((float)h) / 180);  // woo trig!
  hy = H_LENGTH * sin(PI * ((float)h) / 180);  // woo trig!
  // draw the hour hand:
  u8g2.drawLine(MIDDLE_X, MIDDLE_Y, MIDDLE_X + hx, MIDDLE_Y + hy);
}

// Draw an analog clock face
void drawFace()
{
  // Draw the clock border
  u8g2.drawCircle(MIDDLE_X, MIDDLE_Y, CLOCK_RADIUS, U8G2_DRAW_ALL);
  // Draw the clock numbers
  u8g2.setCursor(POS_12_X, POS_12_Y); // points cursor to x=27 y=0
  u8g2.print("12");
  u8g2.setCursor(POS_6_X, POS_6_Y);
  u8g2.print("6");
  u8g2.setCursor(POS_9_X, POS_9_Y);
  u8g2.print("9");
  u8g2.setCursor(POS_3_X, POS_3_Y);
  u8g2.print("3");
}

void setup() {
  u8g2.begin(/*Select=*/ 0, /*Right/Next=*/ 1, /*Left/Prev=*/ 2, /*Up=*/ 4, /*Down=*/ 3, /*Home/Cancel=*/ A6);
  // ONLY 0, 1, 2 used in this project
  rtc.begin();
  // Enter the time
  updatedTimeHMS();
  rtc.setSeconds(int(s));
  // Display the time
  displayTime();
}

void loop() {
  if (digitalActive == false) {
    displayTime();
  } else {
    displayDigital();
  }

  int key = u8g2.getMenuEvent();
  u8g2.setFont(u8g2_font_7x13_tf);
  if (key == U8X8_MSG_GPIO_MENU_SELECT) //select enter to go into update time option
  {
    int key2 = u8g2.userInterfaceMessage("UPDATE TIME", "", "", " Yes \n No ");
    if (key2 == 1 ) // if YES is selcted
    {
      updatedTimeHMS();
    }
    if (key2 == 2 ) // if NO is selected
    {
      u8g2.setFont(u8g2_font_7x13_tf);
      int key3 = u8g2.userInterfaceSelectionList("Menu", 1, "Clock\nAlarm\nLight\nTest\nOff\nAdvert\nDigital");
      u8g2.setFontRefHeightAll();

      if (key3 == 1) { // Alalog option
        displayTime();
      }

      if (key3 == 2) { // Alarm option
        alarmActive = true;
        updatedTimeHMS();
        rtc.setAlarmTime(alarmh, alarmm, s);
        rtc.enableAlarm(rtc.MATCH_HHMMSS);
        rtc.attachInterrupt(alarmActivated);
        
        u8g2.clearBuffer();
        u8g2.clearDisplay();
        u8g2.drawStr(5, 22, "-----------------");
        u8g2.drawStr(5, 32, "| Alarm set for |");
        u8g2.drawStr(5, 44, "|");
        u8g2.setCursor(50, 44);
        addZero(alarmh);
        u8g2.print(":");
        addZero(alarmm);
        u8g2.drawStr(117, 44, "|");
        u8g2.drawStr(5, 52, "-----------------");
        u8g2.sendBuffer();
        delay(2000);
      }

      if (key3 == 3) { // light option
        u8g2.clearBuffer();
        u8g2.clearDisplay();
        u8g2.setDrawColor(1);
        do {
          u8g2.drawBox(0, 0, 130, 65);
          u8g2.sendBuffer();
        } while ( u8g2.getMenuEvent() == 0 ); // wait for key press
      }
      if (key3 == 4) { // Test option
        u8g2.clearBuffer();
        u8g2.clearDisplay();
        char s[2] = " ";
        uint8_t x, y;
        for ( y = 0; y < 6; y++ ) {
          for ( x = 0; x < 16; x++ ) {
            s[0] = y * 16 + x + 32;
            u8g2.drawStr(x * 7, y * 10 + 10, s);
            u8g2.sendBuffer();
          }
        }
        delay(2000);  // leave test pattern on the screen for 2 seconds
      }
      if (key3 == 5) { // Off option
        do {
          u8g2.clearBuffer();
          u8g2.clearDisplay();
          u8g2.sendBuffer();
        } while ( u8g2.getMenuEvent() == 0 ); // wait for key press
      }
      if (key3 == 6) { // Advert option
        u8g2.clearBuffer();
        u8g2.clearDisplay();
        u8g2.drawStr(10, 22, "----------------");
        u8g2.drawStr(10, 32, "| Steve Groves |");
        u8g2.drawStr(10, 44, "|  April 2021  |");
        u8g2.drawStr(10, 52, "----------------");
        u8g2.sendBuffer();
        delay(3000);  // leave the avert on the screen for 3 seconds
      }
      if (key3 == 7) {  // Digital display
        digitalActive = true;
      }
    }
  }
  flashAlarm(); // check to see if alarm active, if so flash the display
}

void displayDigital() {
  u8g2.setFont(u8g2_font_inb19_mf);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 35);
  // time
  addZero(rtc.getHours());
  u8g2.print(":");
  addZero(rtc.getMinutes());
  u8g2.print(":");
  addZero(rtc.getSeconds());
  // date
  u8g2.setFont(u8g2_font_crox3c_mf);
  u8g2.setCursor(20, 54);
  addZero(rtc.getDay());
  u8g2.print("/");
  addZero(rtc.getMonth());
  u8g2.print("/");
  addZero(rtc.getYear());
  u8g2.sendBuffer();
}

void addZero(int addZero) {
  if (addZero < 10) {
    u8g2.print("0");
  }
  u8g2.print(addZero);
}

void updatedTimeHMS() {
  do {
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.setFontRefHeightAll();
    if (alarmActive == false) {
      u8g2.userInterfaceInputValue("Update Time", "Hours = ", &h, 0, 23, 2, "");
    } else {
      u8g2.userInterfaceInputValue("Alarm Update", "Hours = ", &h, 0, 23, 2, "");
      alarmh = h;
    }
    if ( event == U8X8_MSG_GPIO_MENU_SELECT )
    {
      exitHours = true;
    }
  } while (exitHours = false);

  do {
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.setFontRefHeightAll();
    if (alarmActive == false) {
      u8g2.userInterfaceInputValue("Update Time", "Minutes = ", &m, 0, 59, 2, "");
    } else {
      u8g2.userInterfaceInputValue("Alarm Update", "Minutes = ", &m, 0, 59, 2, "");
      alarmm = m;
    }
    if ( event == U8X8_MSG_GPIO_MENU_SELECT )
    {
      exitMins = true;
    }
  } while (exitMins = false);

  do {
    if (alarmActive == true) 
    {
      break; // Don't set the date if changing the alarm time
    }
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.setFontRefHeightAll();
    u8g2.userInterfaceInputValue("Update Clock", "Day = ", &dy, 1, 31, 2, "");
    if ( event == U8X8_MSG_GPIO_MENU_SELECT )
    {
      exitDay = true;
    }
  } while (exitDay = false );

  do {
    if (alarmActive == true) 
    {
      break;  // Don't set the date if changing the alarm time
    }
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.setFontRefHeightAll();
    u8g2.userInterfaceInputValue("Update Clock", "Month = ", &mth, 1, 31, 2, "");
    if ( event == U8X8_MSG_GPIO_MENU_SELECT )
    {
      exitMonth = true;
    }
  } while (exitMonth = false);

  do {
    if (alarmActive == true) 
    {
      break;  // Don't set the date if changing the alarm time
    }
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.setFontRefHeightAll();
    u8g2.userInterfaceInputValue("Update Clock", "Year = ", &yr, 20, 99, 2, "");
    if ( event == U8X8_MSG_GPIO_MENU_SELECT )
    {
      exitYear = true;
    }
  } while (exitYear = false);

  if (alarmActive == false ) { // if updating time then set the hours and minutes
    rtc.setHours(int(h));
    rtc.setMinutes(int(m));
    rtc.setDay(int(dy));
    rtc.setMonth(int(mth));
    rtc.setYear(int(yr));
  }
}

void displayTime()
{
  digitalActive = false;
  u8g2.clearBuffer();
  initClockVariables();
  drawFace();
  drawArms(int(rtc.getHours()), int(rtc.getMinutes()), int(rtc.getSeconds()));
  u8g2.sendBuffer();
}

// Alarm section
void flashAlarm() {
  if (flashDisplay == true ) {
    do
    {
      unsigned long currentMillis = millis();
      u8g2.clearBuffer();
      u8g2.sendBuffer();
      if (currentMillis - previousMillis  >= interval) {
        previousMillis = currentMillis;
        u8g2.drawBox(0, 0, 128, 64);
        u8g2.sendBuffer();
      }
    } while ( u8g2.getMenuEvent() == 0 ); // leave in alarm mode until enter key pressed
    flashDisplay = false;
  }
}

void alarmActivated()
{
  flashDisplay = true;
}
