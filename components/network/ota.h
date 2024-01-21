/********************************************************************
 * ramses_esp
 * ota.h
 *
 * (C) 2023 Peter Price
 *
 * Over The Air upgrade
 *
 */
#ifndef _OTA_H_
#define _OTA_H_

extern void ota_set_url( const char * url );
extern void ota_set_version( const char * version );
extern void ota_set_filename( const char * filename );
extern void ota_start(void);

extern void ota_init( BaseType_t coreID );

#endif // _OTA_H_
