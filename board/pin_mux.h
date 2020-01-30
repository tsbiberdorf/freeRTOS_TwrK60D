/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

#define SOPT5_UART0RXSRC_UART_RX 0x00u /*!<@brief UART 0 receive data source select: UART0_RX pin */
#define SOPT5_UART0TXSRC_UART_TX 0x00u /*!<@brief UART 0 transmit data source select: UART0_TX pin */

/*! @name PORTC1 (coord B11), HV_PH2
  @{ */
#define BOARD_INITPINS_HV_PH2_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_HV_PH2_PIN 1U     /*!<@brief PORTC pin index: 1 */
                                         /* @} */

/*! @name PORTC2 (coord A12), HV_PH1
  @{ */
#define BOARD_INITPINS_HV_PH1_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_HV_PH1_PIN 2U     /*!<@brief PORTC pin index: 2 */
                                         /* @} */

/*! @name PORTC3 (coord A11), ch2
  @{ */
#define BOARD_INITPINS_ch2_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_ch2_PIN 3U     /*!<@brief PORTC pin index: 3 */
                                      /* @} */

/*! @name PORTA6 (coord J7), ch3
  @{ */
#define BOARD_INITPINS_ch3_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_ch3_PIN 6U     /*!<@brief PORTA pin index: 6 */
                                      /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
