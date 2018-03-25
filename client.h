#ifndef __CLIENT__
#define __CLIENT__

#include <stdbool.h>

/* types */
typedef struct {

} client_t;

/* API */
bool client_create(client_t *c);
void client_destroy(client_t *c);
bool client_send(client_t *c, const void *data);


#endif
