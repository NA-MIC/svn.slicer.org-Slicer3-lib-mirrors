#include "bltInt.h"
#include "bltChain.h"

#define DELETED		((Edge *)-2)
#define LE 0
#define RE 1

#define Dist(p,q)    \
	hypot((p)->point.x - (q)->point.x, (p)->point.y - (q)->point.y)

typedef struct HalfEdgeStruct HalfEdge;
typedef struct FreeNodeStruct FreeNode;

typedef struct {
    int a, b, c;
    double min, max;
} Triplet;

struct FreeNodeStruct {
    FreeNode *nextPtr;
};

typedef struct {
    FreeNode *headPtr;
    int nodesize;
} FreeList;

typedef struct {
    Point2d point;
    int neighbor;
    int refCount;
} Site;

typedef struct {
    double a, b, c;
    Site *ep[2];
    Site *leftReg, *rightReg;
    int neighbor;
} Edge;

struct HalfEdgeStruct {
    HalfEdge *leftPtr, *rightPtr;
    Edge *edgePtr;
    int refCount;
    int pm;
    Site *vertex;
    double ystar;
    HalfEdge *pqNext;
};

/* Static variables */

static double left, right, top, bottom, deltax, deltay;
static Site *sites;
static int nSites;
static int siteidx;
static int sqrtNSites;
static int nVertices;
static Site *bottomsite;
static int nEdges;
static FreeList freeSites;
static FreeList freeEdges;
static FreeList freeHalfEdges;
static HalfEdge *elLeftEnd, *elRightEnd;
static int elHashsize;
static HalfEdge **elHash;
static int pqHashsize;
static HalfEdge *pqHash;
static int pqCount;
static int pqMin;

static Blt_Chain allocChain = NULL;

static void
FreeInit(FreeList *fl, int size)
{
    fl->headPtr = NULL;
    fl->nodesize = size;
}

static void
InitMemorySystem()
{
    if (allocChain == NULL) {
	allocChain = Blt_ChainCreate();
    }
    FreeInit(&freeSites, sizeof(Site));
}

static void *
AllocMemory(size)
    unsigned int size;
{
    void *ptr;

    ptr = Blt_Malloc(size);
    if (ptr == NULL) {
	return NULL;
    }
    Blt_ChainAppend(allocChain, ptr);
    return ptr;
}
    
static void
ReleaseMemorySystem()
{
    Blt_ChainLink link;
    void *ptr;

    for (link = Blt_ChainLastLink(allocChain); link != NULL;
	link = Blt_ChainPrevLink(link)) {
	ptr = Blt_ChainGetValue(link);
	if (ptr != NULL) {
	    Blt_Free(ptr);
	}
    }
    Blt_ChainDestroy(allocChain);
    allocChain = NULL;
}

INLINE static void
MakeFree(FreeNode *currPtr, FreeList *fl)
{
    currPtr->nextPtr = fl->headPtr;
    fl->headPtr = currPtr;
}


static FreeNode *
GetFree(FreeList *fl)
{
    int i;
    FreeNode *t;

    if (fl->headPtr == NULL) {
	t = AllocMemory(sqrtNSites * fl->nodesize);
	/* Thread the free nodes as a list */
	for (i = 0; i < sqrtNSites; i++) {
	    MakeFree((FreeNode *)((char *)t + i * fl->nodesize), fl);
	}
    }
    t = fl->headPtr;
    fl->headPtr = fl->headPtr->nextPtr;
    return t;
}

INLINE static void
DecrRefCount(Site *vertexPtr)
{
    vertexPtr->refCount--;
    if (vertexPtr->refCount == 0) {
	MakeFree((FreeNode *)vertexPtr, &freeSites);
    }
}

INLINE static void
IncrRefCount(Site *vertexPtr)
{
    vertexPtr->refCount++;
}

INLINE static HalfEdge *
HECreate(Edge *edgePtr, int pm)
{
    HalfEdge *he;

    he = (HalfEdge *)GetFree(&freeHalfEdges);
    he->edgePtr = edgePtr;
    he->pm = pm;
    he->pqNext = NULL;
    he->vertex = NULL;
    he->refCount = 0;
    return he;
}


