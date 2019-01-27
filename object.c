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

inline Object **const get_superclass(Object *obj) {
    return obj->super_class;
}

inline Object **const get_subclass(Object *obj) {
    return obj->sub_class;
}

inline void object_print_name(Object *obj) {
    LOG("Object is : %s", obj->name);
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