/*
 * main.c
 * 4-digit 7-segment counter
 * Kenura R. Gunarathna
 * frame rate: 60Hz
 */

#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>
#include "counter_logic.h"

static const uint8_t RefreshTime = 16; // ms (60Hz total)
static const uint8_t DelayPerDigit = RefreshTime / MAX_DIGITS; // ~4ms per digit
static const uint16_t UpdatesPerSecond = 1000 / RefreshTime; // ~62 refreshes per second

// Segment patterns for 0-F
// Bit order: a=bit7, b=bit6, c=bit5, d=bit4, e=bit3, f=bit2, g=bit1, dp=bit0
// dp=0 (dot OFF) for all entries — OR with 0x01 to turn dot ON
static const uint8_t seg7[] PROGMEM = {
    0xFC, // 0  11111100
    0x60, // 1  01100000
    0xDA, // 2  11011010
    0xF2, // 3  11110010
    0x66, // 4  01100110
    0xB6, // 5  10110110
    0xBE, // 6  10111110
    0xE0, // 7  11100000
    0xFE, // 8  11111110
    0xF6, // 9  11110110
    0xEE, // A  11101110
    0x3E, // b  00111110
    0x9C, // C  10011100
    0x7A, // d  01111010
    0x9E, // E  10011110
    0x8E  // F  10001110
};

/*
 * Returns the segment pattern for a given digit.
 *
 * @param digit The digit value (0-15).
 * @return The segment pattern for the digit.
 */
static uint8_t get_segments(uint8_t digit) {
    if (digit > 15) return 0x00;
    return pgm_read_byte(&seg7[digit]);
}

int main(void) {
    DDRD = 0xFF; // Sets all pins of PORTD as outputs (segments)
    DDRC = 0x0F; // Sets PC0-PC3 as outputs (digit selectors)

    uint16_t main_counter = 0;
    uint8_t digit_values[MAX_DIGITS] = {0, 0, 0, 0};
    uint8_t digit_index = 0;
    uint16_t refresh_count = 0;

    while(1) {
        // Display current digit
        PORTD = get_segments(digit_values[digit_index]);
        PORTC = (1 << digit_index); // Select only the active digit

        _delay_ms(DelayPerDigit);

        // Move to next digit for multiplexing
        digit_index++;
        if (digit_index >= MAX_DIGITS) {
            digit_index = 0;

            // Check if one full refresh cycle (all digits) has completed
            // Rendered all the digits of a number.
            // If so, increment the main counter and update digit values
            refresh_count++;
            if (refresh_count >= UpdatesPerSecond) {
                refresh_count = 0;
                main_counter = increment_count(main_counter);
                update_digits(main_counter, digit_values);
            }
        }
    }

    return 0;
}
