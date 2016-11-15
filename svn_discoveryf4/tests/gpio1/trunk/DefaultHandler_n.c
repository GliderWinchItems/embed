/* DefaultHandler_n.c   */
int DefaultHandlerCode;

void DefaultHandler01(void) { DefaultHandlerCode = 01; panic_leds(5); }
void DefaultHandler02(void) { DefaultHandlerCode = 02; panic_leds(5); }