static void
ElInitialize(void)
{
    FreeInit(&freeHalfEdges, sizeof(HalfEdge));
    elHashsize = 2 * sqrtNSites;

    elHash = AllocMemory(elHashsize * sizeof(HalfEdge *));
    assert(elHash);
    memset(elHash, 0, elHashsize * sizeof(HalfEdge *));

    elLeftEnd = HECreate((Edge *)NULL, 0);
    elRightEnd = HECreate((Edge *)NULL, 0);
    elLeftEnd->leftPtr = NULL;
    elLeftEnd->rightPtr = elRightEnd;
    elRightEnd->leftPtr = elLeftEnd;
    elRightEnd->rightPtr = NULL;
    elHash[0] = elLeftEnd;
    elHash[elHashsize - 1] = elRightEnd;
}

INLINE static void
ElInsert(HalfEdge *lb, HalfEdge *edgePtr)
{
    edgePtr->leftPtr = lb;
    edgePtr->rightPtr = lb->rightPtr;
    lb->rightPtr->leftPtr = edgePtr;
    lb->rightPtr = edgePtr;
}

static HalfEdge *
ElGetHash(int b)
{
    HalfEdge *he;

    if ((b < 0) || (b >= elHashsize)) {
	return NULL;
    }
    he = elHash[b];
    if ((he == NULL) || (he->edgePtr != DELETED)) {
	return he;
    }
    /* Hash table points to deleted half edge.  Patch as necessary. */

    elHash[b] = NULL;
    he->refCount--;
    if (he->refCount == 0) {
	MakeFree((FreeNode *)he, &freeHalfEdges);
    }
    return NULL;
}

static int
RightOf(HalfEdge *el, Point2d *p)
{
    Edge *e;
    Site *topsite;
    int rightOfSite, above, fast;
    double dxp, dyp, dxs, t1, t2, t3, yl;

    e = el->edgePtr;
    topsite = e->rightReg;
    rightOfSite = p->x > topsite->point.x;
    if ((rightOfSite) && (el->pm == LE)) {
	return 1;
    }
    if ((!rightOfSite) && (el->pm == RE)) {
	return 0;
    }
    if (e->a == 1.0) {
	dyp = p->y - topsite->point.y;
	dxp = p->x - topsite->point.x;
	fast = 0;
	if ((!rightOfSite & e->b < 0.0) | (rightOfSite & e->b >= 0.0)) {
	    above = dyp >= e->b * dxp;
	    fast = above;
	} else {
	    above = p->x + p->y * e->b > e->c;
	    if (e->b < 0.0) {
		above = !above;
	    }
	    if (!above) {
		fast = 1;
	    }
	}
	if (!fast) {
	    dxs = topsite->point.x - (e->leftReg)->point.x;
	    above = e->b * (dxp * dxp - dyp * dyp) <
		dxs * dyp * (1.0 + 2.0 * dxp / dxs + e->b * e->b);
	    if (e->b < 0.0) {
		above = !above;
	    }
	}
    } else {			/* e->b==1.0 */
	yl = e->c - e->a * p->x;
	t1 = p->y - yl;
	t2 = p->x - topsite->point.x;
	t3 = yl - topsite->point.y;
	above = t1 * t1 > t2 * t2 + t3 * t3;
    }
    return (el->pm == LE ? above : !above);
}

static HalfEdge *
ElLeftBnd(Point2d *p)
{
    int i, bucket;
    HalfEdge *he;

    /* Use hash table to get close to desired halfedge */

    bucket = (p->x - left) / deltax * elHashsize;
    if (bucket < 0) {
	bucket = 0;
    } else if (bucket >= elHashsize) {
	bucket = elHashsize - 1;
    }
    he = ElGetHash(bucket);
    if (he == NULL) {
	for (i = 1; /* empty */ ; i++) {
	    he = ElGetHash(bucket - i);
	    if (he != NULL) {
		break;
	    }
	    he = ElGetHash(bucket + i);
	    if (he != NULL) {
		break;
	    }
	}
    }

    /* Now search linear list of halfedges for the correct one */

    if ((he == elLeftEnd) || (he != elRightEnd && RightOf(he, p))) {
	do {
	    he = he->rightPtr;
	} while ((he != elRightEnd) && (RightOf(he, p)));
	he = he->leftPtr;
    } else {
	do {
	    he = he->leftPtr;
	} while ((he != elLeftEnd) && (!RightOf(he, p)));
    }

    /* Update hash table and reference counts */

    if ((bucket > 0) && (bucket < (elHashsize - 1))) {
	if (elHash[bucket] != NULL) {
	    elHash[bucket]->refCount--;
	}
	elHash[bucket] = he;
	elHash[bucket]->refCount++;
    }
    return he;
}

