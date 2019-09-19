#include "rb.h"

#include <stdlib.h>
#include <assert.h>

void rb_insert_fixup(rbtree* t, rbnode* z);
void rb_delete_fixup(rbtree* t, rbnode* x);
void rb_transplant(rbtree* t, rbnode* u, rbnode* v);
void left_rotate(rbtree* t, rbnode* x);
void right_rotate(rbtree* t, rbnode* x);
rbnode* tree_minimum(rbtree* t, rbnode* x);

rbnode* make_rbnode(rbtree* t, enum color c, void* key)
{
  rbnode* n = (rbnode*) malloc(sizeof(rbnode));
  n->c = c;
  n->p = t->nil;
  n->left = t->nil;
  n->right = t->nil;
  n->key = key;
  return n;
}

void destroy_rbnode(rbnode* n, void (*destroy)(void*))
{
  if (destroy)
    destroy(n->key);
  free(n);
}

rbnode* make_rbnil()
{
  rbnode* nil = (rbnode*) malloc(sizeof(rbnode));
  nil->c = BLACK;
  nil->p = NULL;
  nil->left = NULL;
  nil->right = NULL;
  nil->key = NULL;
  return nil;
}

void init_rbtree(rbtree* t, comp_t(compar))
{
  assert(t && compar);
  t->nil = make_rbnil();
  t->root = t->nil;
  t->compar = compar;
}

void destroy_rbtree(rbtree* t, void (*destroy)(void*))
{
  assert(t);
  destroy_rbsubtree(t, t->root, destroy);
  free(t->nil);
}

void destroy_rbsubtree(rbtree* t, rbnode* n, void (*destroy)(void*))
{
  assert(t && n);
  if (n == t->nil)
    return;
  else if (n->p != t->nil) {
    if (n == n->p->left)
      n->p->left = t->nil;
    else
      n->p->right = t->nil;
  } else if (n == t->root) {
    t->root = t->nil;
  }

  rbnode* left = n->left;
  if (left != t->nil)
    left->p = t->nil;

  rbnode* right = n->right;
  if (right != t->nil)
    right->p = t->nil;

  destroy_rbnode(n, destroy);
  destroy_rbsubtree(t, left, destroy);
  destroy_rbsubtree(t, right, destroy);
}


void* rbfind_key(rbtree* t, void* key)
{
  assert(t && key);
  rbnode* res = rbsearch(t, key);
  if (res)
    return res->key;
  else
    return NULL;
}

rbnode* rbinsert_key(rbtree* t, void* key)
{
  rbnode* nn = make_rbnode(t, RED, key);
  rbnode* inserted = rbinsert(t, nn);
  if (inserted)
    return inserted;
  else {
    destroy_rbnode(nn, NULL);
    return NULL;
  }
}

void rb_delete_key(rbtree* t, void (*destroy)(void*), void* key)
{
  rbnode* n = rbsearch(t, key);
  if (n) {
    rbdelete(t, n);
    destroy_rbnode(n, destroy);
  }
}

rbnode* rbsearch(rbtree* t, void* key)
{
  assert(t && key);
  rbnode* x = t->root;
  while (x != t->nil) {
    int cmp = t->compar(key, x->key);
    if (cmp == 0)
      return x;
    else if (cmp < 0)
      x = x->left;
    else
      x = x->right;
  }
  return NULL;
}

rbnode* rbinsert(rbtree* t, rbnode* z)
{
  rbnode* y = t->nil;
  rbnode* x = t->root;
  while (x != t->nil) {
    y = x;
    int cmp = t->compar(z->key, x->key);
    if (cmp == 0)
      return NULL;
    else if (cmp < 0)
      x = x->left;
    else
      x = x->right;
  }
  z->p = y;
  if (y == t->nil) {
    assert(t->root == t->nil);
    t->root = z;
  } else if (t->compar(z->key, y->key) < 0)
    y->left = z;
  else
    y->right = z;

  z->left = t->nil;
  z->right = t->nil;
  z->c = RED;
  rb_insert_fixup(t, z);
  return z;
}

void rbdelete(rbtree* t, rbnode* z)
{
  assert(t && z);
  rbnode* x = NULL;
  rbnode* y = z;
  enum color y_orig_c = y->c;
  if (z->left == t->nil) {
    x = z->right;
    rb_transplant(t, z, z->right);
  } else if (z->right == t->nil) {
    x = z->left;
    rb_transplant(t, z, z->left);
  } else {
    y = tree_minimum(t, z->right);
    y_orig_c = y->c;
    x = y->right;
    if (y->p == z)
      x->p = y;
    else {
      rb_transplant(t, y, y->right);
      y->right = z->right;
      y->right->p = y;
    }
    rb_transplant(t, z, y);
    y->left = z->left;
    y->left->p = y;
    y->c = z->c;
  }
  if (y_orig_c == BLACK)
    rb_delete_fixup(t, x);
}

