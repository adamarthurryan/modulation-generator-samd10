#ifndef SYSTICK_H_
#define SYSTICK_H_

static inline void systick_init();

static inline void systick_init() {
	SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;
}

static inline uint32_t systick_read() {
	return SysTick->VAL;
}


#endif /* SYSTICK_H_ */