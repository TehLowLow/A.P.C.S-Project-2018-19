#include <stdlib.h>
#include <assert.h>

#include "slist.h"

void init_slist(slist* sl, int (*compar)(const void*,const void*))
{
  assert(sl && compar);
  sl->first = sl->last = NULL;
  sl->compar = compar;
}

snode* make_snode(void* data)
{
  snode* new_snode = (snode*) malloc(sizeof(snode));
  new_snode->next = new_snode->prev = NULL;
  new_snode->data = data;
  return new_snode;
}

void destroy_slist_nodes(slist* sl, void (*destroy)(void*))
{
  snode* i = sl->first;
  snode* next;
  while (i) {
    next = i->next;
    if (destroy)
      destroy(i->data);
    free(i);
    i = next;
  }
  sl->first = sl->last = NULL;
}

snode* slist_insert(slist* sl, void* data)
{
  assert(sl && data);
  snode* i = sl->first;
  int cmp;
  while (i && (cmp = sl->compar(i->data, data)) < 0) {
    i = i->next;
  }
  if (i && cmp == 0) {
    // Already present.
    return NULL;
  }
  assert(!i || cmp > 0);
  snode* new_snode = make_snode(data);
  slist_insert_before(sl, i, new_snode);
  return new_snode;
}

void slist_insert_before(slist* sl, snode* next, snode* new_snode)
{
  assert(sl && new_snode);
  if (!sl->first) {
    // List is empty.
    assert(!next && !sl->last);
    new_snode->next = new_snode->prev = NULL;
    sl->first = sl->last = new_snode;
    return;
  }
  assert(sl->first && sl->last);
  if (!next) {
    // End of list.
    new_snode->next = NULL;
    new_snode->prev = sl->last;
    sl->last->next = new_snode;
    sl->last = new_snode;
    return;
  }
  assert(next);
  new_snode->next = next;
  if (next->prev) {
    new_snode->prev = next->prev;
    next->prev->next = new_snode;
  } else {
    // Beginning of the list.
    assert(sl->first == next);
    new_snode->prev = NULL;
    sl->first = new_snode;
  }
  next->prev = new_snode;
}

void slist_delete(slist* sl, snode* sn, void (*destroy)(void*))
{
  if (destroy)
    destroy(sn->data);

  slist_remove(sl, sn);
  free(sn);
}

void slist_remove(slist* sl, snode* sn)
{
  assert(sl && sn);
  if (sn->prev) {
    sn->prev->next = sn->next;
    if (sn->next)
      sn->next->prev = sn->prev;
    else {
      // End of the list.
      assert(sl->last == sn);
      sl->last = sn->prev;
    }
  } else {
    // Beginning of the list.
    assert(sl->first == sn);
    sl->first = sn->next;
    if (sn->next)
      sn->next->prev = NULL;
    else {
      // sn was the only element, sl is now empty.
      assert(sl->last == sn);
      sl->last = NULL;
    }
  }
  sn->next = sn->prev = NULL;
}

snode* slist_find(slist* sl, void* data)
{
  assert(sl && data);
  snode* i = sl->first;
  int cmp;
  while (i && (cmp = sl->compar(i->data, data)) < 0) {
    i = i->next;
  }
  if (i && cmp == 0)
    return i;
  assert(!i || cmp > 0);
  return NULL;
}
