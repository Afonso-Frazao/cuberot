#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HEDGE 50  // half edge size
#define CHARNUM 7 // number of different characters
#define COLNUM 37
#define ROWNUM 16

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

void printcube(cart *ccube) {
  int i, j;
  int **grid;

  // the char proportions are approximately c*r -> 16x37

  grid = (int **)malloc(ROWNUM * sizeof(int *));
  for (i = 0; i < ROWNUM; i++) {
    grid[i] = (int *)calloc(COLNUM, sizeof(int));
  }

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

  for (i = 0; i < 8; i++) {
    indexc = findindex(ccube[i].y, cregions);
    indexr = findindex(ccube[i].z, rregions);
    indexd = findindex(ccube[i].x, dregions);
    indexd++; // so that the index isn't zero
    if (grid[indexr][indexc] < indexd) {
      grid[indexr][indexc] = indexd;
    }
  }

  free(cregions);
  free(rregions);
  free(dregions);

  // time to print

  for (i = 0; i < ROWNUM; i++) {
    for (j = 0; j < COLNUM; j++) {
      if (grid[i][j] != 0) {
        printf("%c", decodechar(grid[i][j]));
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

  double j = 0.2;
  // double j = 0.3;

  for (i = 0; i < 30; i++) {
    carttosphr(ccube, scube);

    for (i = 0; i < 8; i++) {
      // scube[i].t += 0.1;
      scube[i].f += j;
    }
    // j = -j;

    sphrtocart(ccube, scube);

    printf("\033[H\033[J");

    printcube(ccube);

    usleep(500000);
  }

  free(ccube);
  free(scube);

  return 0;
}
