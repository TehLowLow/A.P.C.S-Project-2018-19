#ifndef RELATION_H
#define RELATION_H

#include <stdbool.h>

#include "slist.h"

/*** Key Type ***/
typedef char* name_t;

/*** Degree Index ***/
typedef slist deg_list;
typedef snode deg_node;

struct entity;
typedef struct {
  struct entity* first;
  unsigned value;
} degree;

int compar_degree(const void* x, const void* y);
#define INIT_DEG_LIST(dl) init_slist(&dl, compar_degree)
#define DESTROY_DEG_LIST(dl) destroy_slist_nodes(&dl, free)
#define GET_FIRST_DEGREE(dl) (dl.first)
#define GET_LAST_DEGREE(dl) (dl.last)
#define GET_DEGREE(dn) ((degree*) dn->data)
#define GET_LOWER_DEG_NODE(dn) (dn->next)
#define GET_HIGHER_DEG_NODE(dn) (dn->prev)
#define DELETE_DEG_NODE(dl,dn) slist_delete(&dl, dn, free)
deg_node* insert_new_indegree_before(deg_list* dl, deg_node* next, unsigned new_indeg);

degree* make_degree(struct entity* first, unsigned value);

int compar_ent(const void* x, const void* y);

/*** Adjacency source list ***/
#ifdef RBTREE
#include "rb.h"
typedef rbtree adj_list;
typedef rbnode adj_node;

#define INIT_ADJ_LIST(al) init_rbtree(&al, compar_ent)
#define DESTROY_ADJ_LIST(al) destroy_rbtree(&al, NULL)
#define ADJ_LIST_INSERT(al, sent) rbinsert_key(&al, sent)
#define ADJ_LIST_FIND(al, sent) rbsearch(&al, sent)
#define ADJ_LIST_DELETE(al, an) rbdelete(&al, an); destroy_rbnode(an, NULL)
#define ADJ_NODE_GET(an) ((struct entity*) an->key)

#else
typedef slist adj_list;
typedef snode adj_node;

#define INIT_ADJ_LIST(sl) init_slist(&sl, compar_ent)
#define DESTROY_ADJ_LIST(sl) destroy_slist_nodes(&sl, NULL)
#define ADJ_LIST_INSERT(sl, sent) slist_insert(&sl, sent)
#define ADJ_LIST_FIND(sl, sent) slist_find(&sl, sent)
#define ADJ_LIST_DELETE(sl, sn) slist_delete(&sl, sn, NULL)
#define ADJ_LIST_FIRST(sl) sl.first
#define ADJ_NODE_NEXT(sn) sn->next
#define ADJ_NODE_GET(sn) ((struct entity*) sn->data)
#endif


/*** Entity and Relation ***/
typedef struct entity {
  name_t name;
  adj_list sources;
  adj_list destinations;
  deg_node* indegree;
  struct entity* next;
  struct entity* prev;
} entity;

typedef struct {
  name_t name;
  entity* ent_first;
  entity* ent_last;
  deg_list degree_index;
#ifdef RBTREE
  rbtree ent_index;
#endif
} relation;


relation* make_relation(name_t name);
void destroy_relation(relation* rel);

bool is_empty_rel(relation* rel);
entity* insert_entity(relation* rel, name_t name);
void delete_entity(relation* rel, name_t name);
void insert_edge(relation* rel, name_t src, name_t dst);
void delete_edge(relation* rel, name_t src, name_t dst);
void delete_edge_ent(relation* r, entity* src_ent, entity* dst_ent);

void insert_entity_new_indegree(relation* r, entity* next_indeg,
				entity* new_ent, unsigned new_indeg);
/*
 * Insert new_ent in alphabetical order between begin and end.
 * begin is the beginning of the subsequence of the same indegree of new_ent,
 * end is the element after the last of the sequence (NULL if end of list).
 */
int insert_entity_same_indegree(relation* r, entity* begin, entity* end, entity* new_ent);
void insert_entity_before(relation* r, entity* next, entity* new_ent);
void remove_entity(relation* r, entity* e);
entity* find_entity(relation* r, name_t name);

entity* make_entity(name_t name);
void destroy_entity(entity* ent);

void report_relation(relation* r);
void report_name(const char* name);

#endif
