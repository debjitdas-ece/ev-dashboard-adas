/* Real-Time EV Dashboard + ADAS Warning System
 * STM32F103C8T6 — STM32CubeIDE — HAL Framework
 * Emertxe Internship @ Jayalaxmi Institute, Jun–Aug 2026
 */

#include "main.h"
#include "ev_control.h"
#include "adas.h"
#include "fault.h"
#include "uart_shell.h"
#include "buzzer.h"

int main(void) {
    HAL_Init();
    SystemClock_Config();

    EV_Init();
    ADAS_Init();
    Fault_Init();
    UART_Shell_Init();
    Buzzer_Init();

    while (1) {
        EV_Update();
        ADAS_Update();
        Fault_Evaluate();
        UART_Shell_Process();
    }
}
