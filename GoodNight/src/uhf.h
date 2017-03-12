#ifndef __UHF_H__
#define __UHF_H__

void uhf_init(void);

void uhf_test(void);

void uhf_load_data(uint32_t truck_id, uint32_t tire_id, uint16_t *pressure, uint16_t *voltage, uint16_t *temp);
void uhf_save_data(uint32_t truck_id, uint32_t tire_id, uint16_t pressure, uint16_t voltage, uint16_t temp);
#endif //__UHF_H__
