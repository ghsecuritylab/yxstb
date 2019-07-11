#ifndef __YX_UART_H__
#define __YX_UART_H__

/*yx common */
#include "yx_type.h"

YX_S32  yx_middle_drv_uart_open(YX_S32 uart_no,YX_S32 baudrate);
YX_VOID yx_middle_drv_uart_close(YX_S32 uart_no);
YX_S32  yx_middle_drv_uart_read(YX_U8 *rd_buf, YX_U32 rd_buf_len, YX_U32 *number_read,YX_U64 uS_timeout);
YX_S32  yx_middle_drv_uart_write(YX_U8 *write_buf,YX_U32 write_buf_len);

#endif /*__YX_UART_H__*/


