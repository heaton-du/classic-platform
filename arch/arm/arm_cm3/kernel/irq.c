/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/

#include "internal.h"
#include "task_i.h"
#include "hooks.h"
#include "stm32f10x.h"
#include "irq.h"
#include "core_cm3.h"

extern void *Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];

/**
 * Init NVIC vector. We do not use subpriority
 *
 * @param vector	The IRQ number
 * @param prio      NVIC priority, 0-15, 0-high prio
 */
static void NVIC_InitVector(IRQn_Type vector, uint32_t prio)
{
	// Set prio
	NVIC_SetPriority(vector,prio);

	// Enable
    NVIC->ISER[vector >> 5] = (uint32_t)1 << (vector & (uint8_t)0x1F);
}

/*
* PRIGROUP[2:0] 	Group prios 	Sub prios
* 0b011 			16 				None
* 0b100 			8 				2
* 0b101 			4 				4
* 0b110 			2 				8
* 0b111 			None 			16
*/
void Irq_Init( void ) {
	NVIC_SetPriorityGrouping(7); // No preemtion priority TODO is this correct
	NVIC_SetPriority(SVCall_IRQn, 15); // Set lowest prio
	NVIC_SetPriority(PendSV_IRQn, 15); // Set lowest prio

	/* Stop counters and watchdogs when halting in debug */
	DBGMCU->CR |= 0x00ffffff00;
}

void Irq_EOI( void ) {
	/* Note!
	 * This is not applicable on the Cortex-M3 since we
	 * can't terminate the interrupt request without popping
	 * back registers..have to be solved in the context switches
	 * themselves.
	 */
}

#define ICSR_VECTACTIVE		0x1ff

/**
 * Get Active ISR number field.
 * You can subtract 16 from the VECTACTIVE field to index into the Interrupt
 * Clear/Set Enable, Interrupt Clear Pending/SetPending and Interrupt Priority
 * Registers. INTISR[0] has vector number 16.
 *
 */
static uint32_t NVIC_GetActiveVector( void) {
	return (SCB->ICSR &  ICSR_VECTACTIVE);
}


/**
 *
 * @param stack_p Ptr to the current stack.
 *
 * The stack holds C, NVGPR, VGPR and the EXC frame.
 *
 */
void *Irq_Entry( void *stack_p ){
	uint32_t vector = 0;

	Irq_Disable();

	/* 0. Set the default handler here....
	 * 1. Grab the vector from the interrupt controller
	 *    INT_CTRL_ST[VECTACTIVE]
	 * 2. Irq_VectorTable[vector] is odd -> ISR1
	 *    Irq_VectorTable[vector] is even-> ISR2
	 */


	vector = NVIC_GetActiveVector();

	Os_Isr_cm3((void *)Irq_VectorTable[vector]);
	Irq_Enable();

	return stack_p;
}

/**
 * Attach an ISR type 1 to the interrupt controller.
 *
 * @param entry
 * @param int_ctrl
 * @param vector
 * @param prio
 */
void Irq_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector, uint8_t prio) {

	// TODO: Use NVIC_InitVector(vector, osPrioToCpuPio(pcb->prio)); here
}

/**
 * NVIC prio have priority 0-15, 0-highest priority.
 * Autosar does it the other way around, 0-Lowest priority
 *
 * Autosar    NVIC
 *   31        0
 *   30        0
 *   ..
 *   0         15
 *   0         15
 * @param prio
 * @return
 */
static inline int osPrioToCpuPio( uint8_t prio ) {
	assert(prio<32);
	prio = 31 - prio;
	return (prio>>1);
}

/**
 * Attach a ISR type 2 to the interrupt controller.
 *
 * @param tid
 * @param int_ctrl
 * @param vector
 */
void Irq_AttachIsr2(TaskType tid,void *int_ctrl,IrqType vector ) {
	OsPcbType *pcb;

	pcb = os_find_task(tid);
	Irq_VectorTable[vector+16] = (void *)pcb;

	NVIC_InitVector(vector, osPrioToCpuPio(pcb->prio));
}


/**
 * Generates a soft interrupt, ie sets pending bit.
 * This could also be implemented using ISPR regs.
 *
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector ) {

	NVIC->STIR = (vector);
}

/**
 * Get the current priority from the interrupt controller.
 * @param cpu
 * @return
 */
uint8_t Irq_GetCurrentPriority( Cpu_t cpu) {

	uint8_t prio = 0;

	// SCB_ICSR contains the active vector
	return prio;
}

typedef struct {
	uint32_t dummy;
} exc_stack_t;


