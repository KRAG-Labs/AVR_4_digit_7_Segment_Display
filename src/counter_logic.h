#ifndef COUNTER_LOGIC_H
#define COUNTER_LOGIC_H

#include <stdint.h>

#define MAX_DIGITS 4

/**
 * Updates the digits array based on a 4-digit number (0-9999).
 * digits[0] is the units digit, digits[3] is the thousands.
 */
void update_digits(uint16_t count, uint8_t *digits);

/**
 * Increments the count and returns the updated count.
 * Wraps around at 9999 to 0000.
 */
uint16_t increment_count(uint16_t count);

#endif // COUNTER_LOGIC_H
