/* FLASH SETTINGS
Board: LOLIN D32 (For ESP32_C3_SM use ESP32C3 Dev Module. For MARAUDER_TTGO_TDISPLAY use LOLIN D32. For MARAUDER_S2MINI use LOLIN S2 Mini)
For MARAUDER_S2MINI: USB CDC on boot (disabled)
Flash Frequency: 80MHz
Partition Scheme: Minimal SPIFFS
https://www.online-utility.org/image/convert/to/XBM
*/

#include "configs.h"

#ifndef HAS_SCREEN
  #define MenuFunctions_h
  #define Display_h
#endif

#include <WiFi.h>
#include "EvilPortal.h"
#include <Wire.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <Arduino.h>

#ifdef HAS_GPS
  #include "GpsInterface.h"
#endif

#include "Assets.h"
#include "WiFiScan.h"
#ifdef HAS_SD
  #include "SDInterface.h"
#endif
#include "Buffer.h"

#ifdef HAS_FLIPPER_LED
  #include "flipperLED.h"
#elif defined(XIAO_ESP32_S3)
  #include "xiaoLED.h"
#elif defined(MARAUDER_TTGO_TDISPLAY) || defined(MARAUDER_M5STICKC) || defined(MARAUDER_M5STICKCP2)
  #include "stickcLED.h"
#elif defined(HAS_NEOPIXEL_LED)
  #include "LedInterface.h"
#endif

#include "settings.h"
#include "CommandLine.h"
#include "lang_var.h"

#ifdef HAS_BATTERY
  #include "BatteryInterface.h"
#endif

#ifdef HAS_SCREEN
  #include "Display.h"
  #include "MenuFunctions.h"
#endif

#ifdef HAS_BUTTONS
  #include "Switches.h"
  
  #if (U_BTN >= 0)
    Switches u_btn = Switches(U_BTN, 1000, U_PULL);
  #else
    // Prevent linker error for boards without U_BTN (e.g. TTGO T-Display)
    Switches u_btn = Switches(-1, 2000, false);
  #endif
  #if (D_BTN >= 0)
    Switches d_btn = Switches(D_BTN, 1000, D_PULL);
  #endif
  #if (L_BTN >= 0)
    Switches l_btn = Switches(L_BTN, 1000, L_PULL);
  #endif
  #if (R_BTN >= 0)
    Switches r_btn = Switches(R_BTN, 1000, R_PULL);
  #endif
  #if (C_BTN >= 0)
    Switches c_btn = Switches(C_BTN, 1000, C_PULL);
  #endif

#endif

WiFiScan wifi_scan_obj;
EvilPortal evil_portal_obj;
Buffer buffer_obj;
Settings settings_obj;
CommandLine cli_obj;

// Added for deep sleep when using the TTGO T-Display
#define BUTTON_PIN D_BTN // Button 0 pin
unsigned long buttonPressStart = 0; 
bool buttonHeld = false;

#ifdef HAS_GPS
  GpsInterface gps_obj;
#endif

#ifdef HAS_BATTERY
  BatteryInterface battery_obj;
#endif

#ifdef HAS_SCREEN
  Display display_obj;
  MenuFunctions menu_function_obj;
#endif

#ifdef HAS_SD
  SDInterface sd_obj;
#endif

#ifdef MARAUDER_M5STICKC
  AXP192 axp192_obj;
#endif

#ifdef HAS_FLIPPER_LED
  flipperLED flipper_led;
#elif defined(XIAO_ESP32_S3)
  xiaoLED xiao_led;
#elif defined(MARAUDER_TTGO_TDISPLAY) || defined(MARAUDER_M5STICKC) || defined(MARAUDER_M5STICKCP2)
  stickcLED stickc_led;
#else
  LedInterface led_obj;
#endif

const String PROGMEM version_number = MARAUDER_VERSION;

#ifdef HAS_NEOPIXEL_LED
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(Pixels, PIN, NEO_GRB + NEO_KHZ800);
#endif

uint32_t currentTime  = 0;

void backlightOn() {
  #ifdef HAS_SCREEN
    #ifdef MARAUDER_MINI
      digitalWrite(TFT_BL, LOW);
    #endif
  
    #ifndef MARAUDER_MINI
      digitalWrite(TFT_BL, HIGH);
    #endif
  #endif
}

void backlightOff() {
  #ifdef HAS_SCREEN
    #ifdef MARAUDER_MINI
      digitalWrite(TFT_BL, HIGH);
    #endif
  
    #ifndef MARAUDER_MINI
      digitalWrite(TFT_BL, LOW);
    #endif
  #endif
}

