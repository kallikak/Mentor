#include "Config.h"
#include "LCD.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <Wire.h>
#include "Print.h"


/*!
 *  @brief Device I2C Arress
 */
#define LCD_ADDRESS     (0x7c>>1)
#define RGB_ADDRESS     (0xc0>>1)

/*!
 *  @brief color define
 */ 
#define WHITE           0
#define RED             1
#define GREEN           2
#define BLUE            3
#define ONLY            3

#define REG_RED         0x04        // pwm2
#define REG_GREEN       0x03        // pwm1
#define REG_BLUE        0x02        // pwm0
#define REG_ONLY        0x02

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

/*!
 *  @brief commands
 */
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

/*!
 *  @brief flags for display entry mode
 */
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/*!
 *  @brief flags for display on/off control
 */
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

/*!
 *  @brief flags for display/cursor shift
 */
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

/*!
 *  @brief flags for function set
 */
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class DFRobot_LCD : public Print 
{
public:

  /*!
   *  @brief Constructor
   */
  DFRobot_LCD(uint8_t lcd_cols,uint8_t lcd_rows,uint8_t lcd_Addr=LCD_ADDRESS,uint8_t RGB_Addr=RGB_ADDRESS);
  
  /*!
   *  @brief initialize
   */ 
  void init();
  
  void clear();
  void home();

  /*!
   *  @brief Turn the display on/off (quickly)
   */
  void noDisplay();
  void display();

  /*!
   *  @brief Turn on and off the blinking showCursor
   */
  void stopBlink();
  void blink();

  /*!
   *  @brief Turns the underline showCursor on/off
   */
  void noCursor();
  void cursor();

  /*!
   *  @brief These commands scroll the display without changing the RAM
   */
  void scrollDisplayLeft();
  void scrollDisplayRight();
 
  /*!
   *  @brief This is for text that flows Left to Right
   */
  void leftToRight();
 
  /*!
   *  @brief This is for text that flows Right to Left
   */
  void rightToLeft();

  /*!
   *  @brief This will 'left justify' text from the showCursor
   */
  void noAutoscroll();
 
  /*!
   *  @brief This will 'right justify' text from the showCursor
   */
  void autoscroll();
   
  /*!
   *  @brief Allows us to fill the first 8 CGRAM locations
   *     with custom characters
   */
  void customSymbol(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t);  
  
  /*!
   *  @brief color control
   */
  void setRGB(uint8_t r, uint8_t g, uint8_t b);               // set rgb
  void setPWM(uint8_t color, uint8_t pwm){setReg(color, pwm);}      // set pwm 
  void setColor(uint8_t color);
  void setColorAll(){setRGB(0, 0, 0);}
  void setColorWhite(){setRGB(255, 255, 255);}

  /*!
   *  @brief blink the LED backlight
   */
  void blinkLED(void);
  void noBlinkLED(void);

  /*!
   *  @brief send data
   */
  virtual size_t write(uint8_t);

  /*!
   *  @brief send command
   */
  void command(uint8_t);
  
  /*!
   *  @brief compatibility API function aliases
   */
  void blink_on();            // alias for blink()
  void blink_off();                 // alias for noBlink()
  void cursor_on();                 // alias for cursor()
  void cursor_off();                // alias for noCursor()
  void setBacklight(uint8_t new_val);       // alias for backlight() and nobacklight()
  void load_custom_character(uint8_t char_num, uint8_t *rows);  // alias for createChar()
  void printstr(const char[]);
  
  /*!
   *  @brief Unsupported API functions (not implemented in this library)
   */
  uint8_t status();
  void setContrast(uint8_t new_val);
  uint8_t keypad();
  void setDelay(int,int);
  void on();
  void off();
  uint8_t init_bargraph(uint8_t graphtype);
  void draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end);
  void draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end);
  
  using Print::write;
  
private:
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
  void send(uint8_t *data, uint8_t len);
  void setReg(uint8_t addr, uint8_t data);
  uint8_t _showfunction;
  uint8_t _showcontrol;
  uint8_t _showmode;
  uint8_t _initialized;
  uint8_t _numlines,_currline;
  uint8_t _lcdAddr;
  uint8_t _RGBAddr;
  uint8_t _cols;
  uint8_t _rows;
  uint8_t _backlightval;
};

