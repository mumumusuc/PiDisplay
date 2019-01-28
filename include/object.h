//
// Created by mumumusuc on 19-1-25.
//

#ifndef PI_DISPLAY_BASE_H
#define PI_DISPLAY_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "common.h"

#define object(self)                              ((self)->obj)

#define superclass(self, class)                   container_of(object_get_superclass(self->obj),class,obj)
#define subclass(self, class)                     container_of(object_get_subclass(self->obj),class,obj)

#define link(self, name, destructor)              object_link(&((self)->obj),NULL,(self),(destructor),(name))
#define link2(self, super, name, destructor)      object_link(&((self)->obj),&((super)->obj),(self),(destructor),(name))

#define find_subclass(self, class, name)          ({Object* target = object_find_subclass((self)->obj,(name));target?((class*)object_get_host(target)):NULL;})
#define find_superclass(self, class, name)        ({Object* target = object_find_superclass((self)->obj,(name));target?((class*)object_get_host(target)):NULL;})

typedef void (*Destructor)(void *);

typedef struct _Object Object;

const char *object_get_name(Object *);

const void *object_get_host(Object *);

void object_link(Object **, Object **, void *, Destructor, const char *);

void object_delete(Object *);

void delete(Object *);

const Object **object_get_superclass(Object *obj);

const Object **object_get_subclass(Object *obj);

Object *object_find_superclass(Object *, const char *);

Object *object_find_subclass(Object *, const char *);


#ifdef __cplusplus
}
#endif

#endif //PI_DISPLAY_BASE_H
