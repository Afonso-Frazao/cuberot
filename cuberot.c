#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEDGE 50  // half edge size
#define CHARNUM 7 // number of different characters
// #define COLNUM 37
#define COLNUM 74
// #define ROWNUM 16
#define ROWNUM 32
// #define EDGESIZE 8
#define EDGESIZE 16

#define RED_COLOR "\033[38;5;9m"
#define RESET_COLOR "\033[0m"

typedef struct _cart {
  double x; // the viewpoint of the user will be to the yz plane with
  double y; // x pointing to the user
  double z;
} cart;

typedef struct _sphr {
  double r; // distance to origin
  double t; // horizontal angle
  double f; // vertical angle
} sphr;

typedef struct _edge {
  int a;
  int b;
} edge;

typedef struct _param { // parametric equations
  double xslope;
  double xintercept;
  double yslope;
  double yintercept;
  double zslope;
  double zintercept;
} param;

double sqr(double x) { return x * x; }

int findindex(double c, double *regions) {
  int i;

  for (i = 0; 1; i++) {
    if (c <= regions[i]) {
      return i;
    }
  }
}

char decodechar(int num) {

  switch (num) {
  case 1:
    return '.';
  case 2:
    return ':';
  case 3:
    return ';';
  case 4:
    return 'u';
  case 5:
    return 'o';
  case 6:
    return 'O';
  case 7:
    return '0';
  default: // TODO remove this later
    perror("Error");
    exit(1);
  }
}

void initcube(cart *ccube) {
  int i, j, k;

  k = 1;
  for (i = 0; i < 8; i++) {
    ccube[i].x = HEDGE * k;
    k = -k;
  }

  k = 1;
  for (i = 0; i < 8; i += 2) {
    for (j = 0; j < 2; j++) {
      ccube[i + j].y = HEDGE * k;
    }
    k = -k;
  }

  k = 1;
  for (i = 0; i < 8; i += 4) {
    for (j = 0; j < 4; j++) {
      ccube[i + j].z = HEDGE * k;
    }
    k = -k;
  }

  /*for (i = 0; i < 8; i++) {
    printf("%lf\t%lf\t%lf\n", ccube[i].x, ccube[i].y, ccube[i].z);
  }
  exit(0);*/

  return;
}

void initedges(cart *ccube, edge *edgeindexes) {
  int i, j, k, l, buff;

  int vertexesarr[4] = {1, 2, 4, 7};

  l = 0;                    // edgeindexes index
  for (i = 0; i < 4; i++) { // i is the vertexesarr index
    buff = vertexesarr[i];
    for (j = 0; j < 8; j++) { // j is the current index being compared
      k = 0;                  // k is the number of equal coordinates
      if (ccube[buff].x == ccube[j].x) {
        k++;
      }
      if (ccube[buff].y == ccube[j].y) {
        k++;
      }
      if (ccube[buff].z == ccube[j].z) {
        k++;
      }
      if (k == 2) {
        edgeindexes[l].a = vertexesarr[i];
        edgeindexes[l].b = j;
        l++;
      }
    }
  }

  /*for (i = 0; i < 12; i++) {
    printf("edge %d %d\n", edgeindexes[i].a, edgeindexes[i].b);
  }
  exit(0);*/

  return;
}

void carttosphr(cart *ccube, sphr *scube) {
  int i;

  for (i = 0; i < 8; i++) {
    scube[i].r = sqrt(sqr(ccube[i].x) + sqr(ccube[i].y) + sqr(ccube[i].z));
    scube[i].t = atan2(ccube[i].y, ccube[i].x);
    scube[i].f = atan2(sqrt(sqr(ccube[i].x) + sqr(ccube[i].y)), ccube[i].z);
  }

  return;
}

void sphrtocart(cart *ccube, sphr *scube) {
  int i, j;

  for (i = 0; i < 8; i++) {
    ccube[i].x = scube[i].r * cos(scube[i].t) * sin(scube[i].f);
    ccube[i].y = scube[i].r * sin(scube[i].t) * sin(scube[i].f);
    ccube[i].z = scube[i].r * cos(scube[i].f);
  }

  return;
}

