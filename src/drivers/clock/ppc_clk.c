/**
 * @file
 * @brief PowerPC Microprocessor Family clock device driver
 *
 * @date 06.11.12
 * @author Ilia Vaprol
 */

#include <types.h>
#include <hal/clock.h>
#include <hal/system.h>
#include <kernel/irq.h>
#include <kernel/time/clock_source.h>
#include <asm/psr.h>

#include <embox/unit.h>

EMBOX_UNIT_INIT(ppc_clk_init);

#define PPCCLK_IRQ  10
#define PPCCLK_FREQ SYS_CLOCK
#define PPCCLK_DECR 400

static irq_return_t clock_handler(unsigned int irq_nr, void *data) {
	__set_tsr(__get_tsr() & ~TSR_DIS);
	clock_tick_handler(irq_nr, data);
	return IRQ_HANDLED;
}

static int ppc_clk_config(struct time_dev_conf *conf) {
	__set_dec(PPCCLK_DECR);
	__set_decar(PPCCLK_DECR);
    __set_tcr(TCR_DIE | TCR_ARE);
	return 0;
}

static cycle_t ppc_clk_read(void) {
	uint32_t l, u, tmp;
	tmp = __get_tbu();
	do {
		u = tmp;
		l = __get_tbl();
		tmp = __get_tbu();
	} while (tmp != u);
	return ((uint64_t)u << 32) | l;
}

static struct time_event_device ppc_clk_event = {
	.config = ppc_clk_config,
	.resolution = PPCCLK_FREQ / PPCCLK_DECR,
	.irq_nr = PPCCLK_IRQ,
};

static struct time_counter_device ppc_clk_counter = {
	.read = ppc_clk_read,
	.resolution = PPCCLK_FREQ,
};

static struct clock_source ppc_clk_clock_source = {
	.name = "ppc_clk",
	.event_device = &ppc_clk_event,
	.counter_device = &ppc_clk_counter,
	.read = clock_source_read,
};

static int ppc_clk_init(void) {
	clock_source_register(&ppc_clk_clock_source);
	return irq_attach(PPCCLK_IRQ, clock_handler, 0, NULL, "ppc_clk");
}

