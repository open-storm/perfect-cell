#include "sensors_uart_control.h"

static const uint32_t MASTER_CLOCK_FREQ = 24000000u;  //  24 MHz

static char sensors_uart_buf[257] = {'\0'};
static uint8_t buf_idx = 0u;

void sensors_uart_start() {
    Sensors_UART_Start();
    isr_sensors_uart_rx_StartEx(isr_sensors_uart_rx);
}

void sensors_uart_stop() {
    isr_sensors_uart_rx_Stop();
    Sensors_UART_Stop();
}

inline static unsigned int pos_round(const float val) { return val + 0.5f; }

uint8_t sensors_uart_set_baud(const uint32_t baud) {
    const uint32_t desired_clock = 8 * baud;
    const uint16_t divider =
        pos_round((float) MASTER_CLOCK_FREQ / desired_clock);

    Clock_sensors_SetDividerValue(divider);
    return Clock_sensors_GetDividerRegister() + 1u == divider;
}

char* sensors_uart_get_string() { return sensors_uart_buf; }

size_t sensors_uart_get_string_size() {
    return buf_idx;  // equivalent to strlen(sensors_uart_buf)
}

void sensors_uart_clear_string() {
    Sensors_UART_ClearRxBuffer();
    memset(sensors_uart_buf, '\0', sizeof(sensors_uart_buf));
    buf_idx = 0u;
}

CY_ISR(isr_sensors_uart_rx) {
    // hold the next char in the rx register as a temporary variable
    char rx_char = Sensors_UART_GetChar();

    // store the char in sensors_uart_buf
    if (rx_char) {
        // unsigned ints don't overflow, they reset at 0 if they are incremented
        // one more than it's max value. It will be obvious if an `overflow`
        // occurs
        sensors_uart_buf[buf_idx++] = rx_char;
    }
}
