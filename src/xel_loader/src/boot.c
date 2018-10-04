/*
 *  boot.c
 *
 *  Created on: 2016. 7. 13.
 *      Author: Baram
 */


#include "boot.h"
#include "cmd.h"
#include "util.h"
#include "uart.h"
#include "string.h"


typedef struct
{
  uint32_t type;          // 0
  uint32_t fw_addr;       // 4
  uint32_t length;        // 8
  uint16_t crc;           // 12
  uint8_t  reserved[498]; // 14
} file_tag_t;

#define BOOT_CMD_READ_VERSION           0x80
#define BOOT_CMD_PACKET_DATA_SIZE_MAX   0x81
#define BOOT_CMD_READ_BOARD_NAME        0x82
#define BOOT_CMD_FLASH_ERASE            0x83
#define BOOT_CMD_FLASH_WRITE            0x84
#define BOOT_CMD_FLASH_READ             0x85
#define BOOT_CMD_FLASH_VERIFY           0x86
#define BOOT_CMD_JUMP                   0x87



cmd_t cmd_boot;


uint8_t boot_cmd_buffer[16*1024];



bool bootInit(uint8_t channel, char *port_name, uint32_t baud)
{

  cmdInit(&cmd_boot);

  if (cmdBegin(&cmd_boot, channel, port_name, baud) == false)
  {
    return false;
  }

  return true;
}

bool bootReset(uint8_t channel, char *port_name, uint32_t baud)
{
  uartOpen(channel, port_name, baud);
  uartPrintf(channel, USB_AUTO_BOOT_CMD);
  uartClose(channel);
  delay(2000);
  return true;
}


err_code_t bootCmdAddTagToBIN(uint32_t fw_addr, char *src_filename, char *dst_filename)
{
  FILE    *p_fd;
  uint8_t *buf;
  size_t   src_len;
  uint16_t t_crc = 0;
  file_tag_t file_tag;

  if (!strcmp(src_filename, dst_filename))
  {
    fprintf( stderr, "  src file(%s) and dst file(%s) is same! \n", src_filename, dst_filename );
    return NOK;
  }


  /* Open src file */
  if ((p_fd = fopen(src_filename, "rb")) == NULL)
  {
    fprintf( stderr, "  Unable to open src file(%s)\n", src_filename );
    return NOK;
  }

  fseek( p_fd, 0, SEEK_END );
  src_len = ftell( p_fd );
  fseek( p_fd, 0, SEEK_SET );

  if ((buf = (uint8_t *) calloc(_DEF_TAG_SIZE + src_len, sizeof(uint8_t))) == NULL)
  {
    fclose(p_fd);
    fprintf( stderr, "  Malloc Error \n");
    return NOK;
  }


  /* Copy read fp to buf */
  if(fread( &buf[_DEF_TAG_SIZE], 1, src_len, p_fd ) != src_len)
  {
    fclose(p_fd);
    free(buf);
    fprintf( stderr, "  CRC length is wrong! \n" );
    return NOK;
  }
  fclose(p_fd);


  /* Calculate CRC16 */
  size_t i;
  for (i = 0; i<src_len; i++)
  {
    utilUpdateCrc(&t_crc, buf[_DEF_TAG_SIZE + i]);
  }

  memset(&file_tag, 0, sizeof(file_tag_t));
  file_tag.type    = 0;
  file_tag.fw_addr = _DEF_TAG_SIZE + fw_addr;
  file_tag.length  = src_len;
  file_tag.crc     = t_crc;
  memcpy(buf, (uint8_t*) &file_tag, sizeof(file_tag_t));


  /* Store data to dst file */
  if ((p_fd = fopen(dst_filename, "wb")) == NULL)
  {
    free(buf);
    fprintf( stderr, "  Unable to open dst file(%s)\n", dst_filename );
    return NOK;
  }

  if(fwrite(buf, 1, _DEF_TAG_SIZE + src_len, p_fd) != _DEF_TAG_SIZE + src_len)
  {
    fclose(p_fd);
    free(buf);
    //_unlink(dst_filename);
    fprintf( stderr, "  Total write fail! \n" );
    return NOK;
  }

  fclose(p_fd);
  free(buf);

  printf("  created file  : %s (%d KB)\n", dst_filename, (int)((_DEF_TAG_SIZE + src_len)/1024) );
  printf("  tag fw_addr   : 0x%08X \n", file_tag.fw_addr);
  printf("  tag length    : %d KB (0x%X)\n", (int)(file_tag.length/1024), file_tag.length);
  printf("  tag crc       : 0x%04X \n", file_tag.crc);

  return OK;
}

