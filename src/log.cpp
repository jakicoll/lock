#include "log.h"

/**
 * This function comes from (arduino-lmic) src/hal/hal.c
 *
 * copied, so we don't have to include the full hal.c
 * only to get the same timestamps in our logs
 *
 * modification: use esp_timer_get_time() instead of millis()
 *               so we don't have to include Arduino.h here
 */
uint32_t log_ticks() {
  // Because micros() is scaled down in this function, micros() will
  // overflow before the tick timer should, causing the tick timer to
  // miss a significant part of its values if not corrected. To fix
  // this, the "overflow" serves as an overflow area for the micros()
  // counter. It consists of three parts:
  //  - The US_PER_OSTICK upper bits are effectively an extension for
  //    the micros() counter and are added to the result of this
  //    function.
  //  - The next bit overlaps with the most significant bit of
  //    micros(). This is used to detect micros() overflows.
  //  - The remaining bits are always zero.
  //
  // By comparing the overlapping bit with the corresponding bit in
  // the micros() return value, overflows can be detected and the
  // upper bits are incremented. This is done using some clever
  // bitwise operations, to remove the need for comparisons and a
  // jumps, which should result in efficient code. By avoiding shifts
  // other than by multiples of 8 as much as possible, this is also
  // efficient on AVR (which only has 1-bit shifts).
  static uint8_t overflow = 0;

  // Scaled down timestamp. The top US_PER_OSTICK_EXPONENT bits are 0,
  // the others will be the lower bits of our return value.
  uint32_t scaled =
      ((unsigned long)esp_timer_get_time()) >> LOG_US_PER_OSTICK_EXPONENT;
  // Most significant byte of scaled
  uint8_t msb = scaled >> 24;
  // Mask pointing to the overlapping bit in msb and overflow.
  const uint8_t mask = (1 << (7 - LOG_US_PER_OSTICK_EXPONENT));
  // Update overflow. If the overlapping bit is different
  // between overflow and msb, it is added to the stored value,
  // so the overlapping bit becomes equal again and, if it changed
  // from 1 to 0, the upper bits are incremented.
  overflow += (msb ^ overflow) & mask;

  // Return the scaled value with the upper bits of stored added. The
  // overlapping bit will be equal and the lower bits will be 0, so
  // bitwise or is a no-op for them.
  return scaled | ((uint32_t)overflow << 24);

  // 0 leads to correct, but overly complex code (it could just return
  // micros() unmodified), 8 leaves no room for the overlapping bit.
  static_assert(LOG_US_PER_OSTICK_EXPONENT > 0 &&
                    LOG_US_PER_OSTICK_EXPONENT < 8,
                "Invalid US_PER_OSTICK_EXPONENT value");
}
