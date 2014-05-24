/*
 * panic.h
 *
 *  Created on: 16/04/2014
 *      Author: cliu712
 */

#ifndef PANIC_H_
#define PANIC_H_

#include <stdio.h>

#define panic(s)	do {			\
	printf("%s:%s():%d  %s\n", __FILE__, __func__,	\
	       __LINE__, s);				\
	for (;;) ;					\
	} while (0)

#endif /* PANIC_H_ */
