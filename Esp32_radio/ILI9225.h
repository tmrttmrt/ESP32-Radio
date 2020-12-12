// ILI9225.h
//
// Contributed by Uksa007@gmail.com
// Separated from the main sketch to allow several display types.
// Includes for various ILI9225 displays.  Tested on 320 x 240.
// Requires Adafruit ILI9225 library, available from library manager.
// Below set your dsp_getwidth() and dsp_getwidth() to suite your display.

#include <TFT_22_ILI9225.h>
#include <../fonts/FreeSans18pt7b.h>

// Color definitions for the TFT screen (if used)
// TFT has bits 6 bits (0..5) for RED, 6 bits (6..11) for GREEN and 4 bits (12..15) for BLUE.
#define BLACK   COLOR_BLACK
#define BLUE    COLOR_BLUE
#define RED     COLOR_RED
#define GREEN   COLOR_GREEN
#define CYAN    GREEN | BLUE
#define MAGENTA RED | BLUE
#define YELLOW  RED | GREEN
#define WHITE   RED | BLUE | GREEN

// Data to display.  There are TFTSECS sections
#define TFTSECS 4
scrseg_struct     tftdata[TFTSECS] =                        // Screen divided in 3 segments + 1 overlay
{                                                           // One text line is 11 pixels
    { false, WHITE,   0, 12, "" },                            // 1 top line
    { false, CYAN,   22, 92, "" },                            // 8 lines in the middle
    { false, YELLOW, 120, 53, "" },                            // 4 lines at the bottom
    { false, GREEN,  120, 53, "" }                             // 4 lines at the bottom for rotary encoder
} ;

void utf8ascii_ip ( char* s );

TFT_22_ILI9225*     tft = NULL ;                                  // For instance of display driver

int cursor_x=0;
int cursor_y=0;
uint16_t ccol;
int textsize;

// Various macro's to mimic the ILI9225 version of display functions
#define dsp_setRotation()       tft->setOrientation ( 1 )             // Use landscape format (3 for upside down)
#define dsp_fillRect(a,b,c,d,e) tft->fillRectangle ( a, b, a+c, b+d, e ) ;  // Fill a rectange
#define dsp_erase()             tft->clear() ;        // Clear the screen
#define dsp_getwidth()          220                                // Adjust to your display
#define dsp_getheight()         176                                // Get height of screen
#define dsp_update()                                               // Updates to the physical screen
#define dsp_usesSPI()           true                               // Does use SPI

void dsp_setCursor(int a,int b){
    cursor_x=a;
    cursor_y=b;
}

void dsp_setTextColor(uint16_t a){
    ccol=a;
}

void dsp_setTextSize(int a){
    textsize=a;
}

size_t write(char c){
    if(c == '\n') {
        cursor_x  = 0;
        cursor_y += (int16_t)textsize * tft->getFont().height;
    } else if(c != '\r') {
        int x = cursor_x + tft->getCharWidth(c)+1;
        if(x > dsp_getwidth()){
            cursor_x  = 0;
            cursor_y += (int16_t)textsize * tft->getFont().height;
        }
        if(cursor_y<dsp_getheight()-tft->getFont().height)
          tft->drawChar(cursor_x,cursor_y,c,ccol);
        cursor_x  += tft->getCharWidth(c)+1;
    }
}

void dsp_print(const char *a){
    for(char c=*a++ ; c != 0 ; c=*a++){
        write(c);
    }
}

void dsp_println(const char* b){
    dsp_print(b);
    write('\n');
}

bool dsp_begin()
{
    tft = new TFT_22_ILI9225 ( -1,
    ini_block.tft_dc_pin,
    ini_block.tft_cs_pin,
    ini_block.tft_bl_pin ) ;            // Create an instant for TFT

    tft->begin();                                                    // Init TFT interface
    tft->setFont(Terminal11x16);
    tft->setGFXFont(&FreeSans18pt7b);
    return ( tft != NULL ) ;
}

//**************************************************************************************************
//                                      D I S P L A Y I N F O                                      *
//**************************************************************************************************
// Show a string on the LCD at a specified y-position (0..2) in a specified color.                 *
// The parameter is the index in tftdata[].                                                        *
//**************************************************************************************************
void displayinfo ( uint16_t inx )
{
    uint16_t       width = dsp_getwidth() ;                  // Normal number of colums
    scrseg_struct* p = &tftdata[inx] ;
    uint16_t len ;                                           // Length of string, later buffer length

    switch(inx){
        case 2:
            dsp_fillRect ( 0, p->y, width, p->height, BLACK ) ;
            dsp_fillRect ( 0, p->y - 4, width, 1, GREEN );
            len = p->str.length() ;
            if ( len++ )                                           // Check string length, set buffer length
                {
                    int16_t w, h;
                    char buf [ len ] ;                                   // Need some buffer space
                    p->str.toCharArray ( buf, len ) ;                    // Make a local copy of the string
                    utf8ascii_ip ( buf ) ;                                  // Convert possible UTF8
                    tft->getGFXTextExtent(buf, 0, p->y, &w, &h);
                    if(w <= width){
                        tft->drawGFXText(0, p->y+h, buf, p->color);
                    } else {
                        dsp_setTextColor ( p->color ) ;                      // Set the requested color
                        dsp_setCursor ( 0, p->y ) ;                          // Prepare to show the info
                        dsp_println ( buf ) ;                                // Show the string
                    }
                }
            break;
        default:
            if ( inx == 0 )                                          // Topline is shorter
            {
                width -= 8 * (tft->getCharWidth('0')+1) ;                                     // Leave space for time
            }
            if ( tft )                                               // TFT active?
            {
                dsp_fillRect ( 0, p->y, width, p->height, BLACK ) ;    // Clear the space for new info
                if ( ( dsp_getheight() > 64 ) && ( p->y > 1 ) )        // Need and space for divider?
                {
                    dsp_fillRect ( 0, p->y - 4, width, 1, GREEN ) ;      // Yes, show divider above text
                }
                len = p->str.length() ;                                // Required length of buffer
                if ( len++ )                                           // Check string length, set buffer length
                {
                    char buf [ len ] ;                                   // Need some buffer space
                    p->str.toCharArray ( buf, len ) ;                    // Make a local copy of the string
                    utf8ascii_ip ( buf ) ;                                  // Convert possible UTF8
                    dsp_setTextColor ( p->color ) ;                      // Set the requested color
                    dsp_setCursor ( 0, p->y ) ;                          // Prepare to show the info
                    dsp_println ( buf ) ;                                // Show the string
                }
            }
            break;
    }
}


