/*
 * Copyright (C) 2014 Markus Ippy
 *
 * Digital Gauges for Apexi Power FC for RX7 on Raspberry Pi 
 * 
 * 
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 * 
 * No warranty is made or implied. You use this program at your own risk.
 */

/*! 
  \file raspexi/helpers.h
  \brief Raspexi Viewer helpers and stub functions
  \author Suriyan Laohaprapanon
 */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __HELPERS_H__
#define __HELPERS_H__

#include <glib.h>

void init(void);
void mem_dealloc(void);

#endif
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
