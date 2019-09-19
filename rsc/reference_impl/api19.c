#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "relset.h"

#define ADDENT "addent"
#define DELENT "delent"
#define ADDREL "addrel"
#define DELREL "delrel"
#define REPORT "report"
#define END_C  "end"

#define BUFFER_SIZE 256

void p_error(const char* msg)
{
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}

void ignore_whitespace()
{
  int c;
  while (isblank(c = getchar()));
  ungetc(c, stdin);
}

bool expect_newline()
{
  ignore_whitespace();
  int c = getchar();
  if (c != '\n') {
    p_error("Newline expected.");
    return false;
  } else
    return true;
}

bool next_token(char* buf)
{
  ignore_whitespace();
  size_t i = 0;
  int c;
  while (i < BUFFER_SIZE - 1 && isgraph(c = getchar())) {
    buf[i] = c;
    ++i;
  }
  buf[i] = '\0';
  if (i == 0)
    return false;
  else if (!isgraph(c))
    ungetc(c, stdin);

  return true;
}

bool expect_name(char* buf)
{
  ignore_whitespace();
  size_t i = 0;
  int c = getchar();
  if (c != '\"') {
    p_error("Name expected.");
    return false;
  }
  while (i < BUFFER_SIZE - 1 && (c = getchar()) != '\"') {
    buf[i] = c;
    ++i;
  }
  buf[i] = '\0';
  if (i == 0) {
    p_error("Name expected.");
    return false;
  }
  else if (c != '\"') {
    p_error("Name too long.");
    return false;
  }

  return true;

}

void process_addent(relset* rs, char* buf)
{
  if (!expect_name(buf)) {
    p_error("addent: relation name expected.");
    return;
  }

  addent(rs, buf);

  expect_newline();
}

void process_delent(relset* rs, char* buf)
{
  if (!expect_name(buf)) {
    p_error("delent: relation name expected.");
    return;
  }

  delent(rs, buf);

  expect_newline();
}

void process_addrel(relset* rs, char* buf)
{
  if (!expect_name(buf)) {
    p_error("addrel: source entity expected.");
    return;
  }
  char* src_name = strdup(buf);

  if (!expect_name(buf)) {
    p_error("addrel: destination entity expected.");
    return;
  }
  char* dst_name = strdup(buf);

  if (!expect_name(buf)) {
    p_error("addrel: relation name expected.");
    return;
  }

  addrel(rs, src_name, dst_name, buf);

  free(src_name);
  free(dst_name);
  expect_newline();
}

void process_delrel(relset* rs, char* buf)
{
  if (!expect_name(buf)) {
    p_error("delrel: source entity expected.");
    return;
  }
  char* src_name = strdup(buf);

  if (!expect_name(buf)) {
    p_error("delrel: destination entity expected.");
    return;
  }
  char* dst_name = strdup(buf);

  if (!expect_name(buf)) {
    p_error("delrel: relation name expected.");
    return;
  }

  delrel(rs, src_name, dst_name, buf);

  free(src_name);
  free(dst_name);
  expect_newline();
}

void process_report(relset* rs, char* buf)
{
/*
  if (!expect_name(buf)) {
    p_error("report: relation name expected.");
    return;
  }
*/

  report(rs);

  expect_newline();
}

bool process_command(relset* rs, char* buf)
{
  if (!next_token(buf)) {
    p_error("Command expected.");
    return false;
  }

  if (strcmp(buf, ADDENT) == 0)
    process_addent(rs, buf);
  else if (strcmp(buf, DELENT) == 0)
    process_delent(rs, buf);
  else if (strcmp(buf, ADDREL) == 0)
    process_addrel(rs, buf);
  else if (strcmp(buf, DELREL) == 0)
    process_delrel(rs, buf);
  else if (strcmp(buf, REPORT) == 0)
    process_report(rs, buf);
  else if (strcmp(buf, END_C) == 0) {
    return false;
  } else {
    strcat(buf, ": unexpected command.");
    p_error(buf);
    return false;
  }
  return true;
}

int main() {

  relset rs;
  init_relset(&rs);

  char* buf = (char*) malloc(BUFFER_SIZE);
  while (process_command(&rs, buf));

  free(buf);
  destroy_relset(&rs);
  return 0;
}
