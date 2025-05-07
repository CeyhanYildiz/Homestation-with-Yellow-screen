//includes
#include <SPI.h>
#include <TFT_eSPI.h>
#include <string>
#include <XPT2046_Touchscreen.h>

//defines
//touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS
//screen settings
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define GENERAL_FONT_SIZE 2
#define DATA_FONT_SIZE 4
//other defines
#define MAXSENSCOUNT 4
#define MAXBUFFERSIZE 128

//global
TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI = SPIClass( VSPI );
XPT2046_Touchscreen touchscreen( XPT2046_CS, XPT2046_IRQ );

//functions

void drawBackground()
{
  tft.fillScreen( TFT_DARKGREY );
  tft.drawLine( 0, 31, SCREEN_WIDTH, 31, TFT_BLACK );
  tft.drawLine( 0, 30, SCREEN_WIDTH, 30, TFT_LIGHTGREY );
}

void drawFrame( String name, float *value )
{
  //draw default frame
  drawBackground();
  tft.setTextColor( TFT_BLACK );
  tft.setTextSize( GENERAL_FONT_SIZE );
  tft.drawString( name, 31, 6 );
  tft.setTextColor( TFT_LIGHTGREY );
  tft.drawString( name, 30, 5 );

  //draw the rest of the frame based on value
  if ( value != nullptr )
  {
    tft.fillCircle( 300, 15, 6, TFT_GREEN );
    tft.setTextColor( TFT_BLACK );
    tft.setTextSize( DATA_FONT_SIZE );
    tft.drawFloat( *value, ( (uint8_t)(*value) % 10 ), 60, 100 );
    tft.setTextColor( TFT_LIGHTGREY );
    tft.drawFloat( *value, ( (uint8_t)(*value) % 10 ), 59, 99 );
  }
  else
  {
    tft.fillCircle( 300, 15, 6, TFT_RED );
    tft.setTextColor( TFT_BLACK );
    tft.setTextSize( DATA_FONT_SIZE );
    tft.drawString( "no data", 60, 100 );
    tft.setTextColor( TFT_LIGHTGREY );
    tft.drawString( "no data", 59, 99 );
  }
}


//base function calls

void setup()
{
  //init serial communication
  Serial.begin(115200);

  //init screen
  touchscreenSPI.begin( XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS );
  touchscreen.begin( touchscreenSPI );
  touchscreen.setRotation( 1 );
  tft.init();
  tft.setRotation( 1 );
  tft.fillScreen( TFT_GREEN );
}

//variables
int sensCount = 0;
String names[MAXSENSCOUNT] = { "sens1", "sens2", "sens3", "sens4" };
float sensValue[MAXSENSCOUNT] = { 8.5f, 15.1f, -0.7f, 9.9f };

void loop()
{
  //handle touch
  if ( touchscreen.tirqTouched() && touchscreen.touched() ) 
  {
    //get position of the touch
    TS_Point newPoint = touchscreen.getPoint();
    int newX = map( newPoint.x, 200, 3700, 1, SCREEN_WIDTH );
    int newY = map( newPoint.y, 240, 3800, 1, SCREEN_HEIGHT );

    //check touch location and change sensCount accordingly
    if ( newY > SCREEN_HEIGHT / 2 )
    {
      --sensCount;
      sensCount = sensCount < 0 ? ( MAXSENSCOUNT - 1 ) : sensCount;
    }
    else
    {
      ++sensCount;
      sensCount = sensCount == MAXSENSCOUNT ? 0 : sensCount;
    }

    //wait for the touch to be released
    while ( touchscreen.tirqTouched() && touchscreen.touched() ) {}
    drawFrame( names[sensCount], &(sensValue[sensCount]) );
  }
  
  delay(2);
}

//end of file