void setup()
{
  #if defined(MARAUDER_TTGO_TDISPLAY)
    pinMode(D_BTN, INPUT_PULLUP); // Configure button as input for deep sleep TTGO Tdisplay
  #endif

  #ifndef ESP32_C3_SM
    esp_spiram_init();  // Enabled for all except ESP32_C3_SM
    #else
    // esp_spiram_init();  // Disabled for ESP32_C3_SM
  #endif

  #ifdef defined(MARAUDER_M5STICKC) && !defined(MARAUDER_M5STICKCP2)
    axp192_obj.begin();
  #endif

  #if defined(MARAUDER_M5STICKCP2) // Prevent StickCP2 from turning off when disconnect USB cable
    pinMode(POWER_HOLD_PIN, OUTPUT);
    digitalWrite(POWER_HOLD_PIN, HIGH);
  #endif
  
  #ifdef HAS_SCREEN
    pinMode(TFT_BL, OUTPUT);
  #endif
  
  backlightOff();
  #if BATTERY_ANALOG_ON == 1
    pinMode(BATTERY_PIN, OUTPUT);
    pinMode(CHARGING_PIN, INPUT);
  #endif
  
  // Preset SPI CS pins to avoid bus conflicts
  #ifdef HAS_SCREEN
    digitalWrite(TFT_CS, HIGH);
  #endif
  
  #ifdef HAS_SD
    pinMode(SD_CS, OUTPUT);

    delay(10);
  
    digitalWrite(SD_CS, HIGH);

    delay(10);
  #endif

  #ifdef ESP32_C3_SM
    Serial.begin(115200, SERIAL_8N1, 21, 20); // RX = GPIO21, TX = GPIO20 for ESP32C3SuperMini
  #elif defined(MARAUDER_S2MINI)
    Serial.begin(115200, SERIAL_8N1, 17, 18); // RX = GPIO17, TX = GPIO18 // modified by Davide Gatti www.survivalhacking.it to work with Wemos S2 MINI board
  #else
    Serial.begin(115200);
  #endif

  while(!Serial)
    delay(10);

  Serial.println("ESP-IDF version is: " + String(esp_get_idf_version()));

  #ifdef HAS_PSRAM
    if (psramInit()) {
      Serial.println("PSRAM is correctly initialized");
    } else {
      Serial.println("PSRAM not available");
    }
  #endif

  #ifdef HAS_SCREEN
    display_obj.RunSetup();
    display_obj.tft.setTextColor(TFT_WHITE, TFT_BLACK);
  #endif

  backlightOff();

  // Draw the title screen
  /*
  #ifdef HAS_SCREEN
    #ifndef MARAUDER_MINI
      display_obj.drawJpeg("/marauder3L.jpg", 0 , 0);     // 240 x 320 image
    #else
      display_obj.drawJpeg("/marauder3L.jpg", 0, 0);
    #endif
  #endif
  */

  #ifdef HAS_SCREEN
    #ifdef MARAUDER_TTGO_TDISPLAY
      display_obj.tft.drawCentreString("ESP32 Marauder", TFT_WIDTH / 1.1, TFT_HEIGHT * 0.10, 1);
      display_obj.tft.drawCentreString("JustCallMeKoko", TFT_WIDTH / 1.1, TFT_HEIGHT * 0.15, 1);
      display_obj.tft.drawCentreString(display_obj.version_number, TFT_WIDTH / 1.1, TFT_HEIGHT * 0.20, 1);
      display_obj.tft.setTextColor(TFT_VIOLET, TFT_BLACK);
      display_obj.tft.drawCentreString("Ported by ATOMNFT", TFT_WIDTH / 1.1, TFT_HEIGHT * 0.27, 1);
    #else
      display_obj.tft.drawCentreString("ESP32 Marauder", TFT_WIDTH / 2, TFT_HEIGHT * 0.33, 1);
      display_obj.tft.drawCentreString("JustCallMeKoko", TFT_WIDTH / 2, TFT_HEIGHT * 0.5, 1);
      display_obj.tft.drawCentreString(display_obj.version_number, TFT_WIDTH / 2, TFT_HEIGHT * 0.66, 1);
    #endif
  #endif

  backlightOn(); // Need this

  #ifdef HAS_SCREEN
    //delay(2000);

    // Do some stealth mode stuff
    #ifdef HAS_BUTTONS
      if (c_btn.justPressed()) {
        display_obj.headless_mode = true;

        backlightOff();

        Serial.println("Headless Mode enabled");
      }
    #endif

    //display_obj.clearScreen();
  
    //display_obj.tft.setTextColor(TFT_CYAN, TFT_BLACK);
  
    //display_obj.tft.println(text_table0[0]);
  
    //delay(2000);
  
    //display_obj.tft.println("Marauder " + display_obj.version_number + "\n");
  
    //display_obj.tft.println(text_table0[1]);
  #endif

  settings_obj.begin();

  wifi_scan_obj.RunSetup();

  //#ifdef HAS_SCREEN
  //  display_obj.tft.println(F(text_table0[2]));
  //#endif

  buffer_obj = Buffer();
  #if defined(HAS_SD)
    // Do some SD stuff
    if(!sd_obj.initSD())
      Serial.println(F("SD Card NOT Supported"));

  #endif

  #ifdef HAS_SCREEN
    display_obj.tft.setTextColor(TFT_GREEN, TFT_BLACK);
    display_obj.tft.drawCentreString("Initializing...", TFT_WIDTH/2, TFT_HEIGHT * 0.82, 1);
  #endif

  evil_portal_obj.setup();

  #ifdef HAS_BATTERY
    battery_obj.RunSetup();
  #endif
  
  #ifdef HAS_SCREEN
    //display_obj.tft.println(F(text_table0[5]));
  #endif

  #ifdef HAS_SCREEN
    //display_obj.tft.println(F(text_table0[6]));
  #endif

  #ifdef HAS_BATTERY
    battery_obj.battery_level = battery_obj.getBatteryLevel();
  #endif

  // Do some LED stuff
  #ifdef HAS_FLIPPER_LED
    flipper_led.RunSetup();
  #elif defined(XIAO_ESP32_S3)
    xiao_led.RunSetup();
  #elif defined(MARAUDER_M5STICKC) || defined(MARAUDER_TTGO_TDISPLAY)
    stickc_led.RunSetup();
  #else
    led_obj.RunSetup();
  #endif

  #ifdef HAS_SCREEN
    //display_obj.tft.println(F(text_table0[7]));

    //delay(500);
  #endif

  #ifdef HAS_GPS
    gps_obj.begin();
    //#ifdef HAS_SCREEN
      //if (gps_obj.getGpsModuleStatus())
        //display_obj.tft.println("GPS Module connected");
      //else
        //display_obj.tft.println("GPS Module NOT connected");
    //#endif
  #endif

  #ifdef HAS_SCREEN
    //display_obj.tft.println(F(text_table0[8]));
  
    display_obj.tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
    //delay(2000);
  #endif

  #ifdef HAS_SCREEN
    menu_function_obj.RunSetup();
  #endif

  wifi_scan_obj.StartScan(WIFI_SCAN_OFF);
  
  Serial.println(F("CLI Ready"));
  cli_obj.RunSetup();
}


