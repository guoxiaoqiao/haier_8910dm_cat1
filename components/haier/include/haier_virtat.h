#ifndef _HAIER_VIRTAT_H_
#define _HAIER_VIRTAT_H_

#include "haier_appmain.h"

typedef void (*vat_cmd_data_procsee)(char *rsp_buff, uint32_t len);

typedef struct
{
    char *cmd_str;  //command string

    vat_cmd_data_procsee vat_rsp_data_process_handler;
    
}vat_cmd_cb_s;

extern uint8_t get_vat_init_status(void);
extern void set_vat_init_status(uint8_t status);

#endif