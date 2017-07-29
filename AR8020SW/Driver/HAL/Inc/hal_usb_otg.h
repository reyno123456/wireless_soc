#ifndef __HAL_USB_OTG_H__
#define __HAL_USB_OTG_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    HAL_USB_PORT_0 = 0,
    HAL_USB_PORT_1,
    HAL_USB_PORT_NUM,
} ENUM_HAL_USB_PORT;


/**
* @brief    initiate the USB Port
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_InitOTG(ENUM_HAL_USB_PORT e_usbPort);


/**
* @brief    Configure the parameters optimized by IC Designer
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_ConfigPHY(void);

#ifdef __cplusplus
}
#endif

#endif