void rb_insert_fixup(rbtree* t, rbnode* z)
{
  assert(t && z);
  while (z->p->c == RED) {
    assert(z->p->p);
    if (z->p == z->p->p->left) {
      rbnode* y = z->p->p->right;
      if (y->c == RED) {
	z->p->c = BLACK;
	y->c = BLACK;
	z->p->p->c = RED;
	z = z->p->p;
      } else {
	if (z == z->p->right) {
	  z = z->p;
	  left_rotate(t, z);
	}
	z->p->c = BLACK;
	z->p->p->c = RED;
	right_rotate(t, z->p->p);
      }
    } else {
      assert(z->p == z->p->p->right);
      rbnode* y = z->p->p->left;
      if (y->c == RED) {
	z->p->c = BLACK;
	y->c = BLACK;
	z->p->p->c = RED;
	z = z->p->p;
      } else {
	if (z == z->p->left) {
	  z = z->p;
	  right_rotate(t, z);
	}
	z->p->c = BLACK;
	z->p->p->c = RED;
	left_rotate(t, z->p->p);
      }
    }
  }
  t->root->c = BLACK;
}

void rb_delete_fixup(rbtree* t, rbnode* x)
{
  assert(t && x);
  while (x != t->root && x->c == BLACK) {
    if (x == x->p->left) {
      rbnode* w = x->p->right;
      if (w->c == RED) {
	w->c = BLACK;
	x->p->c = RED;
	left_rotate(t, x->p);
	w = x->p->right;
      }
      if (w->left->c == BLACK && w->right->c == BLACK) {
	w->c = RED;
	x = x->p;
      } else {
	if (w->right->c == BLACK) {
	  w->left->c = BLACK;
	  w->c = RED;
	  right_rotate(t, w);
	  w = x->p->right;
	}
	w->c = x->p->c;
	x->p->c = BLACK;
	w->right->c = BLACK;
	left_rotate(t, x->p);
	x = t->root;
      }
    } else {
      rbnode* w = x->p->left;
      if (w->c == RED) {
	w->c = BLACK;
	x->p->c = RED;
	right_rotate(t, x->p);
	w = x->p->left;
      }
      if (w->right->c == BLACK && w->left->c == BLACK) {
	w->c = RED;
	x = x->p;
      } else {
	if (w->left->c == BLACK) {
	  w->right->c = BLACK;
	  w->c = RED;
	  left_rotate(t, w);
	  w = x->p->left;
	}
	w->c = x->p->c;
	x->p->c = BLACK;
	w->left->c = BLACK;
	right_rotate(t, x->p);
	x = t->root;
      }
    }
  }
  x->c = BLACK;
}

void rb_transplant(rbtree* t, rbnode* u, rbnode* v)
{
  if (u->p == t->nil)
    t->root = v;
  else if (u == u->p->left)
    u->p->left = v;
  else
    u->p->right = v;
  v->p = u->p;
}

void left_rotate(rbtree* t, rbnode* x)
{
  assert(t && x && x->right);
  rbnode* y = x->right;
  x->right = y->left;
  if (y->left != t->nil)
    y->left->p = x;

  y->p = x->p;
  if (x->p == t->nil) {
    assert(t->root == x);
    t->root = y;
  } else if (x == x->p->left)
    x->p->left = y;
  else {
    assert(x == x->p->right);
    x->p->right = y;
  }

  y->left = x;
  x->p = y;
}

void right_rotate(rbtree* t, rbnode* x)
{
  assert(t && x && x->left);
  rbnode* y = x->left;
  x->left = y->right;
  if (y->right != t->nil)
    y->right->p = x;

  y->p = x->p;
  if (x->p == t->nil) {
    assert(t->root == x);
    t->root = y;
  } else if (x == x->p->left)
    x->p->left = y;
  else {
    assert(x == x->p->right);
    x->p->right = y;
  }

  y->right = x;
  x->p = y;
}

rbnode* tree_minimum(rbtree* t, rbnode* x)
{
  assert(t && x && x != t->nil);
  while (x->left != t->nil)
    x = x->left;

  return x;
}
