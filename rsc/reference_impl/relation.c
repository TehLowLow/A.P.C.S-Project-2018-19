#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "relation.h"

int compar_degree(const void* x, const void* y)
{
  return (int) ((degree*) y)->value - (int) ((degree*) x)->value;
}

deg_node* insert_new_indegree_before(deg_list* dl, deg_node* next, unsigned new_indeg)
{
  assert(dl);
  degree* new_degree = make_degree(NULL, new_indeg);
  deg_node* new_dnode = make_snode(new_degree);
  slist_insert_before(dl, next, new_dnode);
  return new_dnode;
}

degree* make_degree(struct entity* first, unsigned value)
{
  degree* new_degree = (degree*) malloc(sizeof(degree));
  new_degree->first = first;
  new_degree->value = value;
  return new_degree;
}

int compar_ent(const void* x, const void* y)
{
  return strcmp(((const entity*) x)->name, ((const entity*) y)->name);
}

relation* make_relation(name_t name)
{
  assert(name);
  relation* r = malloc(sizeof(relation));
  r->name = strdup(name);
  r->ent_first = NULL;
  r->ent_last = NULL;
  INIT_DEG_LIST(r->degree_index);
#ifdef RBTREE
  init_rbtree(&r->ent_index, compar_ent);
#endif
  return r;
}

void destroy_relation(relation* rel)
{
  assert(rel);
  free(rel->name);

  entity* i = rel->ent_first;
  entity* next;
  while (i) {
    next = i->next;
    destroy_entity(i);
    i = next;
  }

  DESTROY_DEG_LIST(rel->degree_index);
#ifdef RBTREE
  destroy_rbtree(&rel->ent_index, NULL);
#endif
  free(rel);
}

bool is_empty_rel(relation* r)
{
  deg_node* first = GET_FIRST_DEGREE(r->degree_index);
  return !first || GET_DEGREE(first)->value == 0;
}

entity* insert_entity(relation* r, name_t name)
{
  assert(r && name);
  entity* new_ent = make_entity(name);

  deg_node* deg_node0 = GET_LAST_DEGREE(r->degree_index);
  if (!deg_node0 || GET_DEGREE(deg_node0)->value != 0) {
    insert_entity_new_indegree(r, NULL, new_ent, 0);
  } else {
    new_ent->indegree = deg_node0;
    int inserted = insert_entity_same_indegree(r, GET_DEGREE(deg_node0)->first, NULL, new_ent);
    if (!inserted) {
      destroy_entity(new_ent);
      return NULL;
    }
 }
#ifdef RBTREE
  rbinsert_key(&r->ent_index, new_ent);
#endif

  return new_ent;
}

void delete_entity(relation* r, name_t name)
{
  assert(r && name);
  entity* ent = find_entity(r, name);
  if (!ent)
    return;

#ifdef RBTREE
  while (!rb_is_empty(&ent->sources)) {
    delete_edge_ent(r, ADJ_NODE_GET(ent->sources.root), ent);
  }
  while (!rb_is_empty(&ent->destinations)) {
    delete_edge_ent(r, ent, ADJ_NODE_GET(ent->destinations.root));
  }
#else
  adj_node* i = ADJ_LIST_FIRST(ent->sources);
  while (i) {
    adj_node* i_next = ADJ_NODE_NEXT(i);
    delete_edge_ent(r, ADJ_NODE_GET(i), ent);
    i = i_next;
  }
  i = ADJ_LIST_FIRST(ent->destinations);
  while (i) {
    adj_node* i_next = ADJ_NODE_NEXT(i);
    delete_edge_ent(r, ent, ADJ_NODE_GET(i));
    i = i_next;
  }
#endif

#ifdef RBTREE
  rb_delete_key(&r->ent_index, NULL, ent);
#endif
  remove_entity(r, ent);
  destroy_entity(ent);
}