/* 
 * This delete routine can't reclaim node, since pointers from hash table may
 * be present.
 */
INLINE static void
ElDelete(HalfEdge *he)
{
    he->leftPtr->rightPtr = he->rightPtr;
    he->rightPtr->leftPtr = he->leftPtr;
    he->edgePtr = DELETED;
}

INLINE static Site *
LeftRegion(HalfEdge *he)
{
    if (he->edgePtr == NULL) {
	return bottomsite;
    }
    return ((he->pm == LE) ? he->edgePtr->leftReg : he->edgePtr->rightReg);
}

INLINE static Site *
RightRegion(HalfEdge *he)
{
    if (he->edgePtr == NULL) {
	return bottomsite;
    }
    return ((he->pm == LE) ? he->edgePtr->rightReg : he->edgePtr->leftReg);
}

static void
GeomInit(void)
{
    double sn;

    FreeInit(&freeEdges, sizeof(Edge));
    nVertices = nEdges = 0;
    sn = nSites + 4;
    sqrtNSites = sqrt(sn);
    deltay = bottom - top;
    deltax = right - left;
}

static Edge *
Bisect(Site *s1, Site *s2)
{
    double dx, dy, adx, ady;
    Edge *edgePtr;

    edgePtr = (Edge *)GetFree(&freeEdges);

    edgePtr->leftReg = s1;
    edgePtr->rightReg = s2;
    IncrRefCount(s1);
    IncrRefCount(s2);
    edgePtr->ep[0] = edgePtr->ep[1] = NULL;

    dx = s2->point.x - s1->point.x;
    dy = s2->point.y - s1->point.y;
    adx = FABS(dx);
    ady = FABS(dy);
    edgePtr->c = (s1->point.x * dx) + (s1->point.y * dy) +
	((dx * dx) + (dy * dy)) * 0.5;
    if (adx > ady) {
	edgePtr->a = 1.0;
	edgePtr->b = dy / dx;
	edgePtr->c /= dx;
    } else {
	edgePtr->b = 1.0;
	edgePtr->a = dx / dy;
	edgePtr->c /= dy;
    }

    edgePtr->neighbor = nEdges;
    nEdges++;
    return edgePtr;
}

static Site *
Intersect(HalfEdge *el1, HalfEdge *el2)
{
    Edge *e1, *e2, *e;
    HalfEdge *el;
    double d, xint, yint;
    int rightOfSite;
    Site *v;

    e1 = el1->edgePtr;
    e2 = el2->edgePtr;
    if ((e1 == NULL) || (e2 == NULL)) {
	return NULL;
    }
    if (e1->rightReg == e2->rightReg) {
	return NULL;
    }
    d = (e1->a * e2->b) - (e1->b * e2->a);
    if ((-1.0e-10 < d) && (d < 1.0e-10)) {
	return NULL;
    }
    xint = ((e1->c * e2->b) - (e2->c * e1->b)) / d;
    yint = ((e2->c * e1->a) - (e1->c * e2->a)) / d;

    if ((e1->rightReg->point.y < e2->rightReg->point.y) ||
	((e1->rightReg->point.y == e2->rightReg->point.y) &&
	    (e1->rightReg->point.x < e2->rightReg->point.x))) {
	el = el1;
	e = e1;
    } else {
	el = el2;
	e = e2;
    }
    rightOfSite = (xint >= e->rightReg->point.x);
    if ((rightOfSite && el->pm == LE) || (!rightOfSite && el->pm == RE)) {
	return NULL;
    }
    v = (Site *)GetFree(&freeSites);
    v->refCount = 0;
    v->point.x = xint;
    v->point.y = yint;
    return v;
}

