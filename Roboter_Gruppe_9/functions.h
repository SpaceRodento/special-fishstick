/*=====================================================================
  functions.h - LCD and helper functions
=======================================================================*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "config.h"
#include "structs.h"

// =============== LCD INSTANCE ================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =============== INLINE FUNCTIONS ================================

inline void initLCD() {
  lcd.init();
  lcd.clear();    
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ZignalMeister");
  lcd.setCursor(2, 1);
  lcd.print("2000");
  delay(3000);
  lcd.clear();
}

#endif // FUNCTIONS_H