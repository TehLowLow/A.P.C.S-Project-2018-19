#ifndef SLIST_H
#define SLIST_H

typedef struct snode {
  struct snode* next;
  struct snode* prev;
  void* data;
} snode;

typedef struct slist {
  snode* first;
  snode* last;
  int (*compar)(const void*, const void*);
} slist;

void init_slist(slist* sl, int (*compar)(const void*,const void*));
snode* make_snode(void* data);
void destroy_slist_nodes(slist* sl, void (*destroy)(void*));

snode* slist_insert(slist* sl, void* data);
void slist_insert_before(slist* sl, snode* next, snode* new_snode);
void slist_delete(slist* sl, snode* sn, void (*destroy)(void*));
void slist_remove(slist* sl, snode* sn);
snode* slist_find(slist* sl, void* data);

#endif
