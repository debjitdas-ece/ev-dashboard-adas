/**
 * @file    buzzer.c
 * @brief   Non-blocking buzzer driver — PC13 (plain digital GPIO)
 *
 * PC13 has no timer channel on STM32F103 (it's the onboard Blue Pill LED
 * pin), so this drives it as a simple on/off square wave via the beep
 * pattern timing only — no PWM tone frequency, just the same
 * ADVISORY/WARNING/CRITICAL on/off cadence as before. Most PicSimLab
 * buzzer parts just respond to digital HIGH/LOW anyway.
 *
 * Deliberately does NOT use HAL_Delay() anywhere — the main loop already has
 * one blocking-delay problem (HCSR04_ReadAll's 150ms of HAL_Delay(50)s);
 * this driver is a small state machine ticked from Buzzer_Process() so it
 * never adds a second one.
 */

#include "buzzer.h"

#define BUZZER_PORT  GPIOC
#define BUZZER_PIN   GPIO_PIN_13

static BuzzerTone_t _pattern        = BUZZER_OFF;
static uint32_t     _last_toggle_ms = 0;
static uint8_t      _tone_on        = 0;

/* on/off timing (ms) per pattern — same cadence as before, minus the
 * PWM tone frequency (PC13 can't do hardware PWM). */
typedef struct {
    uint16_t on_ms;
    uint16_t off_ms;
} BuzzerProfile_t;

static const BuzzerProfile_t PROFILES[4] = {
    [BUZZER_OFF]    = {   0,   0 },
    [BUZZER_SINGLE] = { 150, 850 },  /* one short beep/sec   — advisory */
    [BUZZER_DOUBLE] = { 120, 200 },  /* quick double beep    — warning  */
    [BUZZER_RAPID]  = {  80,  80 },  /* continuous fast beep — critical */
};

void Buzzer_Init(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);  /* silent at boot */
}

void Buzzer_SetPattern(BuzzerTone_t tone)
{
    if (tone == _pattern) return;   /* already running this pattern */
    _pattern        = tone;
    _tone_on        = 0;
    _last_toggle_ms = HAL_GetTick();
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);  /* mute until next phase */
}

void Buzzer_Process(void)
{
    if (_pattern == BUZZER_OFF) {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
        return;
    }

    const BuzzerProfile_t *p = &PROFILES[_pattern];
    uint32_t now     = HAL_GetTick();
    uint32_t elapsed = now - _last_toggle_ms;
    uint32_t target  = _tone_on ? p->on_ms : p->off_ms;

    if (elapsed >= target) {
        _tone_on        = !_tone_on;
        _last_toggle_ms = now;
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN,
                           _tone_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}
