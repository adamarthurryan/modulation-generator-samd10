/*
 * q16d15.h
 *
 * Created: 1/22/2016 9:44:40 AM
 *  Author: Adam
 */ 


#ifndef Q16D15_H_
#define Q16D15_H_

typedef int32_t q16d15_t;

#define WHOLE_Q16D15(q) (q>>15)
#define FRACT_Q16D15(q) (q & ((1<<15)-1))

#define Q16D15_1 ((1<<15) - 1)
#define Q16D15_1_HALF (1<<14)


#endif /* Q16D15_H_ */