//**************************************************************************************************
//                                      D I S P L A Y B A T T E R Y                                *
//**************************************************************************************************
// Show the current battery charge level on the screen.                                            *
// It will overwrite the top divider.                                                              *
// No action if bat0/bat100 not defined in the preferences.                                        *
//**************************************************************************************************
void displaybattery()
{
    if ( tft )
    {
        //      dbgprint ("displaybattery()" ) ;
        if ( ini_block.bat0 < ini_block.bat100 )              // Levels set in preferences?
        {
            static uint16_t oldpos = 0 ;                        // Previous charge level
            uint16_t        ypos ;                              // Position on screen
            uint16_t        v ;                                 // Constrainted ADC value
            uint16_t        newpos ;                            // Current setting

            v = constrain ( adcval, ini_block.bat0,             // Prevent out of scale
            ini_block.bat100 ) ;
            newpos = map ( v, ini_block.bat0,                   // Compute length of green bar
            ini_block.bat100,
            0, dsp_getwidth() ) ;
            if ( newpos != oldpos )                             // Value changed?
            {
                oldpos = newpos ;                                 // Remember for next compare
                ypos = tftdata[1].y - 5 ;                         // Just before 1st divider
                dsp_fillRect ( 0, ypos, newpos, 2, GREEN ) ;      // Paint green part
                dsp_fillRect ( newpos, ypos,
                dsp_getwidth() - newpos,
                2, RED ) ;                          // Paint red part
            }
        }
    }
}


//**************************************************************************************************
//                                      D I S P L A Y V O L U M E                                  *
//**************************************************************************************************
// Show the current volume as an indicator on the screen.                                          *
// The indicator is 2 pixels heigh.                                                                *
//**************************************************************************************************
void displayvolume()
{
    if ( tft )
    {
        //            dbgprint ("displayvolume()" ) ;

        static uint8_t oldvol = 0 ;                         // Previous volume
        uint8_t        newvol ;                             // Current setting
        uint16_t       pos ;                                // Positon of volume indicator

        newvol = vs1053player->getVolume() ;                // Get current volume setting
        if ( newvol != oldvol )                             // Volume changed?
        {
            oldvol = newvol ;                                 // Remember for next compare
            pos = map ( newvol, 0, 100, 0, dsp_getwidth() ) ; // Compute position on TFT
            dsp_fillRect ( 0, dsp_getheight() - 2,
            pos, 2, RED ) ;                    // Paint red part
            dsp_fillRect ( pos, dsp_getheight() - 2,
            dsp_getwidth() - pos, 2, GREEN ) ; // Paint green part
        }
    }
}


//**************************************************************************************************
//                                      D I S P L A Y T I M E                                      *
//**************************************************************************************************
// Show the time on the LCD at a fixed position in a specified color                               *
// To prevent flickering, only the changed part of the timestring is displayed.                    *
// An empty string will force a refresh on next call.                                              *
// A character on the screen is 8 pixels high and 6 pixels wide.                                   *
//**************************************************************************************************
void displaytime ( const char* str, uint16_t color )
{
    //  dbgprint ("displaytime ( const char* str, uint16_t color )" ) ;
    static char oldstr[9] = "........" ;             // For compare
    uint8_t     i ;                                  // Index in strings
    uint8_t w = tft->getCharWidth('0')+1;
    uint16_t    pos = dsp_getwidth() - 8 * w ;     // X-position of character

    if ( str[0] == '\0' )                            // Empty string?
    {
        for ( i = 0 ; i < 8 ; i++ )                    // Set oldstr to dots
        {
            oldstr[i] = '.' ;
        }
        return ;                                       // No actual display yet
    }
    if ( tft )                                       // TFT active?
    {
        dsp_setTextColor ( color ) ;                   // Set the requested color
        for ( i = 0 ; i < 8 ; i++ )                    // Compare old and new
        {
            if ( str[i] != oldstr[i] )                   // Difference?
            {
                dsp_fillRect ( pos, 0, w,  tft->getFont().height, BLACK ) ;     // Clear the space for new character
                dsp_setCursor ( pos, 0 ) ;                 // Prepare to show the info
                write ( str[i] ) ;                     // Show the character
                oldstr[i] = str[i] ;                       // Remember for next compare
            }
            pos += w;                                   // Next position
        }
    }
}


