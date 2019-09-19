#ifndef RELSET_H
#define RELSET_H

#include <stdlib.h>

#include "slist.h"
#include "relation.h"

/*** Relation list ***/
typedef slist rellist;
typedef snode relnode;

relnode* rellist_find(rellist* rl, const char* name);
int compar_rel(const void* x, const void* y);
void destroy_relation_void(void* r);
#define INIT_RELLIST(rs) init_slist(&rs, compar_rel)
#define DESTROY_RELLIST(rs) destroy_slist_nodes(&rs, destroy_relation_void)
#define RELLIST_FIRST(rl) (rl)->first
#define RELLIST_NEXT(rn) (rn)->next
#define GET_RELATION(rn) ((relation*) rn->data)
#define RELLIST_FIND(rl, name) rellist_find(&rl, name)
#define RELLIST_INSERT(rl, rel) slist_insert(&(rl), rel)

/*** Name Set ***/
#ifdef RBTREE
#include "rb.h"
typedef rbtree nameset;
typedef rbnode namenode;

int compar_name(const void* x, const void* y);
#define INIT_NAMESET(ns) init_rbtree(ns, compar_name)
#define DESTROY_NAMESET(ns) destroy_rbtree(ns, free)
#define NAMESET_INSERT(ns, name) rbinsert_key(ns, name)
#define NAMESET_FIND(ns, name) rbsearch(ns, name)
#define NAMESET_DELETE(ns, name) rb_delete_key(ns, free, name)

#else
typedef slist nameset;
typedef snode namenode;

void nameset_delete(nameset* ns, char* name);
int compar_name(const void* x, const void* y);
#define INIT_NAMESET(nl) init_slist(nl, compar_name)
#define DESTROY_NAMESET(nl) destroy_slist_nodes(nl, free)
#define NAMESET_INSERT(ns, name) slist_insert(ns, name)
#define NAMESET_FIND(ns, name) slist_find(ns, name)
#define NAMESET_DELETE(ns, name) nameset_delete(ns, name)
#endif

typedef struct {
  rellist rl;
  nameset entset;
} relset;

void init_relset(relset* rs);
void destroy_relset(relset* rs);

void addent(relset* rs, char* ent_name);
void delent(relset* rs, char* ent_name);
void addrel(relset* rs, char* src_name, char* dst_name, char* rel_name);
void delrel(relset* rs, char* src_name, char* dst_name, char* rel_name);
void report(relset* rs);

#endif
