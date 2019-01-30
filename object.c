//
// Created by mumumusuc on 19-1-25.
//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object.h"
#include "common.h"

#undef LOG_TAG
#define LOG_TAG "OBJECT"

struct _Object {
    Object **super_class;
    Object **sub_class;
    void *this;
    char name[16];
    Destructor destructor;
};

inline const Object **object_get_superclass(Object *obj) {
    return (const Object **) (obj->super_class);
}

inline const Object **object_get_subclass(Object *obj) {
    return (const Object **) (obj->sub_class);
}

inline const char *object_get_name(Object *obj) {
    return obj->name;
}

inline const void *object_get_host(Object *obj) {
    return obj->this;
}

inline void object_link(Object **self, Object **super, void *this, Destructor destructor, const char *name) {
    (*self) = (Object *) malloc(sizeof(Object));
    (*self)->super_class = NULL;
    (*self)->sub_class = NULL;
    (*self)->this = this;
    (*self)->destructor = destructor;
    strcpy((*self)->name, name);
    if (super) {
        //link super
        (*self)->super_class = super;
        //link sub
        (*super)->sub_class = self;
    }
}

inline void object_delete(Object *obj) {
    free(obj);
    obj = NULL;
}

inline static void delete_self(Object *obj) {
    LOG("Destructing class %s ...", obj->name);
    Destructor destructor = obj->destructor;
    void *this = obj->this;
    destructor(this);
}

inline static void delete_next(Object **obj) {
    if (!obj) return;
    Object **super = (*obj)->super_class;
    delete_self(*obj);
    delete_next(super);
}

inline void delete(Object *obj) {
    if (!obj) return;
    Object **sub = obj->sub_class;
    if (sub) {
        delete(*sub);
        return;
    }
    delete_next(&obj);
}

static inline Object *find_myself(Object *self, const char *name) {
    if (!self || !name) return NULL;
    const char *my_name = object_get_name(self);
    // LOG("strcmp : %s <-> %s", my_name, name);
    if (!strcmp(my_name, name)) {
        // LOG("find class %s !", name);
        return self;
    }
    return NULL;
}

static Object *find_class(Object *self, const char *name) {
    Object *myself = find_myself(self, name);
    if (myself) return myself;
    Object *pre = object_find_superclass(self, name);
    if (pre) return pre;
    Object *next = object_find_subclass(self, name);
    if (next) return next;
    return NULL;
}

inline Object *object_find_superclass(Object *self, const char *name) {
    LOG("%s", __func__);
    Object **pre = self->super_class;
    if (!pre) return NULL;
    Object *who = find_myself(*pre, name);
    if (who) return who;
    return object_find_superclass(*pre, name);
}

inline Object *object_find_subclass(Object *self, const char *name) {
    LOG("%s", __func__);
    Object **next = self->sub_class;
    if (!next) return NULL;
    Object *who = find_myself(*next, name);
    if (who) return who;
    return object_find_subclass(*next, name);
}