err_code_t bootCmdReadVersion(uint8_t *p_version)
{
  err_code_t errcode = OK;
  cmd_t *p_cmd = &cmd_boot;
  uint32_t i;


  errcode = cmdSendCmdRxResp(p_cmd, BOOT_CMD_READ_VERSION, NULL, 0, 1000);
  if (errcode == OK)
  {
    for (i=0; i<2; i++)
    {
      p_version[i] = p_cmd->rx_packet.data[i];
    }
  }

  return errcode;
}

err_code_t bootCmdReadPacketDataSizeMax(uint16_t *p_size)
{
  err_code_t errcode = OK;
  cmd_t *p_cmd = &cmd_boot;

  errcode = cmdSendCmdRxResp(p_cmd, BOOT_CMD_PACKET_DATA_SIZE_MAX, NULL, 0, 1000);
  if (errcode == OK)
  {
    *p_size  = p_cmd->rx_packet.data[0];
    *p_size |= p_cmd->rx_packet.data[1] << 8;
  }

  return errcode;
}

err_code_t bootCmdReadBoardName(uint8_t *p_str)
{
  err_code_t errcode = OK;
  cmd_t *p_cmd = &cmd_boot;
  uint32_t i;


  errcode = cmdSendCmdRxResp(p_cmd, BOOT_CMD_READ_BOARD_NAME, NULL, 0, 1000);
  if (errcode == OK)
  {
    for (i=0; i<p_cmd->rx_packet.length; i++)
    {
      p_str[i] = p_cmd->rx_packet.data[i];
    }
    p_str[i] = 0;
  }

  return errcode;
}


err_code_t bootCmdFlashErase(uint32_t addr, uint32_t length)
{
  err_code_t errcode = OK;
  cmd_t *p_cmd = &cmd_boot;
  uint8_t data[32];
  uint32_t index = 0;


  data[index++] = addr >> 0;
  data[index++] = addr >> 8;
  data[index++] = addr >> 16;
  data[index++] = addr >> 24;

  data[index++] = length >> 0;
  data[index++] = length >> 8;
  data[index++] = length >> 16;
  data[index++] = length >> 24;


  errcode = cmdSendCmdRxResp(p_cmd, BOOT_CMD_FLASH_ERASE, data, index, 3000);

  return errcode;
}


err_code_t bootCmdFlashWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  err_code_t errcode = OK;
  cmd_t *p_cmd = &cmd_boot;
  uint32_t i;
  uint8_t *data = boot_cmd_buffer;
  uint32_t index = 0;


  data[index++] = addr >> 0;
  data[index++] = addr >> 8;
  data[index++] = addr >> 16;
  data[index++] = addr >> 24;

  data[index++] = length >> 0;
  data[index++] = length >> 8;
  data[index++] = length >> 16;
  data[index++] = length >> 24;

  for (i=0; i<length; i++)
  {
    data[index++] = p_data[i];
  }

  errcode = cmdSendCmdRxResp(p_cmd, BOOT_CMD_FLASH_WRITE, data, index, 3000);

  return errcode;
}


err_code_t bootCmdFlashVerfy(uint32_t addr, uint32_t length, uint16_t tx_crc, uint16_t *rx_crc)
{
  err_code_t errcode = OK;
  cmd_t *p_cmd = &cmd_boot;
  uint8_t *data = boot_cmd_buffer;
  uint32_t index = 0;


  data[index++] = addr >> 0;
  data[index++] = addr >> 8;
  data[index++] = addr >> 16;
  data[index++] = addr >> 24;

  data[index++] = length >> 0;
  data[index++] = length >> 8;
  data[index++] = length >> 16;
  data[index++] = length >> 24;

  data[index++] = tx_crc >> 0;
  data[index++] = tx_crc >> 8;

  errcode = cmdSendCmdRxResp(p_cmd, BOOT_CMD_FLASH_VERIFY, data, index, 10000);

  *rx_crc  = p_cmd->rx_packet.data[0];
  *rx_crc |= p_cmd->rx_packet.data[1] << 8;

  return errcode;
}

err_code_t bootCmdJumpToAddress(uint32_t addr)
{
  err_code_t errcode = OK;
  cmd_t *p_cmd = &cmd_boot;
  uint8_t *data = boot_cmd_buffer;
  uint32_t index = 0;


  data[index++] = addr >> 0;
  data[index++] = addr >> 8;
  data[index++] = addr >> 16;
  data[index++] = addr >> 24;

  errcode = cmdSendCmdRxResp(p_cmd, BOOT_CMD_JUMP, data, index, 1000);

  return errcode;
}

