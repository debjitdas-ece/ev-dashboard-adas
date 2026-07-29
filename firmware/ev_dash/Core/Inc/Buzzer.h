/**
 * @file    buzzer.h
 * @brief   Non-blocking buzzer driver — PC13 (plain digital GPIO)
 *
 * BuzzerTone_t (BUZZER_OFF/SINGLE/DOUBLE/RAPID) is defined in common.h and
 * was already referenced (commented out) in uart_shell.c's "alarm test"
 * command — this file is the missing driver that makes it real.
 */

#ifndef BUZZER_H
#define BUZZER_H

#include "common.h"

void Buzzer_Init(void);
void Buzzer_SetPattern(BuzzerTone_t tone);
void Buzzer_Process(void);   /* call every main-loop iteration — non-blocking */

#endif /* BUZZER_H */