void insert_edge(relation* r, name_t src, name_t dst)
{
  assert(r && src && dst);
  entity* dst_ent = find_entity(r, dst);
  if (!dst_ent)
    dst_ent = insert_entity(r, dst);

  entity* src_ent = find_entity(r, src);
  if (!src_ent)
    src_ent = insert_entity(r, src);

  if (ADJ_LIST_FIND(dst_ent->sources, src_ent))
    return;

  ADJ_LIST_INSERT(dst_ent->sources, src_ent);
  ADJ_LIST_INSERT(src_ent->destinations, dst_ent);

  unsigned new_degree = GET_DEGREE(dst_ent->indegree)->value + 1;
  deg_node* succ_dn = GET_HIGHER_DEG_NODE(dst_ent->indegree);
  entity* dst_next = dst_ent->next;
  remove_entity(r, dst_ent);
  entity* lower_first = (dst_ent->indegree) ? GET_DEGREE(dst_ent->indegree)->first : dst_next;

  if (succ_dn && GET_DEGREE(succ_dn)->value == new_degree) {
    dst_ent->indegree = succ_dn;
    insert_entity_same_indegree(r, GET_DEGREE(succ_dn)->first, lower_first, dst_ent);
  } else {
    assert(!succ_dn || GET_DEGREE(succ_dn)->value > new_degree);
    insert_entity_new_indegree(r, lower_first, dst_ent, new_degree);
  }
}

void delete_edge(relation* r, name_t src, name_t dst)
{
  assert(r && src && dst);
  delete_edge_ent(r, find_entity(r, src), find_entity(r, dst));
}

void delete_edge_ent(relation* r, entity* src_ent, entity* dst_ent)
{
  assert(r);
  if (!dst_ent || !src_ent)
    return;

  adj_node* src_node = ADJ_LIST_FIND(dst_ent->sources, src_ent);
  adj_node* dst_node = ADJ_LIST_FIND(src_ent->destinations, dst_ent);
  assert((src_node && dst_node) || (!src_node && !dst_node));
  if (src_node) {
    ADJ_LIST_DELETE(dst_ent->sources, src_node);
    ADJ_LIST_DELETE(src_ent->destinations, dst_node);
  } else
    return;

  unsigned new_degree = GET_DEGREE(dst_ent->indegree)->value - 1;
  deg_node* prev_dn = GET_LOWER_DEG_NODE(dst_ent->indegree);
  remove_entity(r, dst_ent);
  if (prev_dn && GET_DEGREE(prev_dn)->value == new_degree) {
    deg_node* prev_prev_dn = GET_LOWER_DEG_NODE(prev_dn);
    dst_ent->indegree = prev_dn;
    insert_entity_same_indegree(r, GET_DEGREE(prev_dn)->first,
				(prev_prev_dn) ? GET_DEGREE(prev_prev_dn)->first : NULL,
				dst_ent);
  } else {
    assert(!prev_dn || GET_DEGREE(prev_dn)->value < new_degree);
    insert_entity_new_indegree(r, prev_dn ? GET_DEGREE(prev_dn)->first : NULL,
			       dst_ent, new_degree);
  }
}

void insert_entity_new_indegree(relation* r, entity* next_indeg,
				entity* new_ent, unsigned new_indeg)
{
  assert(r && new_ent);
  deg_node* next_deg_node = (next_indeg) ? next_indeg->indegree : NULL;
  new_ent->indegree = insert_new_indegree_before(&r->degree_index, next_deg_node, new_indeg);
  GET_DEGREE(new_ent->indegree)->first = new_ent;
  insert_entity_before(r, next_indeg, new_ent);
}