void loop()
{
    // Deep sleep section
  static unsigned long buttonPressStart = 0; 
  static bool buttonHeld = false;

  // Deep sleep for MARAUDER_TTGO_TDISPLAY. Check if button 0 (D_BTN) is held for 5 seconds
  #if defined(MARAUDER_TTGO_TDISPLAY)
    if (digitalRead(D_BTN) == LOW) {
      if (!buttonHeld) {
        buttonHeld = true;
        buttonPressStart = millis();
      } else if (millis() - buttonPressStart >= 5000) {  // Time of button hold to activate deep sleep
        Serial.println("Entering deep sleep...");
        esp_deep_sleep_start();  // Trigger deep sleep
      }
    } else {
      buttonHeld = false;
    }
  #endif
  // End deep sleep section

  currentTime = millis();
  bool mini = false;

  #ifdef SCREEN_BUFFER
    #ifndef HAS_ILI9341
      mini = true;
    #endif
  #endif

  #if (defined(HAS_ILI9341) && !defined(MARAUDER_CYD_2USB))
    #ifdef HAS_BUTTONS
      if (c_btn.isHeld()) {
        if (menu_function_obj.disable_touch)
          menu_function_obj.disable_touch = false;
        else
          menu_function_obj.disable_touch = true;

        menu_function_obj.updateStatusBar();

        while (!c_btn.justReleased())
          delay(1);
      }
    #endif
  #endif

  // Update all of our objects
  cli_obj.main(currentTime);
  #ifdef HAS_SCREEN
    display_obj.main(wifi_scan_obj.currentScanMode);
  #endif
  wifi_scan_obj.main(currentTime);

  #ifdef HAS_GPS
    gps_obj.main();
  #endif
  
  // Detect SD card
  #if defined(HAS_SD)
    sd_obj.main();
  #endif

  // Save buffer to SD and/or serial
  buffer_obj.save();

  #ifdef HAS_BATTERY
    battery_obj.main(currentTime);
  #endif
  settings_obj.main(currentTime);
  if (((wifi_scan_obj.currentScanMode != WIFI_PACKET_MONITOR) && (wifi_scan_obj.currentScanMode != WIFI_SCAN_EAPOL)) ||
      (mini)) {
    #ifdef HAS_SCREEN
      menu_function_obj.main(currentTime);
    #endif
  }
  #ifdef HAS_FLIPPER_LED
    flipper_led.main();
  #elif defined(XIAO_ESP32_S3)
    xiao_led.main();
  #elif defined(MARAUDER_M5STICKC) || defined(MARAUDER_TTGO_TDISPLAY)
    stickc_led.main();
  #else
    led_obj.main(currentTime);
  #endif

  #ifdef HAS_SCREEN
    delay(1);
  #else
    delay(50);
  #endif
}