const uint8_t color_define[4][3] = 
{
    {255, 255, 255},            // white
    {255, 0, 0},                // red
    {0, 255, 0},                // green
    {0, 0, 255},                // blue
};
/*******************************public*******************************/
DFRobot_LCD::DFRobot_LCD(uint8_t lcd_cols,uint8_t lcd_rows,uint8_t lcd_Addr,uint8_t RGB_Addr)
{
  _lcdAddr = lcd_Addr;
  _RGBAddr = RGB_Addr;
  _cols = lcd_cols;
  _rows = lcd_rows;
}

void DFRobot_LCD::init()
{
  Wire.begin();
  TWBR = 12;
  _showfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  begin(_cols, _rows);
}

void DFRobot_LCD::clear()
{
    command(LCD_CLEARDISPLAY);        // clear display, set cursor position to zero
    delayMicroseconds(2000);          // this command takes a long time!
}

void DFRobot_LCD::home()
{
    command(LCD_RETURNHOME);        // set cursor position to zero
    delayMicroseconds(2000);        // this command takes a long time!
}

void DFRobot_LCD::noDisplay()
{
    _showcontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::display() {
    _showcontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::stopBlink()
{
    _showcontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}
void DFRobot_LCD::blink()
{
    _showcontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::noCursor()
{
    _showcontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::cursor() {
    _showcontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::scrollDisplayLeft(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void DFRobot_LCD::scrollDisplayRight(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void DFRobot_LCD::leftToRight(void)
{
    _showmode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _showmode);
}

void DFRobot_LCD::rightToLeft(void)
{
    _showmode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _showmode);
}

void DFRobot_LCD::noAutoscroll(void)
{
    _showmode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _showmode);
}

void DFRobot_LCD::autoscroll(void)
{
    _showmode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _showmode);
}

void DFRobot_LCD::customSymbol(uint8_t location, uint8_t charmap[])
{

    location &= 0x7; // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));
    
    
    uint8_t data[9];
    data[0] = 0x40;
    for(int i=0; i<8; i++)
    {
        data[i+1] = charmap[i];
    }
    send(data, 9);
}

void DFRobot_LCD::setCursor(uint8_t col, uint8_t row)
{

    col = (row == 0 ? col|0x80 : col|0xc0);
    uint8_t data[3] = {0x80, col};

    send(data, 2);

}

void DFRobot_LCD::setRGB(uint8_t r, uint8_t g, uint8_t b)
{
    setReg(REG_RED, r);
    setReg(REG_GREEN, g);
    setReg(REG_BLUE, b);
}


void DFRobot_LCD::setColor(uint8_t color)
{
    if(color > 3)return ;
    setRGB(color_define[color][0], color_define[color][1], color_define[color][2]);
}

void DFRobot_LCD::blinkLED(void)
{
    ///< blink period in seconds = (<reg 7> + 1) / 24
    ///< on/off ratio = <reg 6> / 256
    setReg(0x07, 0x17);  // blink every second
    setReg(0x06, 0x7f);  // half on, half off
}

void DFRobot_LCD::noBlinkLED(void)
{
    setReg(0x07, 0x00);
    setReg(0x06, 0xff);
}

inline size_t DFRobot_LCD::write(uint8_t value)
{

    uint8_t data[3] = {0x40, value};
    send(data, 2);
    return 1; // assume sucess
}

inline void DFRobot_LCD::command(uint8_t value)
{
    uint8_t data[3] = {0x80, value};
    send(data, 2);
}

void DFRobot_LCD::blink_on(){
  blink();
}

void DFRobot_LCD::blink_off(){
  stopBlink();
}

void DFRobot_LCD::cursor_on(){
  cursor();
}

void DFRobot_LCD::cursor_off(){
  noCursor();
}

void DFRobot_LCD::setBacklight(uint8_t new_val){
  if(new_val){
    blinkLED();   // turn backlight on
  }else{
    noBlinkLED();   // turn backlight off
  }
}

void DFRobot_LCD::load_custom_character(uint8_t char_num, uint8_t *rows){
    customSymbol(char_num, rows);
}

void DFRobot_LCD::printstr(const char c[]){
  ///< This function is not identical to the function used for "real" I2C displays
  ///< it's here so the user sketch doesn't have to be changed 
  print(c);
}

/*******************************private*******************************/
void DFRobot_LCD::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) 
{
    if (lines > 1) {
        _showfunction |= LCD_2LINE;
    }
    _numlines = lines;
    _currline = 0;

    ///< for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _showfunction |= LCD_5x10DOTS;
    }

    ///< SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    ///< according to datasheet, we need at least 40ms after power rises above 2.7V
    ///< before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    delay(50);


    ///< this is according to the hitachi HD44780 datasheet
    ///< page 45 figure 23

    ///< Send function set command sequence
    command(LCD_FUNCTIONSET | _showfunction);
    delay(5);  // wait more than 4.1ms
  
  ///< second try
    command(LCD_FUNCTIONSET | _showfunction);
    delay(5);

    ///< third go
    command(LCD_FUNCTIONSET | _showfunction);




    ///< turn the display on with no cursor or blinking default
    _showcontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    ///< clear it off
    clear();

    ///< Initialize to default text direction (for romance languages)
    _showmode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    ///< set the entry mode
    command(LCD_ENTRYMODESET | _showmode);
    
    
    ///< backlight init
    setReg(REG_MODE1, 0);
    ///< set LEDs controllable by both PWM and GRPPWM registers
    setReg(REG_OUTPUT, 0xFF);
    ///< set MODE2 values
    ///< 0010 0000 -> 0x20  (DMBLNK to 1, ie blinky mode)
    setReg(REG_MODE2, 0x20);
    
    setColorWhite();

}

void DFRobot_LCD::send(uint8_t *data, uint8_t len)
{
    Wire.beginTransmission(_lcdAddr);        // transmit to device #4
    for(int i=0; i<len; i++) {
        Wire.write(data[i]);
//    delay(5);
    }
    Wire.endTransmission();                     // stop transmitting
}

void DFRobot_LCD::setReg(uint8_t addr, uint8_t data)
{
    Wire.beginTransmission(_RGBAddr); // transmit to device #4
    Wire.write(addr);
    Wire.write(data);
    Wire.endTransmission();    // stop transmitting
}

/************************unsupported API functions***************************/
void DFRobot_LCD::off(){}
void DFRobot_LCD::on(){}
void DFRobot_LCD::setDelay (int cmdDelay,int charDelay) {}
uint8_t DFRobot_LCD::status(){return 0;}
uint8_t DFRobot_LCD::keypad (){return 0;}
uint8_t DFRobot_LCD::init_bargraph(uint8_t graphtype){return 0;}
void DFRobot_LCD::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end){}
void DFRobot_LCD::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_row_end){}
void DFRobot_LCD::setContrast(uint8_t new_val){}

