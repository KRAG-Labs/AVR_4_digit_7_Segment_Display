#include <stdio.h>
#include <assert.h>
#include "../src/counter_logic.h"

void test_increment_count() {
    printf("Testing increment_count...\n");
    assert(increment_count(0) == 1);
    assert(increment_count(9998) == 9999);
    assert(increment_count(9999) == 0); // Wrap around
    printf("increment_count passed!\n");
}

void test_update_digits() {
    printf("Testing update_digits...\n");
    uint8_t digits[4] = {0};
    
    update_digits(1234, digits);
    assert(digits[0] == 4); // Units
    assert(digits[1] == 3); // Tens
    assert(digits[2] == 2); // Hundreds
    assert(digits[3] == 1); // Thousands

    update_digits(5, digits);
    assert(digits[0] == 5);
    assert(digits[1] == 0);
    assert(digits[2] == 0);
    assert(digits[3] == 0);

    update_digits(9999, digits);
    assert(digits[0] == 9);
    assert(digits[1] == 9);
    assert(digits[2] == 9);
    assert(digits[3] == 9);
    
    printf("update_digits passed!\n");
}

int main() {
    test_increment_count();
    test_update_digits();
    printf("\nAll logic tests passed successfully!\n");
    return 0;
}