int insert_entity_same_indegree(relation* r, entity* begin, entity* end, entity* new_ent)
{
  assert(r && begin && new_ent);
  assert(begin->indegree == new_ent->indegree);
  entity* i = begin;
  int cmp;
  while (i != end && (cmp = strcmp(i->name, new_ent->name)) < 0) {
    i = i->next;
  }

  if (i != end && cmp == 0) {
    // Entity already existing.
    return 0;
  }

  assert(i == end || cmp > 0);
  insert_entity_before(r, i, new_ent);

  if (i == begin)
    GET_DEGREE(i->indegree)->first = new_ent;

  return 1;
}

void insert_entity_before(relation* r, entity* next, entity* new_ent)
{
  assert(r && new_ent);
  if (!r->ent_first) {
    // The list is empty.
    assert(!next && !r->ent_last);
    new_ent->next = new_ent->prev = NULL;
    r->ent_first = r->ent_last = new_ent;
    return;
  }
  assert(r->ent_first && r->ent_last);
  if (!next) {
    // End of the list.
    assert(r->ent_last);
    new_ent->next = NULL;
    new_ent->prev = r->ent_last;
    r->ent_last->next = new_ent;
    r->ent_last = new_ent;
    return;
  }
  assert(next);
  new_ent->next = next;
  if (next->prev) {
    new_ent->prev = next->prev;
    next->prev->next = new_ent;
  } else {
    // Beginning of the list.
    assert(r->ent_first == next);
    new_ent->prev = NULL;
    r->ent_first = new_ent;
  }
  next->prev = new_ent;
}

void remove_entity(relation* r, entity* e)
{
  assert(r && e);
  if (GET_DEGREE(e->indegree)->first == e) {
    if (!e->next || e->next->indegree != e->indegree) {
      // This was the only entity with this indegree.
      DELETE_DEG_NODE(r->degree_index, e->indegree);
      e->indegree = NULL;
    } else {
      GET_DEGREE(e->indegree)->first = e->next;
    }
  }

  if (e->prev) {
    e->prev->next = e->next;
    if (e->next) {
      e->next->prev = e->prev;
    } else {
      assert(r->ent_last == e);
      r->ent_last = e->prev;
    }
  } else {
    // Beginning of the list.
    assert(r->ent_first == e);
    r->ent_first = e->next;
    if (e->next)
      e->next->prev = NULL;
    else {
      assert(r->ent_last == e);
      r->ent_last = NULL;
    }
  }
  e->next = e->prev = NULL;
}

entity* find_entity(relation* r, name_t name)
{
  assert(r && name);
#ifdef RBTREE
  entity tmp;
  tmp.name = name;
  return (entity*) rbfind_key(&r->ent_index, &tmp);
#else
  entity* i = r->ent_first;
  int cmp;
  while (i && (cmp = strcmp(i->name, name)) != 0) {
    i = i->next;
  }
  if (i && cmp == 0)
    return i;
  else
    return NULL;
#endif
}

entity* make_entity(name_t name)
{
  entity* new_ent = (entity*) malloc(sizeof(entity));
  new_ent->name = strdup(name);
  INIT_ADJ_LIST(new_ent->sources);
  INIT_ADJ_LIST(new_ent->destinations);
  new_ent->indegree = NULL;
  new_ent->next = new_ent->prev = NULL;
  return new_ent;
}

void destroy_entity(entity* ent)
{
  free(ent->name);
  DESTROY_ADJ_LIST(ent->sources);
  DESTROY_ADJ_LIST(ent->destinations);
  free(ent);
}

void report_relation(relation* r)
{
  entity* i = r->ent_first;
  if (!i)
    return;

  report_name(r->name);
  putchar(' ');

  deg_node* next_deg = GET_LOWER_DEG_NODE(i->indegree);
  entity* end = (next_deg) ? GET_DEGREE(next_deg)->first : NULL;
  while (i != end) {
    report_name(i->name);
    putchar(' ');
    i = i->next;
  }
  printf("%d;", GET_DEGREE(r->ent_first->indegree)->value);
}

void report_name(const char* name)
{
  putchar('\"');
  fputs(name, stdout);
  putchar('\"');
}
