/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _VERSION_H_
#define _VERSION_H_

#define VERSION_MAJOR 3 /* eg API breaking */
#define VERSION_MDIUM 0 /* Feature  update */
#define VERSION_MINOR 0 /* Bufix /  Hotfix */


#define _STRINGIFY(x) #x
#define STRINGIFY(x)  _STRINGIFY(x)

#define VERSION STRINGIFY(VERSION_MAJOR)"."STRINGIFY(VERSION_MDIUM)"."STRINGIFY(VERSION_MINOR)

#endif /* #ifndef _VERSION_H_ */
