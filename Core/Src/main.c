/* ============================================================
 * ETHERNET BRING-UP — NUCLEO-F411RE + W5500
 * PA1  STATUS LED — solid ON when W5500 responds + link up
 * ============================================================ */
#include <stdio.h>
#include "main.h"
#include "w5500_port.h"
#include "wizchip_conf.h"
#include "socket.h"

#define SOCK_TCP0   0
#define TCP_PORT    5000


void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);

UART_HandleTypeDef huart2;


int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);

	return ch;
}


int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_USART2_UART_Init();
    MX_GPIO_Init();

    printf("\r\n========================================\r\n");
    printf("  NUCLEO-F411RE + W5500 Ethernet Bring-Up\r\n");
    printf("========================================\r\n\r\n");

    /* ── W5500 init ── */
    printf("[INIT] Starting W5500 port init...\r\n");
    w5500_port_init();
    printf("[INIT] W5500 port init complete.\r\n");

    {
        uint8_t tx[8] = {2,2,2,2,2,2,2,2};
        uint8_t rx[8] = {2,2,2,2,2,2,2,2};
        printf("[INIT] Calling wizchip_init (TX/RX buffer sizes = 2KB each)...\r\n");
        wizchip_init(tx, rx);
        printf("[INIT] wizchip_init complete.\r\n");
    }

    /* ── Set static network info (AFTER wizchip_init) ── */
    wiz_NetInfo netInfo = {
        .mac  = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x33},
        .ip   = {10, 0, 0, 200},
        .sn   = {255, 255, 255, 0},
        .gw   = {10, 0, 0, 1},
        .dns  = {8, 8, 8, 8},
        .dhcp = NETINFO_STATIC
    };
    ctlnetwork(CN_SET_NETINFO, &netInfo);
    printf("[NET] Static IP: %d.%d.%d.%d / GW: %d.%d.%d.%d\r\n",
           netInfo.ip[0], netInfo.ip[1], netInfo.ip[2], netInfo.ip[3],
           netInfo.gw[0], netInfo.gw[1], netInfo.gw[2], netInfo.gw[3]);

    /* ── STATUS LED ON if W5500 chip responds ── */
    {
        uint8_t ver = getVERSIONR();
        printf("[W5500] Chip version register: 0x%02X ", ver);
        if (ver == 0x04)
        {
            printf("(OK - W5500 detected)\r\n");
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
        }
        else
        {
            printf("(FAIL - expected 0x04!)\r\n");
        }
    }

    /* ── Step 2: wait for PHY link up ── */
    printf("[PHY] Waiting for Ethernet link up...\r\n");
    while (wizphy_getphylink() != PHY_LINK_ON)
    {
        /* blocking wait — plug in Ethernet cable */
    }
    printf("[PHY] Ethernet link UP!\r\n");

    /* ── Step 3: open TCP socket 0, listen, echo server ── */
    uint8_t buf[64];

    printf("[TCP] Opening socket 0 on port %d...\r\n", TCP_PORT);
    socket(SOCK_TCP0, Sn_MR_TCP, TCP_PORT, 0x00);
    listen(SOCK_TCP0);
    printf("[TCP] Socket 0 listening on port %d\r\n", TCP_PORT);

    while (1)
    {
        uint8_t status = getSn_SR(SOCK_TCP0);

        if (status == SOCK_ESTABLISHED)
        {
            int32_t len = getSn_RX_RSR(SOCK_TCP0);
            if (len > 0)
            {
                if (len > (int32_t)sizeof(buf))
                    len = sizeof(buf);

                len = recv(SOCK_TCP0, buf, len);
                if (len > 0)
                {
                    printf("[TCP] Echoing %ld bytes\r\n", (long)len);
                    send(SOCK_TCP0, buf, len);   /* echo back */
                }
            }
        }
        else if (status == SOCK_CLOSE_WAIT)
        {
            printf("[TCP] Socket 0: CLOSE_WAIT — disconnecting\r\n");
            disconnect(SOCK_TCP0);
        }
        else if (status == SOCK_CLOSED)
        {
            printf("[TCP] Socket 0: CLOSED — reopening\r\n");
            socket(SOCK_TCP0, Sn_MR_TCP, TCP_PORT, 0x00);
            listen(SOCK_TCP0);
            printf("[TCP] Socket 0 listening on port %d\r\n", TCP_PORT);
        }
    }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }

    // Enable USART2 interrupt in NVIC
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA1 — STATUS LED, OFF initially */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin   = GPIO_PIN_1;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState            = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM            = 4;
    RCC_OscInitStruct.PLL.PLLN            = 96;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ            = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK
                                     | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1
                                     | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
        Error_Handler();
}

void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart2);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file; (void)line;
}
#endif
