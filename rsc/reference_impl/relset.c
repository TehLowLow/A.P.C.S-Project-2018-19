#include "relset.h"

#include <string.h>
#include <stdio.h>

relnode* rellist_find(rellist* rl, const char* name)
{
  relnode* rn = RELLIST_FIRST(rl);
  int cmp;
  while (rn && (cmp = strcmp(GET_RELATION(rn)->name, name)) < 0) {
    rn = RELLIST_NEXT(rn);
  }
  if (rn && cmp == 0)
    return rn;
  else
    return NULL;
}

int compar_rel(const void* x, const void* y)
{
  return strcmp(((const relation*) x)->name, ((const relation*) y)->name);
}

void destroy_relation_void(void* r)
{
  destroy_relation((relation*) r);
}

#ifndef RBTREE
void nameset_delete(nameset* ns, char* name)
{
  namenode* nn = NAMESET_FIND(ns, name);
  if (nn) {
    slist_delete(ns, nn, free);
  }
}
#endif

int compar_name(const void* x, const void* y)
{
  return strcmp((const char*) x, (const char*) y);
}

void init_relset(relset* rs)
{
  INIT_RELLIST(rs->rl);
  INIT_NAMESET(&rs->entset);
}

void destroy_relset(relset* rs)
{
  DESTROY_RELLIST(rs->rl);
  DESTROY_NAMESET(&rs->entset);
}

void addent(relset* rs, char* ent_name)
{
  char* name_copy = strdup(ent_name);
  if (!NAMESET_INSERT(&rs->entset, name_copy))
    free(name_copy);
}

void delent(relset* rs, char* ent_name)
{
  relnode* i = RELLIST_FIRST(&rs->rl);
  while (i) {
    delete_entity(GET_RELATION(i), ent_name);
    i = RELLIST_NEXT(i);
  }

  NAMESET_DELETE(&rs->entset, ent_name);
}

void addrel(relset* rs, char* src_name, char* dst_name, char* rel_name)
{
  if (!NAMESET_FIND(&rs->entset, src_name) || !NAMESET_FIND(&rs->entset, dst_name))
    return;

  relnode* rn = RELLIST_FIND(rs->rl, rel_name);
  if (!rn) {
    rn = RELLIST_INSERT(rs->rl, make_relation(rel_name));
  }

  relation* rel = GET_RELATION(rn);
  insert_edge(rel, src_name, dst_name);
}

void delrel(relset* rs, char* src_name, char* dst_name, char* rel_name)
{
  relnode* rn = RELLIST_FIND(rs->rl, rel_name);
  if (rn) {
    delete_edge(GET_RELATION(rn), src_name, dst_name);
  }
}

void report(relset* rs)
{
  relnode* i = RELLIST_FIRST(&rs->rl);
  bool reported = false;
  while (i) {
    relation* r = GET_RELATION(i);
    if (!is_empty_rel(r)) {
      if (reported)
	putchar(' ');

      reported = true;
      report_relation(r);
    }
    i = RELLIST_NEXT(i);
  }

  if (!reported)
    puts("none");
  else
    putchar('\n');
}
