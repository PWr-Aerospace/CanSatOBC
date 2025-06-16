#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void print(const char* str);
void uart4_receive_to_idle();
void uart4_setup();
void uart6_setup();

#ifdef __cplusplus
}
#endif
