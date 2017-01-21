#include <asf.h>

#include <string.h>

#include "wifi.h"

#define MAX_WIFI_BUF_LEN  2048
uint8_t wifi_buf[MAX_WIFI_BUF_LEN];
uint16_t wifi_buf_len;
uint8_t wifi_cmd_buf[MAX_WIFI_BUF_LEN];
uint8_t wifi_token[MAX_WIFI_BUF_LEN];
uint8_t wifi_ssid[64];

void wifi_init(void)
{
  uint8_t atstart[13] = {0xA5, 0x01, 0xF2, 0x00, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x61, 0x2F, 0x52, 0xEF}; //at start command

  usart_serial_options_t usart_options = {
    .baudrate = CONF_WIFI_UART_BAUDRATE,
    #ifdef CONF_UART_CHAR_LENGTH
    .charlength = CONF_UART_CHAR_LENGTH,
    #endif
    .paritytype = CONF_WIFI_UART_PARITY,
    #ifdef CONF_UART_STOP_BITS
    .stopbits = CONF_UART_STOP_BITS,
    #endif
  };
  /* Configure console UART. */
  usart_serial_init((usart_if)CONF_WIFI_UART, &usart_options);

  uart_enable_interrupt(UART1,UART_IER_RXRDY);
  NVIC_EnableIRQ(UART1_IRQn);

  //reset pins
  gpio_set_pin_low(WIFI_WAKE_GPIO);
  gpio_set_pin_low(WIFI_RESET_GPIO);
  gpio_set_pin_low(WIFI_EN_GPIO);
  delay_ms(100);
  gpio_set_pin_high(WIFI_WAKE_GPIO);
  gpio_set_pin_high(WIFI_EN_GPIO);
  delay_ms(10);
  gpio_set_pin_high(WIFI_RESET_GPIO);
  delay_ms(100);

  //Start AT Command
  usart_serial_write_packet((usart_if)CONF_WIFI_UART,atstart, 13);
  printf("Send ATCommand Start\r\n");
  delay_s(2);
  wifi_parse_buf();

  strcpy(wifi_cmd_buf, "AT+GETMAC\r\n");
  wifi_send_data(wifi_cmd_buf, strlen(wifi_cmd_buf));
  delay_ms(100);
  wifi_parse_buf();

  //Start AP
  sprintf(wifi_cmd_buf, "AT+AP=%s,0,3,hello\r\n", wifi_ssid);
  wifi_send_data(wifi_cmd_buf, strlen(wifi_cmd_buf));
  delay_ms(100);

  sprintf(wifi_cmd_buf, "AT+TCPSVR=8000,1\r\n");
  wifi_send_data(wifi_cmd_buf, strlen(wifi_cmd_buf));
  delay_ms(100);
  //printf("AP Started\r\n");
}

void wifi_recv_data(uint8_t *data, uint16_t len)
{
  //copy received packet to buffer
  for(int i=0;i<len;i++) {
    if(wifi_buf_len < MAX_WIFI_BUF_LEN) {
      wifi_buf[wifi_buf_len] = data[i];
      wifi_buf_len++;
    }
  }
}

void wifi_parse_token()
{
    uint8_t mac[8];
    if(strncmp(wifi_token,"+OK+AT", 6) == 0) {
        printf("Wifi Initialized. %s",wifi_token+7);
    } else if(strncmp(wifi_token, "+OK+GETMAC", 10) == 0) {
        strncpy(mac, wifi_token+20, 8);
        sprintf(wifi_ssid, "WINC1500_%s", mac);
        printf("SSID : %s\r\n", wifi_ssid);
    } else if(strncmp(wifi_token, "+OK+AP", 6) == 0) {
      printf("AP Started\r\n");
    } else if(strncmp(wifi_token, "+OK+TCPSVR", 10) == 0) {
      printf("TCP Server Started(%c)\r\n", wifi_token[11]);
    } else if(strncmp(wifi_token, "+EVT+CONN", 9) == 0) {
      printf("Client Connected\r\n");
    } else if(strncmp(wifi_token, "+EVT+ACCEPT", 11) == 0) { //FIXME
      wifi_cmd_buf[0] = 0x1; //<SOH>
      sprintf(wifi_cmd_buf+1, "S10007HELLO\r\n");
      wifi_cmd_buf[14] = 0x1;
      wifi_cmd_buf[15] = 'E';
      wifi_cmd_buf[16] = '\r';
      wifi_cmd_buf[17] = '\n';
      wifi_cmd_buf[18] = 0;
      wifi_send_data(wifi_cmd_buf, strlen(wifi_cmd_buf));
    } else if(strncmp(wifi_token, "+EVT+RECV", 9) == 0) {
      wifi_cmd_buf[0] = 0x1; //<SOH>
      sprintf(wifi_cmd_buf+1, "S10007HELLO\r\n");
      wifi_cmd_buf[14] = 0x1;
      wifi_cmd_buf[15] = 'E';
      wifi_cmd_buf[16] = '\r';
      wifi_cmd_buf[17] = '\n';
      wifi_cmd_buf[18] = 0;
      wifi_send_data(wifi_cmd_buf, strlen(wifi_cmd_buf));
    } else {
        if(wifi_token[0] != 0)
          printf("TOKEN = %s(%d)", wifi_token, strlen(wifi_token));
    }

    memset(wifi_token, 0, 2048);
}

void wifi_send_data(uint8_t *data, uint16_t len)
{
  usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)data, len);
}

void wifi_parse_buf(void) //FIXME
{
  int i,j;
  for(i=0;i<wifi_buf_len;i++) {
    if(wifi_buf[i] == '\n') {
      strncpy(wifi_token, wifi_buf, i+1);
      for(j=0; j<wifi_buf_len-i;j++) {
        wifi_buf[j] = wifi_buf[i+j+1];
      }
      wifi_buf_len -= (i+1);
      break;
    }
  }

  if(i==wifi_buf_len) //Something Wrong
    wifi_buf_len = 0;

  wifi_parse_token();
}

void wifi_test()
{
  delay_ms(1000);
  usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"AT\r\n", 4);
  delay_ms(1000);
  usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"AT+LIST\r\n", 9);
  delay_ms(1000);
  usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"AT+CHIPID\r\n",11);
  delay_ms(1000);
  usart_serial_write_packet((usart_if)CONF_WIFI_UART,(const uint8_t *)"AT+AP=WINC1500,0,1\r\n",20);
}