#ifndef __WIFI_H__
#define __WIFI_H__

void wifi_init(void);

void wifi_send_data(uint8_t *data, uint16_t len);
void wifi_recv_data(uint8_t *data, uint16_t len);
void wifi_parse_buf(void);

void wifi_test(void);

#endif //__WIFI_H__
