#ifndef SPI_H
#define SPI_H

#include "stm32f4xx.h"
#include <stdio.h>

/* Pin mapping (Nucleo-F411RE, SPI1)
 * SCK  -> PA5  (AF5)
 * MISO -> PA6  (AF5)
 * MOSI -> PA7  (AF5)
 * CS   -> PB6  (GPIO output, manual control)
 * RST  -> PC7  (GPIO output, manual control)
 */

void SPI1_Init(void);
uint8_t SPI1_TransferByte(uint8_t data);

void W5500_CS_Low(void);
void W5500_CS_High(void);

void W5500_Reset(void);

#endif /* SPI_H */
