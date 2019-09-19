#ifndef RB_H
#define RB_H

#include <stdbool.h>

#define comp_t(name) int (*name)(const void*,const void*)

enum color { BLACK, RED };

typedef struct rbnode {
  struct rbnode* p;
  struct rbnode* left;
  struct rbnode* right;
  void* key;
  enum color c;
} rbnode;

typedef struct rbtree {
  rbnode* root;
  rbnode* nil;
  comp_t(compar);
} rbtree;

void init_rbtree(rbtree* t, comp_t(compar));
void destroy_rbtree(rbtree* t, void (*destroy)(void*));
void destroy_rbsubtree(rbtree* t, rbnode* n, void (*destroy)(void*));

static inline bool rb_is_empty(rbtree* t) { return t->root == t->nil; }

void* rbfind_key(rbtree* t, void* key);
rbnode* rbinsert_key(rbtree* t, void* key);
void rb_delete_key(rbtree* t, void (*destroy)(void*), void* key);

rbnode* rbsearch(rbtree* t, void* key);
rbnode* rbinsert(rbtree* t, rbnode* z);
void rbdelete(rbtree* t, rbnode* z);

rbnode* make_rbnode(rbtree* t, enum color c, void* key);
void destroy_rbnode(rbnode* n, void (*destroy)(void*));

#endif
