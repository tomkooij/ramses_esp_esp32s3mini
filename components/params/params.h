/*
 * ramses_esp
 * params.h
 *
 *  Created on: 2 Jan 2024
 *      Author: Peter Price
 *
 *  NVS Management
 */

#ifndef _PARAMS_H_
#define _PARAMS_H_

extern int nvs_reset(void);
extern void nvs_start(void);

extern void params_init(void);

#endif // _PARAMS_H_
