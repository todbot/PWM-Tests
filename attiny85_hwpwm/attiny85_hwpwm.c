/*
 * attiny_hwpwm.c -- Demonstrate 3-channel hardware PWM on ATtiny85
 *
 * 2012, Tod E. Kurt, http://todbot.com/blog/
 *
 * ATiny85 is wired up like: 
 *
 *                      ATtiny85
 *                   -------u-------
 *            n/c --| 1 RST   VCC 8 |--- +5V
 *                  |               |
 *            n/c --| 2 PB3   PB2 7 |-- n/c
 *                  |               | 
 * (OCR0B) BLUE <---| 3 PB4   PB1 6 |---> RED (OCR1B)
 *                  |               |      
 *           Gnd ---| 4 GND   PB0 5 |---> GREEN (OCR0A)
 *                  -----------------           
 *                                    
 * Note that because PB0 & PB2 are SDA & SCL, this means hardware I2C cannot
 * work with this arrangement.
 * 
 * many hints gained from: 
 *   http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=848902
 *
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  // for sei() 
#include <util/delay.h>     // for _delay_ms() 

// this is available hardware pwm
#define PIN_RED	 PB4   // OCR1B
#define PIN_GRN	 PB0   // OCR0A
#define PIN_BLU	 PB1   // OCR0B

// some "functions" to help set the r,g,b values
#define setRed(x) ( OCR1B = (x) )
#define setGrn(x) ( OCR0A = 255 - (x) )
#define setBlu(x) ( OCR0B = 255 - (x) )
#define setRGB(x,y,z) { setRed(x); setGrn(y); setBlu(z); }


// for "millis()" function, a count of 0.256ms units
static volatile uint32_t tick;
//
uint32_t millis(void)
{
    return tick/4;
}
// overflows every 0.256msec -> "millis" = ~tick/4
ISR(SIG_OVERFLOW1,ISR_NOBLOCK)
{
    tick++;
}

//
int main(void)
{
    wdt_disable();  // just in case

    DDRB = _BV(PIN_RED) | _BV(PIN_GRN) | _BV(PIN_BLU); 

    // WGM00 & WGM01 = FastPWM
    // COM0x1 & COM0x0 = Set OC0x on match, clear OC0x at TOP
    TCCR0A = 
        _BV(WGM01)  | _BV(WGM00) |
        _BV(COM0A1) | _BV(COM0A0) | 
        _BV(COM0B1) | _BV(COM0B0);
    
    //Timer0 prescaler /8  
    TCCR0B = _BV(CS01);

    // PWM1B = enable PWM on OCR1B 
    // COM1B1 = clear on compare match, set when 
    GTCCR = _BV(PWM1B) | 
        _BV(COM1B1); 

    //Timer1 prescaler /8  (4kHz @8MHz clock: 8e6/8/256 = 3906 Hz,aka 0.256ms)
    TCCR1 = _BV(CS12);

    // create a "millis" by using Timer1 overflow interrupt
    TIFR  |= _BV( TOV1 );
    TIMSK |= _BV( TOIE1 );
    sei();

    int v = 0;
    setRGB(v,v,v); // start black

    uint32_t lastMillis = millis();

    // a little demo of just on/off
    int flashcnt = 20;
    while( flashcnt ) {
        if( (millis() - lastMillis) >= 250 ) {
            lastMillis = millis();
            v = (v) ? 0 : flashcnt*10;  // toggle hi/lo, but dimming high
            setRGB(v,v,v);
            flashcnt--;
        }
    }

    // a little demo to ramp up and down the brightness of all three
    int incdec = 1;
    while( 1 ) { 
        //_delay_ms(10);
        if( (millis() - lastMillis) < 10 )  {
            continue;
        }
        lastMillis = millis();
        v += incdec;
        if( v == 255 ) incdec = -1;
        if( v ==   0 ) incdec = 1;
        
        setRGB( v,v,v );
    }
    return 0;
}

