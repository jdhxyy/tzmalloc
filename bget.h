// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// bet.memory allocation library.
// Authors: jdh99 <jdh821@163.com>
// Bget is open source memory allocation library.Based on these,edit and support several ram

#ifndef _
#ifdef PROTOTYPES
#define  _(x)  x		      /* If compiler knows prototypes */
#else
#define  _(x)  ()                     /* It it doesn't */
#endif /* PROTOTYPES */
#endif

// suport ram num
#define BGET_RAM_NUM 3

// silent warn
#pragma diag_suppress 1294
#pragma diag_suppress 1295

typedef long bufsize;
void	bpool	    _((int ramIndex, void *buffer, bufsize len));
void   *bget	    _((int ramIndex, bufsize size));
void   *bgetz	    _((int ramIndex, bufsize size));
void   *bgetr	    _((int ramIndex, void *buffer, bufsize newsize));
void	brel	    _((int ramIndex, void *buf));
void	bectl	    _((int (*compact)(bufsize sizereq, int sequence),
		       void *(*acquire)(bufsize size),
		       void (*release)(void *buf), bufsize pool_incr));
void	bstats	    _((int ramIndex, bufsize *curalloc, bufsize *totfree, bufsize *maxfree,
		       long *nget, long *nrel));
void	bstatse     _((int ramIndex, bufsize *pool_incr, long *npool, long *npget,
		       long *nprel, long *ndget, long *ndrel));
void	bufdump     _((void *buf));
void	bpoold	    _((void *pool, int dumpalloc, int dumpfree));
int	bpoolv	    _((void *pool));