void calccube(cart *ccube, int **grid, edge *edgeindexes) {
  int i;
  double cstep, rstep;

  cstep = ((double)(2 * sqrt(3) * HEDGE)) / (COLNUM);
  rstep = ((double)(2 * sqrt(3) * HEDGE)) / (ROWNUM);

  double *cregions, *rregions;

  cregions = (double *)malloc((COLNUM) * sizeof(double));
  rregions = (double *)malloc((ROWNUM) * sizeof(double));

  for (i = 0; i < COLNUM; i++) {
    cregions[i] = ((i + 1) * cstep) - (HEDGE * sqrt(3));
  }

  for (i = 0; i < ROWNUM; i++) {
    rregions[i] = ((i + 1) * rstep) - (HEDGE * sqrt(3));
  }

  // select the grid values now that I have the regions delimited

  int indexc, indexr, indexd;

  double *dregions;
  double dstep;

  dregions = (double *)malloc(CHARNUM * sizeof(double));

  dstep = (HEDGE * 2 * sqrt(3)) / CHARNUM;

  for (i = 0; i < CHARNUM; i++) {
    dregions[i] = ((i + 1) * dstep) - (HEDGE * sqrt(3));
  }

  for (i = 0; i < 8; i++) { // negative indexes for the vertexes
    indexc = findindex(ccube[i].y, cregions);
    indexr = findindex(ccube[i].z, rregions);
    indexd = -findindex(ccube[i].x, dregions);
    indexd--; // so that the index isn't zero
    if (grid[indexr][indexc] > indexd) {
      grid[indexr][indexc] = indexd;
    }
  }

  double j;
  int x1, x2, y1, y2, z1, z2;
  param *edges;
  double ystep, zstep;

  edges = (param *)malloc(12 * sizeof(param));

  for (i = 0; i < 12; i++) { // get the parametric edge equations
    x1 = ccube[edgeindexes[i].a].x;
    x2 = ccube[edgeindexes[i].b].x;
    y1 = ccube[edgeindexes[i].a].y;
    y2 = ccube[edgeindexes[i].b].y;
    z1 = ccube[edgeindexes[i].a].z;
    z2 = ccube[edgeindexes[i].b].z;

    edges[i].xintercept = x1;
    edges[i].xslope = x2 - x1;
    edges[i].yintercept = y1;
    edges[i].yslope = y2 - y1;
    edges[i].zintercept = z1;
    edges[i].zslope = z2 - z1;
  }

  // ystep = ((double)(2 * sqrt(3) * HEDGE)) / (COLNUM - 2);
  // zstep = ((double)(2 * sqrt(3) * HEDGE)) / (COLNUM - 2);

  for (i = 0; i < 12; i++) { // calculate the points of the edges
    for (j = ((double)1) / EDGESIZE; j < 1; j += ((double)1) / EDGESIZE) {
      indexc = findindex(edges[i].yintercept + (edges[i].yslope * j), cregions);
      indexr = findindex(edges[i].zintercept + (edges[i].zslope * j), rregions);
      indexd = findindex(edges[i].xintercept + (edges[i].xslope * j), dregions);
      indexd++; // so that the index isn't zero
      if (grid[indexr][indexc] >= 0) {
        if (grid[indexr][indexc] < indexd) {
          grid[indexr][indexc] = indexd;
        }
      }
    }
  }

  free(edges);

  free(cregions);
  free(rregions);
  free(dregions);

  return;
}

void calcedges(cart *ccube, int **grid, edge *edgeindexes) {
  int i, j;
  return;
}

void printcube(cart *ccube, edge *edgeindexes) {
  int i, j;
  int **grid;

  // the char proportions are approximately c*r -> 16x37

  grid = (int **)malloc(ROWNUM * sizeof(int *));
  for (i = 0; i < ROWNUM; i++) {
    grid[i] = (int *)calloc(COLNUM, sizeof(int));
  }

  calccube(ccube, grid, edgeindexes);

  // calcedges(ccube, grid, edgeindexes);

  // time to print

  for (i = 0; i < ROWNUM; i++) {
    for (j = 0; j < COLNUM; j++) {
      if (grid[i][j] != 0) {
        if (grid[i][j] < 0) { // print vertices
          printf(RED_COLOR "%c" RESET_COLOR, decodechar(-grid[i][j]));
        } else { // print edges
          printf("%c", decodechar(grid[i][j]));
        }
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }

  for (i = 0; i < ROWNUM; i++) {
    free(grid[i]);
  }
  free(grid);

  return;
}

int main() {
  int i;

  cart *ccube; // cube vertices in cartesian coordinates
  sphr *scube; // cube vertices in spherical coordinates

  ccube = (cart *)malloc(8 * sizeof(cart));
  scube = (sphr *)malloc(8 * sizeof(sphr));

  initcube(ccube);

  carttosphr(ccube, scube);

  edge *edgeindexes;

  edgeindexes = (edge *)malloc(12 * sizeof(edge));

  initedges(ccube, edgeindexes);

  double j = 0.05;

  for (i = 0; i < 30; i++) {

    for (i = 0; i < 8; i++) {
      scube[i].t += 0.1;
      /*if (scube[i].f >= 0) {
        scube[i].f += j;
      } else {
        scube[i].f -= j;
      }*/
    }
    for (i = 0; i < 8; i += 2) {
      scube[i].f += j;
      scube[i + 1].f += j;
      j = -j;
    }

    sphrtocart(ccube, scube);

    printf("\033[H\033[J"); // clear board

    printcube(ccube, edgeindexes);
    printf("teta: %lf\nfi: %lf\n", scube[0].t / M_PI, scube[0].f / M_PI);

    usleep(500000);
  }

  free(ccube);
  free(scube);
  free(edgeindexes);

  return 0;
}
