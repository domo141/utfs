/*
 * $Id; amiga_list.h
 *
 * Created: Fri Jun 17 14:17:17 1994 too
 * Last modified: Thu Jan 17 20:45:53 EET 2008 too
 *
 */

#ifndef AMIGA_LIST_H
#define AMIGA_LIST_H

#ifndef AMIGA_LIST_HAVECODE /* trigger. defined only in simplexdr.c */

#ifdef AMIGA_LIST_INLINE
#define AMIGA_LIST_HAVECODE 1
#define __AMIGA_LIST_INLINE static inline
#else
#define AMIGA_LIST_HAVECODE 0
#define __AMIGA_LIST_INLINE
#endif

#else /* AMIGA_LIST_HAVECODE not defined */

#undef AMIGA_LIST_HAVECODE
#define AMIGA_LIST_HAVECODE 1
#define __AMIGA_LIST_INLINE

#endif


typedef struct AListNode
{
  struct AListNode * succ;
  struct AListNode * pred;
} AListNode;

typedef struct AList
{
  AListNode * head;
  AListNode * tail;
  AListNode * tailpred;
} AList;


__AMIGA_LIST_INLINE
void amiga_list_init(AList * list)
#if ! AMIGA_LIST_HAVECODE
    ;
#else
{
  list->head = (AListNode *)&list->tail;
  list->tail = 0;
  list->tailpred = (AListNode *)&list->head;
}
#endif

__AMIGA_LIST_INLINE
int amiga_is_list_empty(AList * list)
#if ! AMIGA_LIST_HAVECODE
    ;
#else
{
  return (list->tailpred == (AListNode *)list);
}
#endif

__AMIGA_LIST_INLINE
void amiga_list_addtail(AList * list, AListNode * node)
#if ! AMIGA_LIST_HAVECODE
    ;
#else
{
  /* order is importart, for atomicity (many readers, one writer) */
  node->pred = list->tailpred;
  node->succ = (AListNode *)&list->tail;
  list->tailpred->succ = node;
  list->tailpred = node;
}
#endif

__AMIGA_LIST_INLINE
AListNode * amiga_list_remheadq(AList * list)
#if ! AMIGA_LIST_HAVECODE
    ;
#else
{
  AListNode * node = list->head;

  list->head = node->succ;
  node->succ->pred = (AListNode *)&list->head;

  return node;
}
#endif

__AMIGA_LIST_INLINE
void amiga_node_remove(AListNode * node)
#if ! AMIGA_LIST_HAVECODE
    ;
#else
{
  AListNode * node2;

  node2 = node->succ;
  node = node->pred;
  node->succ = node2;
  node2->pred = node;
}
#endif


#endif /* amiga_list.h */

/*
 * Local variables:
 * mode: c
 * c-file-style: "stroustrup"
 * tab-width: 8
 * End:
 */