INLINE static void
EndPoint(Edge *e, int lr, Site *s)
{
    e->ep[lr] = s;
    IncrRefCount(s);
    if (e->ep[RE - lr] == NULL) {
	return;
    }
    DecrRefCount(e->leftReg);
    DecrRefCount(e->rightReg);
    MakeFree((FreeNode *)e, &freeEdges);
}

INLINE static void
MakeVertex(Site *vertex)
{
    vertex->neighbor = nVertices;
    nVertices++;
}

static int
PQBucket(HalfEdge *he)
{
    int bucket;

    bucket = (he->ystar - top) / deltay * pqHashsize;
    if (bucket < 0) {
	bucket = 0;
    }
    if (bucket >= pqHashsize) {
	bucket = pqHashsize - 1;
    }
    if (bucket < pqMin) {
	pqMin = bucket;
    }
    return bucket;
}

static void
PQInsert(HalfEdge *he, Site *vertex, double offset)
{
    HalfEdge *last, *next;

    he->vertex = vertex;
    IncrRefCount(vertex);
    he->ystar = vertex->point.y + offset;
    last = &pqHash[PQBucket(he)];
    while (((next = last->pqNext) != NULL) &&
	((he->ystar > next->ystar) || 
	 ((he->ystar == next->ystar) &&
	  (vertex->point.x > next->vertex->point.x)))) {
	last = next;
    }
    he->pqNext = last->pqNext;
    last->pqNext = he;
    pqCount++;
}

static void
PQDelete(HalfEdge *he)
{
    HalfEdge *last;

    if (he->vertex != NULL) {
	last = &pqHash[PQBucket(he)];
	while (last->pqNext != he) {
	    last = last->pqNext;
	}
	last->pqNext = he->pqNext;
	pqCount--;
	DecrRefCount(he->vertex);
	he->vertex = NULL;
    }
}

INLINE static int
PQEmpty(void)
{
    return (pqCount == 0);
}

INLINE static Point2d
PQMin(void)
{
    Point2d p;

    while (pqHash[pqMin].pqNext == NULL) {
	pqMin++;
    }
    p.x = pqHash[pqMin].pqNext->vertex->point.x;
    p.y = pqHash[pqMin].pqNext->ystar;
    return p;
}

INLINE static HalfEdge *
PQExtractMin(void)
{
    HalfEdge *curr;

    curr = pqHash[pqMin].pqNext;
    pqHash[pqMin].pqNext = curr->pqNext;
    pqCount--;
    return curr;
}

static void
PQInitialize(void)
{
    pqCount = pqMin = 0;
    pqHashsize = 4 * sqrtNSites;
    pqHash = AllocMemory(pqHashsize * sizeof(HalfEdge));
    assert(pqHash);
    memset(pqHash, 0, pqHashsize * sizeof(HalfEdge *));
}

INLINE static Site *
NextSite(void)
{
    if (siteidx < nSites) {
	Site *s;

	s = &sites[siteidx];
	siteidx++;
	return s;
    }
    return NULL;
}

