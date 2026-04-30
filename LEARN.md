# AVR Microcontrollers: A Deep Dive for C Programmers

Welcome to the world of embedded systems! Since you already know C, you have a massive advantage. This guide will bridge the gap between standard C programming and hardware-level programming using `avr-libc`, and then take you deep into the heart of the machine with AVR Assembly.

---

## Part 1: The AVR Architecture (The 10,000-foot view)

The ATmega328p (the brain of the Arduino Uno) is an **8-bit AVR RISC microcontroller**.

### Key Concepts
1. **Harvard Architecture:** Unlike your PC (von Neumann architecture), AVR separates program memory (Flash) and data memory (SRAM). Your code lives in Flash; your variables live in SRAM.
2. **8-bit Architecture:** The natural data size is 8 bits (1 byte). Doing math on a 16-bit or 32-bit integer takes multiple instructions. Keep things 8-bit (`uint8_t`) whenever possible for speed!
3. **Registers:** It has 32 general-purpose 8-bit registers (`R0` to `R31`). These are directly connected to the Arithmetic Logic Unit (ALU).
4. **Special Function Registers (SFRs):** These are memory-mapped registers used to control the hardware peripherals (Pins, Timers, ADC, UART).

---

## Part 2: C Programming with `avr-libc`

`avr-libc` is the standard library for AVR development. It provides the macros and definitions needed to talk to the hardware.

### 1. The Holy Trinity of Bitwise Operations
In desktop C, you rarely manipulate individual bits. In embedded C, **it's everything.** You need to master these three operations:

**Setting a bit (OR):**
```c
PORTC |= (1 << 5);  // Set bit 5 of PORTC to 1
```

**Clearing a bit (AND NOT):**
```c
PORTC &= ~(1 << 5); // Set bit 5 of PORTC to 0
```

**Toggling a bit (XOR):**
```c
PORTC ^= (1 << 5);  // Flip bit 5 of PORTC
```

*Note: `avr-libc` provides a macro `_BV(bit)` which means "Bit Value". `_BV(5)` is exactly the same as `(1 << 5)`. You will see `_BV()` everywhere in AVR code.*

### 2. GPIO (General Purpose Input/Output)
Every port (like Port B, Port C, Port D) has three SFRs associated with it:

*   **`DDRx` (Data Direction Register):** Determines if a pin is an Input (`0`) or Output (`1`).
    *   `DDRC |= (1 << PC5);` -> Make Pin C5 an output.
*   **`PORTx` (Data Register):** 
    *   If the pin is an **Output**, setting `PORTx` to `1` drives it HIGH (5V). Setting it to `0` drives it LOW (0V).
    *   If the pin is an **Input**, setting `PORTx` to `1` turns on the internal pull-up resistor.
*   **`PINx` (Input Pins Register):** You **read** from this register to see the physical voltage on the pin.
    *   `if (PINC & (1 << PC4)) { ... }` -> Check if Pin C4 is HIGH.

### 3. Delays
`avr-libc` provides highly optimized, cycle-accurate delay loops.
```c
#define F_CPU 1000000UL // MUST be defined before the include!
#include <util/delay.h>

_delay_ms(100); // Pauses execution for exactly 100 milliseconds
```

### 4. Interrupts
Instead of constantly checking if a button was pressed (polling), you can tell the hardware to interrupt your main program when an event happens.

```c
#include <avr/interrupt.h>

// ISR (Interrupt Service Routine) for Timer 1 Overflow
ISR(TIMER1_OVF_vect) {
    // This code runs automatically when Timer 1 overflows
    PORTC ^= (1 << PC5); // Toggle LED
}

int main(void) {
    // ... setup timer ...
    sei(); // "Set Enable Interrupts" - Turns on global interrupts
    while(1) { /* Main loop */ }
}
```

---

## Part 3: Deep Dive into AVR Assembly

C is great, but the compiler hides the raw machinery. Writing assembly gives you absolute control over timing and size.

### The Toolchain
*   `avr-gcc`: Compiles C to Assembly, then Assembly to Machine Code.
*   `avr-as`: The GNU Assembler. This is what processes your `.S` files.

### 1. The Assembly File Structure
An assembly file (`.S` extension allows C preprocessor macros like `#include`) looks like this:

```assembly
#define __SFR_OFFSET 0  ; Crucial! Makes C headers work in ASM
#include <avr/io.h>

.global main            ; Make 'main' visible to the linker

main:
    ; Your code here
```
*Why `__SFR_OFFSET 0`?* In C, registers are memory pointers (e.g., address `0x28`). In Assembly, instructions like `out` use an I/O space address (e.g., `0x08`). `avr-libc` defaults to C-style addresses. Defining the offset to 0 forces the headers to provide the correct I/O addresses for the `in` and `out` instructions.

