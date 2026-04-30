#include "counter_logic.h"

/*
 * Updates the digit values for a given count.
 * Each digit is stored in the `digits` array from right to left.
 *
 * @param count The current count value.
 * @param digits Pointer to the array where digit values will be stored.
 */
void update_digits(uint16_t count, uint8_t *digits) {
    for (int i = 0; i < MAX_DIGITS; i++) {
        digits[i] = count % 10; // Divide by 10 and store the remainder (rightmost digit).
        count /= 10;
    }
}

/*
 * Increments the count and wraps around to 0 after 9999.
 *
 * @param count The current count value.
 * @return The incremented count.
 */
uint16_t increment_count(uint16_t count) {
    count++;
    if (count > 9999) {
        count = 0;
    }
    return count;
}
