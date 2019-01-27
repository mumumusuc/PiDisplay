//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_BASE_H
#define PI_DISPLAY_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define superclass(self, class)                 container_of(get_superclass(self->obj),class,obj)
#define subclass(self, class)                   container_of(get_subclass(self->obj),class,obj)
#define link(self, name, destructor)            object_link(&((self)->obj),NULL,(self),(destructor),(name))
#define link2(self, super, name, destructor)    object_link(&((self)->obj),&((super)->obj),(self),(destructor),(name))

typedef void (*Destructor)(void *);

typedef struct _Object Object;

Object **const get_superclass(Object *obj);

Object **const get_subclass(Object *obj);

void object_print_name(Object *);

void object_link(Object **, Object **, void *, Destructor, const char *);

void object_delete(Object *);

void delete(Object *);

#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_BASE_H
