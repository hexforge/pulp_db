#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "dynref.h"



/*---
 PUBLIC API
---*/
int dynref__open(struct dynref__obj *const p, const char *ref_file_path, const char mode)
{
}

int dynref__close(struct dynref__obj *p, bool del)
{
}

signed long long dynref__append(struct dynref__obj *const p, struct dynref__buffer *const buf, const signed long long ref)
{
}

int finalise(struct dynref__obj *const p)
{
}

unsigned long long dynref__nmsgs(const struct dynref__obj *p)
{
}

unsigned long long dynref__nlinks(const struct dynref__obj *p)
{
}

unsigned long long dynref__nvals(const struct dynref__obj *p)
{
}

bool dynref__surjective(const struct dynref__obj *p)
{
}

signed long long dynref__iter__(const struct dynref__obj *p, const struct dynref__iter *i)
{
}

signed long long dynref__next(const struct dynref__iter *i)
{
}