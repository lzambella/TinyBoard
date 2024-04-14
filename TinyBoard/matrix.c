#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "matrix.h"
#include "debug.h"
#include "config.h"

// Variable to store the current row
static uint8_t debouncing = DEBOUNCE;

static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];

// Function definitions
static matrix_row_t read_cols(void);
static void unselect_rows(void);
static void select_row(uint8_t row);

void matrix_init(void) {
    MCUCR |= (1<<JTD);
    MCUCR |= (1<<JTD);

    // Set row pins to output
    // PORF 0,1,4,5 >> ROW 1,2,3,4
    DDRF |= (1 << 0);
    DDRF |= (1 << 1);
    DDRF |= (1 << 4);
    DDRF |= (1 << 5);

    // Initialize all rows to HIGH
    PORTF |= (1 << PF0);
    PORTF |= (1 << PF1);
    PORTF |= (1 << PF4);
    PORTF |= (1 << PF5);
    
    // Set column pins as input
    DDRF &= ~(1 << 6); // col 1

    DDRB &= ~(1 << 6); // 2
    DDRB &= ~(1 << 5); // 3
    DDRB &= ~(1 << 4); // 4
    DDRB &= ~(1 << 3); // 5
    DDRB &= ~(1 << 2); // 6
    DDRB &= ~(1 << 1); // 7
    DDRB &= ~(1 << 0); // 8

    DDRC &= ~(1 << 6); // 9
    DDRC &= ~(1 << 7); // 10

    DDRD &= ~(1 << 0); // 11
    DDRD &= ~(1 << 1); // 12

    // Disable internal pull ups on col
    PORTF &= ~(1 << 6); // col 1

    PORTB &= ~(1 << 6); // 2
    PORTB &= ~(1 << 5); // 3
    PORTB &= ~(1 << 4); // 4
    PORTB &= ~(1 << 3); // 5
    PORTB &= ~(1 << 2); // 6
    PORTB &= ~(1 << 1); // 7
    PORTB &= ~(1 << 0); // 8

    PORTC &= ~(1 << 6); // 9
    PORTC &= ~(1 << 7); // 10

    PORTD &= ~(1 << 0); // 11
    PORTD &= ~(1 << 1); // 12

    // make sure matrix is cleared on init
    for (int i = 0; i < MATRIX_ROWS; i++) { 
        matrix[i] = 0;
        matrix_debouncing[i] = 0;
    }
}

uint8_t matrix_scan(void) {
    debug("Scan row\n");
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        select_row(i);
        _delay_us(100);  // without this wait read unstable value.
        matrix_row_t cols = read_cols();
        if (matrix_debouncing[i] != cols) {
            matrix_debouncing[i] = cols;
            if (debouncing) {
                debug("bounce!: "); debug_hex(debouncing); debug("\n");
            }
            debouncing = DEBOUNCE;
        }
        unselect_rows();
    }

    if (debouncing) {
        if (--debouncing) {
            _delay_ms(1);
        } else {
            for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
                matrix[i] = matrix_debouncing[i];
            }
        }
    }

    return 1;
}

/*
* Deselects rows by writing a HIGH to all the row pins
*/
static void unselect_rows(void)
{
    PORTF |= (1 << PF0);
    PORTF |= (1 << PF1);
    PORTF |= (1 << PF4);
    PORTF |= (1 << PF5);
}

/*
* Get a single row in the matrix array
* Each row contains bits that represent the columns in the matrix
*/
matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

/*
* Reads all of the columns into an integer
*/
static matrix_row_t read_cols()
{
    // Get the state of the columns

    // If we read a LOW at the pin input (switch pressed) we want the variable to be HIGH
    uint16_t c1 =  ((~PINF & (1 << 6)) >> 6); // F6

    uint16_t c2 =  ((~PINB & (1 << 6)) >> 6); // B6
    uint16_t c3 =  ((~PINB & (1 << 5)) >> 5); // B5
    uint16_t c4 =  ((~PINB & (1 << 4)) >> 4); // B4
    uint16_t c5 =  ((~PINB & (1 << 3)) >> 3); // B3
    uint16_t c6 =  ((~PINB & (1 << 2)) >> 2); // B2
    uint16_t c7 =  ((~PINB & (1 << 1)) >> 1); // B1
    uint16_t c8 =  ((~PINB & (1 << 0)) >> 0); // B0

    uint16_t c9 =  ((~PINC & (1 << 6)) >> 6); // C6
    uint16_t c10 = ((~PINC & (1 << 7)) >> 7); // C7

    uint16_t c11 = ((~PIND & (1 << 0)) >> 0); // D0
    uint16_t c12 = ((~PIND & (1 << 1)) >> 1); // D1

    // Make a continuous integer of the inputs
    matrix_row_t C = 0;

    C = C | c1  << 0;
    C = C | c2  << 1;
    C = C | c3  << 2;
    C = C | c4  << 3;
    C = C | c5  << 4;
    C = C | c6  << 5;
    C = C | c7  << 6;
    C = C | c8  << 7;
    C = C | c9  << 8;
    C = C | c10 << 9;
    C = C | c11 << 10;
    C = C | c12 << 11;

    debug_bin16(C); debug("\n");
    return C;
}

/**
 * Select a row by writing LOW to it
*/
static void select_row(uint8_t row)
{
    switch (row) {
        // Row 1 (pin F0)
        case 0:
            DDRF |= (1 << DDF0);
            PORTF &= ~(1 << PF0); 
            break;
        case 1:
        // Row 2 (pin F1)
            DDRF |= (1 << DDF1);
            PORTF &= ~(1 << PF1); 
            break;
        case 2:
        // Row 3 (Pin F4)
            DDRF |= (1 << DDF4);
            PORTF &= ~(1 << PF4);
            break;
        case 3:
        // Row 4 (Pin F5)
            DDRF |= (1 << DDF5);
            PORTF &= ~(1 << PF5); 
            break;
    }
}