static int
Voronoi(Triplet *triplets)
{
    Site *newsite, *bot, *top, *temp, *p;
    Site *vertex;
    Point2d newintstar;
    int pm, count = 0;
    HalfEdge *lbnd, *rbnd, *llbnd, *rrbnd, *bisector;
    Edge *e;

    PQInitialize();
    bottomsite = NextSite();
    ElInitialize();

    newsite = NextSite();
    for (;;) {
	if (!PQEmpty()) {
	    newintstar = PQMin();
	}
	if ((newsite != NULL)
	    && ((PQEmpty()) || (newsite->point.y < newintstar.y) ||
		(newsite->point.y == newintstar.y)
		&& (newsite->point.x < newintstar.x))) {

	    /* New site is smallest */

	    lbnd = ElLeftBnd(&(newsite->point));
	    rbnd = lbnd->rightPtr;
	    bot = RightRegion(lbnd);
	    e = Bisect(bot, newsite);
	    bisector = HECreate(e, LE);
	    ElInsert(lbnd, bisector);
	    p = Intersect(lbnd, bisector);
	    if (p != NULL) {
		PQDelete(lbnd);
		PQInsert(lbnd, p, Dist(p, newsite));
	    }
	    lbnd = bisector;
	    bisector = HECreate(e, RE);
	    ElInsert(lbnd, bisector);
	    p = Intersect(bisector, rbnd);
	    if (p != NULL) {
		PQInsert(bisector, p, Dist(p, newsite));
	    }
	    newsite = NextSite();
	} else if (!PQEmpty()) {

	    /* Intersection is smallest */

	    lbnd = PQExtractMin();
	    llbnd = lbnd->leftPtr;
	    rbnd = lbnd->rightPtr;
	    rrbnd = rbnd->rightPtr;
	    bot = LeftRegion(lbnd);
	    top = RightRegion(rbnd);
	    triplets[count].a = bot->neighbor;
	    triplets[count].b = top->neighbor;
	    triplets[count].c = RightRegion(lbnd)->neighbor;
	    ++count;
	    vertex = lbnd->vertex;
	    MakeVertex(vertex);
	    EndPoint(lbnd->edgePtr, lbnd->pm, vertex);
	    EndPoint(rbnd->edgePtr, rbnd->pm, vertex);
	    ElDelete(lbnd);
	    PQDelete(rbnd);
	    ElDelete(rbnd);
	    pm = LE;
	    if (bot->point.y > top->point.y) {
		temp = bot, bot = top, top = temp;
		pm = RE;
	    }
	    e = Bisect(bot, top);
	    bisector = HECreate(e, pm);
	    ElInsert(llbnd, bisector);
	    EndPoint(e, RE - pm, vertex);
	    DecrRefCount(vertex);
	    p = Intersect(llbnd, bisector);
	    if (p != NULL) {
		PQDelete(llbnd);
		PQInsert(llbnd, p, Dist(p, bot));
	    }
	    p = Intersect(bisector, rrbnd);
	    if (p != NULL) {
		PQInsert(bisector, p, Dist(p, bot));
	    }
	} else {
	    break;
	}
    }
    return count;
}

static int
CompareSites(Site *s1, Site *s2)
{
    if (s1->point.y < s2->point.y) {
	return -1;
    }
    if (s1->point.y > s2->point.y) {
	return 1;
    }
    if (s1->point.x < s2->point.x) {
	return -1;
    }
    if (s1->point.x > s2->point.x) {
	return 1;
    }
    return 0;
}

int
Blt_Triangulate(x, y, nPoints, triplets)
    double *x, double *y,
    int nPoints;		/* Number of points in X and Y vectors */
    Triplet *triplets;		/* (out) Array of computed triangles */
{
    int i;
    Site *sp, *send;
    int n;
    
    InitMemorySystem();

    nSites = nPoints;
    sites = AllocMemory(nPoints * sizeof(Site));
    for (sp = sites, send = sites + nPoints, i = 0; sp < send; i++, sp++) {
	sp->point.x = x[i];
	sp->point.y = y[i];
	sp->neighbor = i;
	sp->refCount = 0;
    }
    qsort(sites, nSites, sizeof(Site), (QSortCompareProc *)CompareSites);

    sp = sites;
    left = right = sp->point.x;
    top = bottom = sp->point.y;
    for (sp++, send = sites + nPoints; sp < send; sp++) {
	if (sp->point.x < left) {
	    left = sp->point.x;
	} else if (sp->point.x > right) {
	    right = sp->point.x;
	}
	if (sp->point.y < top) {
	    top = sp->point.y;
	} else if (sp->point.y > bottom) {
	    bottom = sp->point.y;
	}
    }
    siteidx = 0;
    GeomInit();
    n = Voronoi(triplets);

    for (tp = triplets, tend = triplets + n; tp < tend; tp++) {
	double za, zb, zc;

	za = points[tp->a].z;
	zb = points[tp->b].z;
	zc = points[tp->c].z;

	tp->min = tp->max = za;
	if (zb < tp->min) {
	    tp->min = zb;
	}
	if (zc < tp->min) {
	    tp->min = zc;
	}
	if (zb > tp->max) {
	    tp->max = zb;
	}
	if (zc > tp->max) {
	    tp->max = zc;
	}
    }

    /* Release memory allocated for triangulation */
    ReleaseMemorySystem();
    return n;
}
