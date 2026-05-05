/*
 * PT_LOGGER_L433_V1 — main.cpp
 * Zephyr 4.4.x  (USB_DEVICE_STACK_NEXT)
 *
 * Shell over USART1 (debug console).
 * USB CDC-ACM for application data (nanopb → COBS → USB).
 *
 * Shell commands:
 *   pt reboot   — cold reboot
 *   pt dfu      — jump to STM32 system bootloader (USB DFU)
 */

#include <inttypes.h>               /* PRIx32 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/w1.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/usb_device.h>
#include <cmsis_core.h>
#include <zephyr/init.h>
#include <soc.h>


LOG_MODULE_REGISTER(pt_logger, LOG_LEVEL_DBG);


// #define CDC_DEV DEVICE_DT_GET(DT_NODELABEL(cdc_acm_uart0))
// #define TEMP_SENSOR_DEV DEVICE_DT_GET(DT_NODELABEL(w1))

#define CRASH_MAGIC_NUMBER 0xDEADBEEF
#define REBOOT_REASON_DFU  0xDFDFDFDF

__noinit struct {
    uint32_t magic;
    uint32_t reset_reason;
} crash_data;

/* ============================================================
 * USB init / deinit
 * ========================================================== */

/* ============================================================
 * Peripheral initialization
 *=========================================================== */
const struct device *w1_dev = TEMP_SENSOR_DEV;


/* ============================================================
 * STM32L433 system bootloader jump
 *
 * Address from ST AN2606 — STM32L4 series system memory.
 * Vector table: [0] = initial MSP, [1] = reset handler PC.
 * ========================================================== */
#define STM32_BOOTLOADER_ADDR  0x1FFF0000UL

static int early_dfu_check(void)
{
    if (crash_data.magic == CRASH_MAGIC_NUMBER && crash_data.reset_reason == REBOOT_REASON_DFU) {
        
        crash_data.magic = 0;
        crash_data.reset_reason = 0;

        __disable_irq();

        MPU->CTRL = 0;
        __DSB();
        __ISB();

        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;

        RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
        volatile uint32_t tmpreg = RCC->APB2ENR; 
        (void)tmpreg;
        
        SYSCFG->MEMRMP = 0x01;
        __DSB();
        __ISB();

        SCB->VTOR = STM32_BOOTLOADER_ADDR;
        __DSB();
        __ISB();

        uint32_t bootloader_msp = *((volatile uint32_t *)STM32_BOOTLOADER_ADDR);
        uint32_t bootloader_pc  = *((volatile uint32_t *)(STM32_BOOTLOADER_ADDR + 4));
        
        __set_MSP(bootloader_msp);
        ((void (*)(void))bootloader_pc)();

        CODE_UNREACHABLE;
    }

    return 0;
}
SYS_INIT(early_dfu_check, PRE_KERNEL_1, 0);


/* ============================================================
 * Shell command handlers
 * ========================================================== */
static int cmd_reboot(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "Rebooting...");
    k_msleep(50);
    sys_reboot(SYS_REBOOT_COLD);
    return 0;
}

static int cmd_dfu(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "Jumping to DFU bootloader...");
    shell_print(sh, "Flash with: dfu-util -a 0 -s 0x08000000:leave -D firmware.bin");

    k_msleep(50); 


    crash_data.magic = CRASH_MAGIC_NUMBER;
    crash_data.reset_reason = REBOOT_REASON_DFU;

    sys_reboot(SYS_REBOOT_COLD);
    return 0;
}

static int cmd_temp(const struct shell *sh, size_t argc, char **argv){
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    k_sleep(K_SECONDS(2));

    return 0;
}

static int cmd_exit(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(sh, "Stopping Zephyr Shell. Terminal is now detached.");
    k_msleep(50);


    shell_stop(sh);

   
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(pt_cmds,
    SHELL_CMD_ARG(reboot, NULL,
                  "Cold reboot the device.",
                  cmd_reboot, 1, 0),
    SHELL_CMD_ARG(dfu, NULL,
                  "Jump to STM32 system bootloader (USB DFU).\n"
                  "Flash with: dfu-util -a 0 -s 0x08000000:leave -D firmware.bin",
                  cmd_dfu, 1, 0),
    SHELL_CMD_ARG(temp, NULL,
                  "Read temperature from MAX31888 sensor.",
                  cmd_temp, 1, 0),
    SHELL_CMD_ARG(exit, NULL, "Detach shell and free UART/USB port",
                  cmd_exit, 1, 0),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(pt, &pt_cmds, "PT Logger commands.", NULL);

void w1_search_callback(struct w1_rom rom, void *user_data)
{
	LOG_INF("Device found; family: 0x%02x, serial: 0x%016llx", rom.family, w1_rom_to_uint64(&rom));
}

/* ============================================================
 * main
 * ========================================================== */
int main(void)
{
    LOG_INF("PT Logger starting — Zephyr %s", KERNEL_VERSION_STRING);
    LOG_INF("Shell on USART1. Type 'pt <tab>' for commands.");

    k_sleep(K_SECONDS(1)); 

 
    while (true) {
        k_sleep(K_SECONDS(1));
    }
    return 0;
}