### 2. Core Instructions

**Data Movement:**
*   `ldi r24, 0x10` : **L**oa**D** **I**mmediate. Puts the literal number `0x10` into register `r24`. *(Note: You can only use `ldi` on registers `r16` to `r31`!)*
*   `mov r16, r24`  : **MOV**e. Copies data from `r24` to `r16`.

**I/O Operations:**
*   `out PORTC, r24`: Writes the contents of `r24` into the `PORTC` register.
*   `in r24, PINC`  : Reads the current state of `PINC` into `r24`.

**Bit Manipulation (The absolute best instructions in AVR):**
*   `sbi DDRC, 5` : **S**et **B**it in **I**/O. Sets bit 5 of `DDRC` to `1`. (Changes one bit without touching the others, in just 2 clock cycles!).
*   `cbi PORTC, 5`: **C**lear **B**it in **I**/O. Sets bit 5 of `PORTC` to `0`.
*   *Note: `sbi` and `cbi` only work on the lower 32 I/O registers (addresses 0x00 to 0x1F).*

**Math and Logic:**
*   `inc r16` : Increment `r16` by 1.
*   `dec r16` : Decrement `r16` by 1.
*   `add r16, r17` : `r16 = r16 + r17`
*   `and r16, r17` : Bitwise AND.

**Control Flow (Branching):**
*   `rjmp label` : **R**elative **J**u**MP**. Go to `label`.
*   `rcall label`: **R**elative **CALL**. Call a subroutine (pushes return address to stack).
*   `ret`        : **RET**urn from subroutine.

**Conditional Branching (The "If" statements):**
Every time you do math (like `dec`), the CPU updates the **Status Register (SREG)**. It contains flags like the **Z (Zero) flag**.

*   `brne label` : **BR**anch if **N**ot **E**qual. (Jumps if the Z flag is 0, meaning the last math operation did NOT result in zero).
*   `breq label` : **BR**anch if **EQ**ual. (Jumps if the Z flag is 1, meaning the last math operation resulted in zero).

### 3. Deconstructing the Assembly Delay Loop
In our `main.S`, we wrote a delay loop. AVR doesn't have a "sleep" instruction, so we just make the CPU count down to zero to waste time.

```assembly
delay:
    ldi r18, 10      ; Outer loop counter
d1: ldi r19, 255     ; Middle loop counter
d2: ldi r20, 255     ; Inner loop counter

d3: dec r20          ; Subtract 1 from r20. (Takes 1 clock cycle)
    brne d3          ; If r20 != 0, jump back to d3. (Takes 2 cycles if it jumps, 1 if it doesn't)
    
    dec r19          ; When r20 hits 0, subtract 1 from r19
    brne d2          ; If r19 != 0, refill r20 and start again
    
    dec r18          ; When r19 hits 0, subtract 1 from r18
    brne d1          ; If r18 != 0, refill r19 and start again
    
    ret              ; Return when all loops finish
```
*Math:* `d3` takes 3 cycles per iteration. `255 * 3 = 765 cycles`. The middle loop runs `255` times: `765 * 255 = ~195,000 cycles`. The outer loop runs `10` times: `195,000 * 10 = ~1.95 million cycles`. At 1MHz (1,000,000 cycles per second), this wastes almost 2 seconds!

---

## Part 4: Mixing C and Assembly

You can call Assembly functions from C, and C functions from Assembly! You just have to follow the **AVR-GCC ABI (Application Binary Interface)**.

**The ABI Rules for Function Calls:**
1.  **Arguments:** When C calls your assembly function `void my_asm_func(uint8_t a, uint16_t b)`, it passes the arguments in registers.
    *   Argument 1 (`uint8_t a`) goes into `r24`.
    *   Argument 2 (`uint16_t b`) goes into `r22` (low byte) and `r23` (high byte).
2.  **Return Values:** If your assembly function returns a `uint8_t`, you MUST put the answer in `r24` before calling `ret`.
3.  **Call-Saved Registers:** If your assembly function uses registers `r2` to `r17`, `r28`, or `r29`, you **must** push them to the stack at the beginning and pop them at the end, because C expects them to remain unchanged. Registers `r18` to `r27`, `r30`, and `r31` are "call-clobbered"—you can use them freely.

---

## Part 5: Official Documentation and Further Reading

To truly master AVR, you need to learn how to read the datasheets. They are the ultimate source of truth.

