// #define PWM_PIN 7
// #define LDR_PIN 4
// #define RESET_PIN 6
// #define RGB_PIN 5

// #define OLED_SDA 2
// #define OLED_SCL 3

// #if BUILD_WITH_4G



// #endif



#if BUILD_WITH_4G

#define PWM_PIN 10
#define LDR_PIN 12
#define RESET_PIN 11
#define RGB_PIN 8

#define OLED_SDA 7
#define OLED_SCL 6

#define UART_TX 17
#define UART_RX 18

#else

#define PWM_PIN 7
#define LDR_PIN 4
#define RESET_PIN 6
#define RGB_PIN 5

#define OLED_SDA 2
#define OLED_SCL 3

#endif