DFRobot_LCD lcd(16, 2);

void setupLCD()
{
  byte sine[] = {
    B00000,
    B00000,
    B01000,
    B10100,
    B00101,
    B00010,
    B00000,
    B00000
  };
  
  byte tri[] = {
    B00000,
    B00100,
    B01010,
    B01010,
    B10001,
    B10001,
    B00000,
    B00000
  };
  
  byte saw[] = {
    B00000,
    B00001,
    B00011,
    B00101,
    B01001,
    B10001,
    B00000,
    B00000
  };
  
  byte square[] = {
    B00000,
    B11101,
    B10101,
    B10101,
    B10101,
    B10111,
    B00000,
    B00000
  };

  Wire.setClock(1000000);
  
  lcd.init();
  
  lcd.customSymbol(1, tri);
  lcd.customSymbol(2, saw);
  lcd.customSymbol(3, square);
  
//  TWBR = 12;
  
  lcd.noAutoscroll();
  lcd.setCursor(0, 0);
  lcd.print("Starting MENTOR ");
  delay(200);
  lcd.setCursor(0, 1);
  lcd.print(" >>> Ready <<<  ");
}

void clearLCD()
{
  lcd.clear();
}

void clearRow(int r)
{
  lcd.setCursor(0, r);
  lcd.print("                "); 
}

Str16 str;
Str16 str2;

void writeRow(int r, const char *str)
{
  lcd.setCursor(0, r);
  sprintf(str2, "%-16s", str);
  lcd.print(str2); 
}

void writeParamValue(int r, const char *param, ccInt cc)
{
  lcd.setCursor(0, r);
  sprintf(str, "%s %d", param, cc);
  sprintf(str2, "%-16s", str);
  lcd.print(str2); 
}

void writeParamString(int r, const char *param, const char *s)
{
  lcd.setCursor(0, r);
  sprintf(str, "%s %s", param, s);
  sprintf(str2, "%-16s", str);
  lcd.print(str2); 
}
