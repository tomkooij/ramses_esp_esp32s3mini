/********************************************************************
 * ramses_esp
 * ramses_buttons.c
 *
 * (C) 2025 Peter Price
 *
 * Monitor buttons
 *
 */
static const char *TAG = "BUTTON";
#include "esp_log.h"

#include "driver/gpio.h"

#include "ramses_buttons.h"

/**********************************************************************************
 * Button context data
 */
struct button_data {
  BaseType_t coreID;
  TaskHandle_t   task;
  QueueHandle_t  queue;
};

static struct button_data *button_context( void ) {
  static struct button_data buttons;

  static struct button_data *ctxt = NULL;
  if( !ctxt ){
    ctxt = &buttons;
    // Initialisation?
  }

  return ctxt;
}

/**********************************************************************************
 * Button status data
 */

struct callback {
  button_cb cb;
  struct callback *next;
};

struct button_status {
  char const * const name;
  uint32_t gpio;
  struct callback *cb;
} ;

static struct button_status *lookup_status( enum buttons button ) {
#define _BUTTON(_e,_t,_i) ,{ .name=_t, .gpio=_i }
  static struct button_status Status[BUTTON_MAX] = { { .name="NONE", .gpio=-1 } _BUTTON_LIST };
#undef _BUTTON

  struct button_status *status = NULL;
  if( button>BUTTON_NONE && button<BUTTON_MAX )
    status = &Status[button];

  return status;
}

static struct button_status *find_status( uint32_t gpio ) {
  enum buttons button;
  for( button=BUTTON_NONE+1 ; button<BUTTON_MAX ; button++ ) {
    struct button_status *status = lookup_status( button );
	if( status && status->gpio==gpio )
	  return status;
  }

  return NULL;
}

/**************************************************************************************/
#define GPIO_PIN_SEL(_p)  ( 1ULL<<(_p) )

static void buttons_config( struct button_data *ctxt ) {
  gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_ANYEDGE,
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = 0,
    .pull_down_en = 0,
    .pull_up_en = 1
  };

  enum buttons button;
  for( button=BUTTON_NONE+1 ; button<BUTTON_MAX ; button++ ) {
    struct button_status *status = lookup_status( button );
    if( status ) {
      io_conf.pin_bit_mask = GPIO_PIN_SEL( status->gpio );
      gpio_config(&io_conf);
    }
  }
}

/**********************************************************************************
 * Button events
 */

static void make_event( struct button_event *event, uint8_t gpio ) {
  if( event ) {
    event->ticks = xTaskGetTickCountFromISR();
    event->gpio  = gpio;
    event->level = gpio_get_level( event->gpio );
  }
}

static void process_event( struct button_event *event ) {
  if( event ) {
    struct button_status *status = find_status( event->gpio );
    if( status ) {
      struct callback *cb;
      for( cb=status->cb ; cb!=NULL ; cb=cb->next ) {
        if(cb->cb)
          (cb->cb)( event );
      }
    }
  }
}

/**********************************************************************************
 * Button Task and ISR
 */

static void IRAM_ATTR button_isr( void* arg ) {
  struct button_data *ctxt = button_context();

  if( ctxt && ctxt->queue ) {
    struct button_event event;
    make_event( &event, (uint32_t)arg );
    xQueueSendFromISR( ctxt->queue, &event, NULL );
  }
}


static void Buttons( void *param ) {
  struct button_data *ctxt = param;

  ESP_LOGI( TAG, "Task Started");

  ctxt->queue = xQueueCreate( 10, sizeof( struct button_event ) );
  buttons_config( ctxt );

  do {
    struct button_event event;
    BaseType_t res = xQueueReceive( ctxt->queue, &event, portTICK_PERIOD_MS );
    if( res ) {
      process_event( &event );
    }
  } while(1);
}

/**************************************************************************************
 * External API
 */

void button_register( enum buttons button, button_cb cb ) {
  struct button_status *status = lookup_status( button );
  if( status ) {
    struct callback *callback = malloc( sizeof(*callback) );
	if( callback ) {
      callback->cb = cb;
      callback->next = status->cb;
      if( !status->cb ) // First use
    	  gpio_isr_handler_add( status->gpio, button_isr, (void *)status->gpio );
      status->cb = callback;
	}
  }
}

void ramses_buttons_init( BaseType_t coreID ) {
  struct button_data *ctxt = button_context();

  ctxt->coreID = coreID;

  xTaskCreatePinnedToCore( Buttons,  "Buttons",  4096, ctxt, 10, &ctxt->task, ctxt->coreID );
}
