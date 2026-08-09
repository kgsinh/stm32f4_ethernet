/* ============================================================
 * w5500_port.c — Bridge between spi.c and Wiznet ioLibrary
 * ============================================================ */
#include "wizchip_conf.h"
#include "spi.h"
#include <stdio.h>

/* ── SPI byte read (send dummy, return received byte) ── */
static uint8_t spi_rb(void)
{
    return SPI1_TransferByte(0xFF);
}

/* ── SPI byte write ── */
static void spi_wb(uint8_t wb)
{
    SPI1_TransferByte(wb);
}

/* ── SPI burst read ── */
static void spi_rbuf(uint8_t *pBuf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        pBuf[i] = SPI1_TransferByte(0xFF);
}

/* ── SPI burst write ── */
static void spi_wbuf(uint8_t *pBuf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        SPI1_TransferByte(pBuf[i]);
}

/* ── Critical section (bare-metal) ── */
static void cris_enter(void)
{
	__disable_irq();
}
static void cris_exit(void)
{
	__enable_irq();
}

/* ── One call wires everything ── */
void w5500_port_init(void)
{
    SPI1_Init();                                          /* hardware layer */
    printf("SPI1_Init done\r\n");

    reg_wizchip_cs_cbfunc(W5500_CS_Low, W5500_CS_High);   /* CS control */
    printf("CS control callback registered\r\n");
    reg_wizchip_spi_cbfunc(spi_rb, spi_wb);               /* SPI byte I/O */
    printf("SPI byte I/O callback registered\r\n");
    reg_wizchip_spiburst_cbfunc(spi_rbuf, spi_wbuf);      /* SPI burst I/O */
    printf("SPI burst I/O callback registered\r\n");
    reg_wizchip_cris_cbfunc(cris_enter, cris_exit);       /* critical section */
    printf("Critical section callback registered\r\n");

    W5500_Reset();                                        /* hardware reset */
    printf("W5500_Reset done\r\n");
}