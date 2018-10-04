/*
 *  boot.h
 *
 *  Created on: 2016. 5. 14.
 *      Author: Baram
 */

#ifndef BOOT_H
#define BOOT_H


#ifdef __cplusplus
extern "C" {
#endif


#include "def.h"


#define USB_AUTO_BOOT_CMD  "RESET XELNETWORK"
#define USB_AUTO_BOOT_BAUD 1200


bool bootInit(uint8_t channel, char *port_name, uint32_t baud);
bool bootReset(uint8_t channel, char *port_name, uint32_t baud);

err_code_t bootCmdAddTagToBIN(uint32_t fw_addr, char *src_filename, char *dst_filename);
err_code_t bootCmdReadVersion(uint8_t *p_version);
err_code_t bootCmdReadPacketDataSizeMax(uint16_t *p_size);
err_code_t bootCmdReadBoardName(uint8_t *p_str);
err_code_t bootCmdFlashErase(uint32_t addr, uint32_t length);
err_code_t bootCmdFlashWrite(uint32_t addr, uint8_t *p_data, uint32_t length);
err_code_t bootCmdFlashVerfy(uint32_t addr, uint32_t length, uint16_t tx_crc, uint16_t *rx_crc);
err_code_t bootCmdJumpToAddress(uint32_t addr);

#ifdef __cplusplus
}
#endif


#endif