1.  **The Bible:** [ATmega328P Datasheet (PDF)](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)
    *   *Where to look:* Look at the "Register Description" at the end of every chapter (e.g., I/O Ports, Timers). It tells you exactly what every single bit in every SFR does.
2.  **The Assembly Dictionary:** [AVR Instruction Set Manual (PDF)](https://ww1.microchip.com/downloads/en/devicedoc/atmel-0856-avr-instruction-set-manual.pdf)
    *   *Where to look:* Use this to look up exactly what `ldi`, `brne`, etc., do, how many clock cycles they take, and which SREG flags they modify.
3.  **The C Library Docs:** [avr-libc User Manual](https://www.nongnu.org/avr-libc/user-manual/)
    *   *Where to look:* Check `<avr/io.h>`, `<avr/interrupt.h>`, and the FAQ section.

### Next Steps / Homework for You
1.  **In C:** Try setting up Timer0 to trigger an interrupt every 1 second, and toggle the LED inside the ISR instead of using `_delay_ms()`.
2.  **Next level:** Write a C program with a `main()` loop that uses multiple timers and interrupts to control several LEDs independently.

---

# Part 6: The Complete AVR Instruction Set Reference

To master assembly, you must know your tools. Here is the categorized breakdown of the 131 instructions available in the AVR core.

### 1. Data Transfer Instructions (The "Movers")
These move bytes between registers, SRAM, and I/O space.

| Mnemonic | Description | Usage Example | Cycles |
| :--- | :--- | :--- | :--- |
| `MOV` | Move (Copy) Reg to Reg | `mov r16, r17` | 1 |
| `LDI` | Load Immediate (8-bit) | `ldi r16, 0xFF` (r16-r31 only) | 1 |
| `LD` | Load Indirect from SRAM | `ld r16, X` (X is r26:r27) | 2 |
| `ST` | Store Indirect to SRAM | `st X, r16` | 2 |
| `LDS` | Load Direct from SRAM | `lds r16, 0x0100` | 2 |
| `STS` | Store Direct to SRAM | `sts 0x0100, r16` | 2 |
| `LPM` | Load Program Memory | `lpm r0, Z` (Z is r30:r31) | 3 |
| `PUSH` | Push Register to Stack | `push r16` | 2 |
| `POP` | Pop Register from Stack | `pop r16` | 2 |

**The Indirect Magic (X, Y, Z Registers):**
AVR uses pairs of registers as 16-bit pointers to access the 2KB of SRAM.
*   **X:** `r27:r26`
*   **Y:** `r29:r28`
*   **Z:** `r31:r30` (Only Z can be used for `LPM` to read from Flash).

Example of a pointer loop in ASM:
```assembly
ldi XL, lo8(0x0100) ; Start of my data
ldi XH, hi8(0x0100)
ld r16, X+          ; Load value, then auto-increment pointer!
```

### 2. Arithmetic and Logic Instructions (The "Calculators")
| Mnemonic | Description | Usage Example |
| :--- | :--- | :--- |
| `ADD` / `ADC` | Add / Add with Carry | `add r16, r17` |
| `SUB` / `SBC` | Sub / Sub with Carry | `sub r16, r17` |
| `ADIW` / `SBIW` | Add/Sub Imm to Word | `adiw r24, 1` (Pairs r24:r25, etc) |
| `MUL` | Multiply (Unsigned) | `mul r16, r17` (Result in r1:r0) |
| `AND` / `OR` | Logic AND / OR | `and r16, r17` |
| `EOR` | Exclusive OR (XOR) | `eor r16, r16` (Quickly zero a reg) |
| `COM` / `NEG` | One's / Two's Complement | `neg r16` (Make it negative) |

### 3. Bit and Bit-Test Instructions (The "Surgical" Tools)
| Mnemonic | Description | Usage |
| :--- | :--- | :--- |
| `SBI` / `CBI` | Set/Clear Bit in I/O | `sbi PORTB, 0` |
| `LSL` / `LSR` | Logical Shift Left/Right | `lsl r16` (Multiplies by 2) |
| `ASR` | Arithmetic Shift Right | `asr r16` (Divides by 2, keeps sign) |
| `ROL` / `ROR` | Rotate Left/Right (thru Carry) | `rol r16` |
| `BST` / `BLD` | Bit Store / Bit Load | `bst r16, 0` (Store bit 0 in T-flag) |
| `SET` / `CLT` | Set/Clear T-flag | `set` |
| `SEI` / `CLI` | Set/Clear Global Interrupts | `sei` |

---

# Part 7: Hardware Peripheral Mastery (Assembly Edition)

This is where you stop writing "code" and start building "machines."

## 1. The UART (Serial Communication)
To send a character via USB-Serial in assembly, you must interact with the `UDR0` (Data Register) and `UCSR0A` (Status Register).

**Assembly UART Setup (9600 baud at 1MHz):**
```assembly
uart_init:
    ldi r16, 6              ; 9600 baud constant for 1MHz
    sts UBRR0L, r16         ; Set baud rate
    ldi r16, (1<<TXEN0)     ; Enable transmitter
    sts UCSR0B, r16
    ret

uart_send:
    lds r17, UCSR0A         ; Read status
    sbrs r17, UDRE0         ; Skip next if Data Reg Empty flag is set
    rjmp uart_send          ; Wait if not ready
    sts UDR0, r24           ; Send byte in r24
    ret
```

## 2. Timers and PWM (Pulse Width Modulation)
Timers are hardware counters that run independently of your code. `Timer 0` is 8-bit, `Timer 1` is 16-bit.

**Setting up a 16-bit Timer 1 Interrupt:**
```assembly
timer1_init:
    ldi r16, (1<<CS12) | (1<<CS10) ; Prescaler 1024
    sts TCCR1B, r16
    ldi r16, (1<<TOIE1)            ; Enable Overflow Interrupt
    sts TIMSK1, r16
    sei                            ; Global enable
    ret
```

## 3. The ADC (Analog to Digital Converter)
The ADC converts a voltage (0-5V) into a 10-bit number (0-1023).

**Reading Analog Pin 0:**
```assembly
adc_read:
    ldi r16, (1<<REFS0)     ; Use AVcc as reference, Channel 0
    sts ADMUX, r16
    ldi r16, (1<<ADEN) | (1<<ADSC) | (1<<ADPS2) | (1<<ADPS1) ; Enable + Start + Prescaler
    sts ADCSRA, r16

wait_adc:
    lds r16, ADCSRA
    sbrc r16, ADSC          ; Skip if Start Conversion bit is cleared (done)
    rjmp wait_adc
    
    lds r24, ADCL           ; ALWAYS read Low byte first!
    lds r25, ADCH           ; Result is now in r25:r24
    ret
```

## 4. Power Management and Sleep
If your device runs on a battery, you must use sleep modes.
```assembly
power_save:
    ldi r16, (1<<SM1) | (1<<SE) ; Power-down mode + Sleep Enable
    out SMCR, r16
    sleep                       ; CPU stops here until an interrupt
```

## 5. The Watchdog Timer (WDT)
If your code hangs (due to a bug or cosmic rays), the WDT will reset the CPU automatically.
```assembly
wdt_reset:
    wdr                         ; "Watchdog Reset" - Pet the dog!
```

---

# Part 8: Advanced Assembly Techniques

### 1. Fixed-Point Arithmetic
AVR has no Floating Point Unit (FPU). If you need decimals (like 12.5), you use fixed-point.
*   Example: Use 1 byte for the whole number and 1 byte for the fraction (scaled by 256).

### 2. The Stack Frame
When writing complex functions, you use the `Y` register (`r29:r28`) as a **Frame Pointer** to access local variables on the stack.
```assembly
push r28                ; Save old Y
push r29
in r28, SPL             ; Copy Stack Pointer to Y
in r29, SPH
sbiw r28, 4             ; Allocate 4 bytes for locals
out SPL, r28            ; Update Stack Pointer
out SPH, r29
; ... use 0(Y), 1(Y), etc. ...
```

### 3. Optimized Multi-byte Math
To add two 32-bit numbers:
```assembly
add r16, r20            ; Add Byte 0
adc r17, r21            ; Add Byte 1 with Carry from Byte 0
adc r18, r22            ; Add Byte 2 with Carry
adc r19, r23            ; Add Byte 3 with Carry
```

---

# Part 9: Comparison: C vs. Assembly Performance

| Operation | C Code (Compiled -Os) | Assembly (Hand-Optimized) |
| :--- | :--- | :--- |
| Bit Flip | 2-4 cycles | 1-2 cycles (`sbi`/`cbi`) |
| 16-bit Add | ~4-6 cycles | 2 cycles |
| Function Call | ~10-15 cycles (prologue/epilogue) | 3-5 cycles |
| ISR Entry | ~20+ cycles | 4 cycles (if you only save SREG) |

**The Golden Rule:** Use C for 95% of your logic. Use Assembly for:
1.  Extremely high-speed interrupts (Bit-banging protocols like WS2812B LEDs).
2.  Precise timing loops that cannot be interrupted.
3.  Mathematical kernels that the compiler struggles to optimize.

---

# Appendix: Useful "Cheat Sheet" Values for ATmega328P

*   **SRAM Start:** `0x0100`
*   **SRAM End:** `0x08FF`
*   **Flash Size:** 32,768 bytes (16,384 words)
*   **EEPROM Size:** 1,024 bytes
*   **I/O Range:** `0x00` to `0x3F` (Use `in`/`out`)
*   **Ext I/O Range:** `0x60` to `0xFF` (Use `lds`/`sts`)

### Final Reference Links
*   [AVR Instruction Set (Deep Reference)](http://ww1.microchip.com/downloads/en/devicedoc/atmel-0856-avr-instruction-set-manual.pdf)
*   [ATmega328P Register Map](https://onlinedocs.microchip.com/pr/GUID-0604A66B-EA7F-4DB9-B4B5-519A4DB97A7E-en-US-10/index.html)

---

# Part 10: Logic and Control Flow Patterns

Assembly has no `if` or `for`. You build them using **Compare** and **Branch** instructions.

### 1. The "If-Else" Pattern
In C: `if (r16 == 10) { A } else { B }`
In ASM:
```assembly
    cpi r16, 10       ; Compare r16 with 10
    brne do_else      ; Branch if Not Equal to 'do_else'
    
    ; --- If Block (A) ---
    sbi PORTB, 0
    rjmp end_if       ; Must jump over the else block!

do_else:
    ; --- Else Block (B) ---
    cbi PORTB, 0

end_if:
```

### 2. The "For Loop" Pattern
In C: `for (uint8_t i=0; i < 10; i++) { ... }`
In ASM:
```assembly
    ldi r16, 0        ; i = 0
loop_start:
    cpi r16, 10       ; is i < 10?
    brsh loop_end     ; Branch if Same or Higher (>= 10) to end
    
    ; ... loop body ...
    
    inc r16           ; i++
    rjmp loop_start
loop_end:
```

---

# Part 11: Memory Mastery

### 1. Optimized `memcpy` (Memory Copy)
Copying a block of data from one SRAM location to another using the `X` and `Y` pointers.

```assembly
; Copy 32 bytes from SRAM 0x0100 to 0x0200
    ldi XL, lo8(0x0100)
    ldi XH, hi8(0x0100)
    ldi YL, lo8(0x0200)
    ldi YH, hi8(0x0200)
    ldi r16, 32       ; counter

copy_loop:
    ld  r17, X+       ; Load from X, then increment X
    st  Y+, r17       ; Store to Y, then increment Y
    dec r16
    brne copy_loop
```

### 2. Reading from Flash (Program Memory)
Since AVR is Harvard architecture, you can't use `ld` to read constants (like strings) stored in Flash. You must use the `Z` pointer and the `LPM` (Load Program Memory) instruction.

```assembly
my_string: .asciz "Hello"  ; String stored in Flash

print_msg:
    ldi ZL, lo8(my_string)
    ldi ZH, hi8(my_string)
read_char:
    lpm r24, Z+       ; Load char from Flash, increment Z
    tst r24           ; Check if it's the null terminator (0)
    breq done
    rcall uart_send   ; Call your UART function
    rjmp read_char
done:
    ret
```

### 3. Writing to EEPROM
Writing to EEPROM is slow and requires a specific hardware "handshake" to prevent accidental writes.

```assembly
eeprom_write:
    ; 1. Wait for completion of previous write
    sbic EECR, EEPE
    rjmp eeprom_write
    
    ; 2. Set up address (r18:r17) and data (r16)
    out EEARH, r18
    out EEARL, r17
    out EEDR, r16
    
    ; 3. Write Master Write Enable
    sbi EECR, EEMPE
    ; 4. Within 4 cycles, set Write Enable
    sbi EECR, EEPE
    ret
```

---

# Part 12: The ALU and the Status Register (SREG)

Every calculation affects the `SREG` (Address `0x3F`). This is the "brain" of your control flow.

*   **Z (Zero):** Set if result was `00000000`.
*   **C (Carry):** Set if an unsigned addition overflowed (e.g., `255 + 1`).
*   **N (Negative):** Set if the most significant bit (MSB) is 1.
*   **V (Two's Complement Overflow):** Set if a signed calculation went out of range.

### Multiplication Secret
When you use `mul r16, r17`, the result is 16 bits.
*   The **Low Byte** is always placed in `r0`.
*   The **High Byte** is always placed in `r1`.
*   *Warning:* C compilers use `r1` as a `__zero_reg__`. If you use `mul` in mixed code, you **must** clear `r1` (`eor r1, r1`) before returning to C!
