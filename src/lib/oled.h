```cpp
//OLED
#include <U8g2lib.h>
int    oledRun = 0;

void oledTest(String pinSCL, String pinSDA) {
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ pinSCL.toInt(), /* data=*/ pinSDA.toInt());

  u8g2.begin();

  u8g2.clearBuffer();  // Clear buffer

  u8g2.setFont(u8g2_font_6x10_tf);  // Set font and size

  // Draw first line text
  u8g2.setCursor(0, 10);
  u8g2.print("First line text");

  // Draw second line text
  u8g2.setCursor(0, 20);
  u8g2.print("Second line text");

  u8g2.sendBuffer();  // Send buffer content to display

  //delay(1000);  // Delay 1 second


} 

void getOLED(){
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 14, /* data=*/ 12);

  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.clearBuffer();
  u8g2.setFontPosTop();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
  
  u8g2.drawUTF8(0, 1,  "Sow a grain of millet in spring, harvest ten thousand seeds in autumn");
  u8g2.drawUTF8(0, 13, "Through cold and heat, live up to the glorious years.");
  u8g2.drawUTF8(0, 25, "");
  u8g2.drawUTF8(0, 49, "    ZhongJiYun Wave 10    ");

  u8g2.sendBuffer();  // Send buffer content to display
}
```
