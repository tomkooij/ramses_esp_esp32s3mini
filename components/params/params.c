/*
 * ramses_esp
 * params.c
 *
 *  Created on: 2 Jan 2024
 *    Author: Peter Price
 *
 *  NVS Management
 */
#include <string.h>

static const char * TAG = "NVS";
#include "esp_log.h"
#include "esp_err.h"

#include "esp_console.h"

#include "nvs_flash.h"

#include "params.h"

/*********************************************************
 * Helper Functions
 */
static esp_console_cmd_func_t cmd_lookup( esp_console_cmd_t const *list, char *command ) {
  while( list->func != NULL ) {
    if( !strcmp( command, list->command ) )
      break;
    list++;
  }

  return list->func;
}

static void cmd_help( esp_console_cmd_t const *list, char *prefix ) {
  while( list->func ) {
	printf("%s %10s %s\n",prefix,list->command,list->help );
	list++;
  }
}

/*********************************************************
 * ERASE command
 */
static int nvs_erase_cmd( int argc, char **argv ) {
  esp_err_t err = ESP_OK ;

  if( argc!=2 ) {
	ESP_LOGE( TAG, "nvs erase <namespace>|* ");
	err = ESP_FAIL;
  } else if( !strcmp( argv[1], "*") ) {
	nvs_reset();  // reset NVS partition
  } else {
    nvs_handle_t handle;
    err = nvs_open( argv[1], NVS_READWRITE, &handle);
    if( err == ESP_OK ) {
      err = nvs_erase_all(handle);
      nvs_commit(handle);
      nvs_close(handle);
    }
  }

  return err;
}

/*********************************************************
 * DUMP command
 */
static int nvs_dump_cmd( int argc, char **argv ) {
  esp_err_t res = ESP_OK ;
  nvs_iterator_t it = NULL;

  res = nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL, NVS_TYPE_ANY, &it);
  while( res==ESP_OK ) {
	char value[32], *pValue=value;
	nvs_entry_info_t info;
	nvs_handle_t h;
    nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
    nvs_open( info.namespace_name, NVS_READONLY, &h );
    switch( info.type ) {
    case NVS_TYPE_U8:	{uint8_t  v; nvs_get_u8(h,info.key,&v); sprintf(value,"%u",v);  } break;
    case NVS_TYPE_I8:	{ int8_t  v; nvs_get_i8(h,info.key,&v); sprintf(value,"%d",v);  } break;
    case NVS_TYPE_U16:	{uint16_t v; nvs_get_u16(h,info.key,&v);sprintf(value,"%u",v);  } break;
    case NVS_TYPE_I16:	{ int16_t v; nvs_get_i16(h,info.key,&v);sprintf(value,"%d",v);  } break;
    case NVS_TYPE_U32:	{uint32_t v; nvs_get_u32(h,info.key,&v);sprintf(value,"%lu",v); } break;
    case NVS_TYPE_I32:	{ int32_t v; nvs_get_i32(h,info.key,&v);sprintf(value,"%ld",v); } break;
    case NVS_TYPE_U64:	{uint64_t v; nvs_get_u64(h,info.key,&v);sprintf(value,"%llu",v);} break;
    case NVS_TYPE_I64:	{ int64_t v; nvs_get_i64(h,info.key,&v);sprintf(value,"%lld",v);} break;
    case NVS_TYPE_STR:
    case NVS_TYPE_BLOB:
      size_t len=32;
      nvs_get_str( h,info.key,value,&len);
      value[31]= '\0';
      break;
    default:
      pValue = "unknown";
      break;
    }
    nvs_close(h);
    printf("[%s]: key '%s', type '%d' %s\n", info.namespace_name, info.key, info.type, pValue );

    res = nvs_entry_next(&it);
  }
  nvs_release_iterator(it);

  return 0;
}

/*********************************************************
 * Top Level command
 */
enum nvs_cmd_list {
  NVS_CMD_ERASE,
  NVS_CMD_DUMP,
  NVS_CMD_MAX
};

static esp_console_cmd_t const cmds[NVS_CMD_MAX+1] = {
  [NVS_CMD_ERASE] = {
    .command = "erase",
    .help = "Erase NVS <namespace> or *",
    .hint = NULL,
    .func = &nvs_erase_cmd,
  },
  [NVS_CMD_DUMP] = {
    .command = "dump",
    .help = "Dump data from default NVS partition",
    .hint = NULL,
    .func = &nvs_dump_cmd,
  },
  // List termination
  [NVS_CMD_MAX] = {
    .command = NULL,
    .help = NULL,
    .hint = NULL,
    .func = NULL,
  },
};

static int nvs_cmd( int argc, char **argv ) {
  if( argc==1 ) {
	cmd_help( cmds, "nvs" );
  } else {
	esp_console_cmd_func_t func = cmd_lookup( cmds, argv[1] );
	if( func )
      return ( func )( --argc, ++argv );
	else
      cmd_help( cmds, "nvs" );
  }

  return 0;
}

static void nvs_register(void) {
  const esp_console_cmd_t nvs = {
    .command = "nvs",
    .help = "NVS commands, enter 'nvs' for list",
    .hint = NULL,
    .func = &nvs_cmd,
  };

  ESP_ERROR_CHECK( esp_console_cmd_register(&nvs) );
}

int nvs_reset(void) {
  ESP_ERROR_CHECK( nvs_flash_erase() );
  return nvs_flash_init();
}

void nvs_start(void) {
  esp_err_t ret = nvs_flash_init();
  if( ret==ESP_ERR_NVS_NO_FREE_PAGES || ret==ESP_ERR_NVS_NEW_VERSION_FOUND )
	ret = nvs_reset();

  ESP_ERROR_CHECK(ret);
}

void params_init(void) {
  esp_log_level_set(TAG, CONFIG_NVS_LOG_LEVEL );

  nvs_register();
}
