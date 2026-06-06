#ifndef EV_CONTROL_H
#define EV_CONTROL_H

/* EV speed model, SOC, torque, power, range, drive modes */

typedef enum { MODE_ECO, MODE_NORMAL, MODE_SPORT } DriveMode_t;

typedef struct {
    float speed_kmh;
    float soc_pct;
    float torque_nm;
    float power_kw;
    float range_km;
    float motor_temp_c;
    DriveMode_t drive_mode;
} EVState_t;

void EV_Init(void);
void EV_Update(void);
EVState_t* EV_GetState(void);

#endif /* EV_CONTROL_H */
