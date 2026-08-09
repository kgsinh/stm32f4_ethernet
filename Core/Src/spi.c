#include "spi.h"

/* Crude blocking delay - replace with a timer-based delay if you have one */
static void delay_loop(volatile uint32_t count)
{
    while (count--)
    {
        __NOP();
    }
}

static void SPI1_GPIO_Init(void)
{
    /* Enable clocks: GPIOA (SCK/MISO/MOSI), GPIOB (CS), GPIOC (RST) */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    /* PA5, PA6, PA7 -> Alternate Function mode (MODER = 10) */
    GPIOA->MODER &= ~((3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));
    GPIOA->MODER |=  ((2U << (5 * 2)) | (2U << (6 * 2)) | (2U << (7 * 2)));

    /* AF5 = SPI1 on PA5/PA6/PA7 (see STM32F411 datasheet AF mapping table) */
    GPIOA->AFR[0] &= ~((0xFU << (5 * 4)) | (0xFU << (6 * 4)) | (0xFU << (7 * 4)));
    GPIOA->AFR[0] |=  ((5U   << (5 * 4)) | (5U   << (6 * 4)) | (5U   << (7 * 4)));

    /* Output type: push-pull (default, OTYPER bit = 0) for SCK/MOSI.
     * MISO is input from the slave's perspective but still configured AF push-pull here. */
    GPIOA->OTYPER &= ~((1U << 5) | (1U << 6) | (1U << 7));

    /* Speed: high speed (11) on SCK/MISO/MOSI */
    GPIOA->OSPEEDR |= (3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2));

    /* No pull-up/pull-down */
    GPIOA->PUPDR &= ~((3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));

    /* PB6 -> general purpose output (CS), MODER = 01 */
    GPIOB->MODER &= ~(3U << (6 * 2));
    GPIOB->MODER |=  (1U << (6 * 2));
    GPIOB->OTYPER &= ~(1U << 6);          /* push-pull */
    GPIOB->OSPEEDR |= (3U << (6 * 2));    /* high speed */
    GPIOB->PUPDR &= ~(3U << (6 * 2));

    /* PC7 -> general purpose output (RST), MODER = 01 */
    GPIOC->MODER &= ~(3U << (7 * 2));
    GPIOC->MODER |=  (1U << (7 * 2));
    GPIOC->OTYPER &= ~(1U << 7);          /* push-pull */
    GPIOC->OSPEEDR |= (3U << (7 * 2));    /* high speed */
    GPIOC->PUPDR &= ~(3U << (7 * 2));

    /* CS idle high (deasserted), RST idle high (not in reset) */
    GPIOB->BSRR = (1U << 6);
    GPIOC->BSRR = (1U << 7);
}

static void SPI1_Peripheral_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* Ensure SPI1 disabled while configuring */
    SPI1->CR1 &= ~SPI_CR1_SPE;

    /* CR1 configuration:
     * MSTR   = 1  -> master mode
     * CPOL   = 0  -> clock idle low   (W5500 requires SPI mode 0)
     * CPHA   = 0  -> data sampled on first clock edge
     * BR     = 100 -> APB2 clock / 32  (conservative speed for first bring-up;
     *                 e.g. 84 MHz / 32 = 2.625 MHz, adjust once link is verified)
     * SSM/SSI = 1 -> software slave management, internal slave select forced high
     *                (required in master mode when NSS pin is not hardware-controlled)
     * DFF    = 0  -> 8-bit data frame
     * LSBFIRST = 0 -> MSB first (W5500 requirement)
     */
    SPI1->CR1 = 0;
    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 |= (4U << SPI_CR1_BR_Pos); /* BR[2:0] = 100 -> /32 */
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;

    /* CR2: nothing special needed for basic polling transfer */
    SPI1->CR2 = 0;

    SPI1->CR1 |= SPI_CR1_SPE; /* enable SPI1 */
}

void SPI1_Init(void)
{
    SPI1_GPIO_Init();
    SPI1_Peripheral_Init();
    printf("[SPI] SPI1 initialized (PA5=SCK, PA6=MISO, PA7=MOSI, PB6=CS, PC7=RST)\r\n");
}

uint8_t SPI1_TransferByte(uint8_t data)
{
    /* Wait until TX buffer empty */
    while (!(SPI1->SR & SPI_SR_TXE))
    {
    }

    *(volatile uint8_t *)&SPI1->DR = data;

    /* Wait until RX buffer has data */
    while (!(SPI1->SR & SPI_SR_RXNE))
    {
    }

    return (uint8_t)SPI1->DR;
}

void W5500_CS_Low(void)
{
    GPIOB->BSRR = (1U << (6 + 16)); /* reset PB6 */
}

void W5500_CS_High(void)
{
    GPIOB->BSRR = (1U << 6); /* set PB6 */
}

void W5500_Reset(void)
{
    GPIOC->BSRR = (1U << (7 + 16)); /* RST low */
    delay_loop(100000);             /* hold low - replace with datasheet-accurate timed delay */
    GPIOC->BSRR = (1U << 7);        /* RST high */
    delay_loop(400000);             /* wait for W5500 internal PLL lock / ready - check datasheet timing */
    printf("[SPI] W5500 hardware reset complete\r\n");
}