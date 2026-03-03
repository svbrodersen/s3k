#include "s3k.h"
#include <stdio.h>

extern char __uart_base[];

void mem_init(s3k_word_t mon_idx, s3k_word_t idx, s3k_word_t slot, s3k_word_t cfree, s3k_word_t perm, s3k_word_t base,
	      s3k_word_t size)
{
	idx = s3k_mon_mem_derive(mon_idx, idx, cfree, perm, base, size);
	if (idx < 0) {
		printf("Failed to derive memory capability %lx\n", base);
		return;
	}

	s3k_word_t addr = s3k_pmp_napot_encode(base, size);
	int err = s3k_mon_mem_pmp_set(mon_idx, idx, slot, perm, addr);
	if (err < 0) {
		printf("Failed to set PMP for derived memory %lx, err=%d\n", base, err);
		return;
	}
}

void app2_init(void)
{
	int mon_idx = 8;   // Monitor index for PID 2
	int ram_idx = 0;   // RAM index (PID 1 owns RAM cap at index 0)
	int tsl_idx = 0;   // TSL index (PID 1 owns TSL cap at index 0 for Hart 0)
	int uart_idx = 16; // UART index (PID 1 owns UART cap at index 16)

	// RAM configuration for App2
	// App1 is at 0x80000000 (64KB)
	// App2 is at 0x80020000 (64KB)
	s3k_word_t ram_base = 0x80020000;
	s3k_word_t ram_size = 0x10000; // 64KB
	s3k_word_t ram_perm = S3K_MEM_PERM_RWX;
	s3k_word_t ram_fuel = 1;
	s3k_word_t ram_slot = 1;
	
	// App1 derives memory for App2 from its own RAM capability (index 0)
	mem_init(mon_idx, ram_idx, ram_slot, ram_fuel, ram_perm, ram_base, ram_size);

	// UART configuration for App2
	s3k_word_t uart_base = (s3k_word_t)__uart_base;
	s3k_word_t uart_size = 0x20;
	s3k_word_t uart_perm = S3K_MEM_PERM_RW;
	s3k_word_t uart_fuel = 1;
	s3k_word_t uart_slot = 2;

	// App1 derives UART for App2 from its own UART capability (index 16)
	mem_init(mon_idx, uart_idx, uart_slot, uart_fuel, uart_perm, uart_base, uart_size);

	// TSL configuration for App2
	// We want to alternate slots on Hart 0.
	// Hart 0 has 32 slots.
	// We'll give App2 every other slot, starting from the end.
	for (int i = 0; i < 15; i++) {
		// Derive 1 slot for App2 (PID 2)
		s3k_mon_tsl_derive(mon_idx, tsl_idx, 1, true, 1);
		// Derive 1 slot for App1 (PID 1) - this just splits the remaining range
		s3k_tsl_derive(tsl_idx, 1, true, 1);
	}
	// Last slot for App2
	s3k_mon_tsl_derive(mon_idx, tsl_idx, 1, true, 1);
	// The original tsl_idx (0) now has 1 slot left for App1 (PID 1).

	// Set App2 PC
	if (s3k_mon_reg_set(mon_idx, S3K_REG_PC, ram_base) != 0) {
		printf("Failed to set program counter for App2\n");
		return;
	}
	
	// Resume App2
	s3k_mon_resume(mon_idx);
}
