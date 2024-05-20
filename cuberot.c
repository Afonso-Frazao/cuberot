#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define HEDGE 50  // half edge size
#define CHARNUM 7 // number of different characters

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

char calcchar(double x) {
  int i;
  double *dregions;
  double dstep;
  char chr;

  dregions = (double *)malloc((CHARNUM + 1) * sizeof(double));

  dstep = (HEDGE * 2 * sqrt(2)) / CHARNUM;

  for (i = 0; i < CHARNUM; i++) {
    dregions[i] = (i * dstep) - (HEDGE * sqrt(2));
    printf("dregion %d: %lf\n", i, dregions[i]);
  }
  printf("\n%lf\n", x);

  for (i = 0; 1; i++) {
    if (x <= dregions[i]) {
      break;
    }
  }

  free(dregions);

  switch (i) {
  case 0:
    return '.';
  case 1:
    return ':';
  case 2:
    return ';';
  case 3:
    return '|';
  case 4:
    return 'o';
  case 5:
    return 'O';
  case 6:
    return '0';
  default:
    printf("ksdhewwgcsdj %d\n", i);
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
    scube[i].r = sqrt(sqr(ccube[i].x) + sqr(ccube[i].y));
    scube[i].t = atan(ccube[i].y / ccube[i].x);
    scube[i].f = atan(scube[i].r / ccube[i].z);
  }

  return;
}

void sphrtocart(cart *ccube, sphr *scube) {
  int i;

  for (i = 0; i < 8; i++) {
    ccube[i].x = scube[i].r * cos(scube[i].t) * sin(scube[i].f);
    ccube[i].y = scube[i].r * sin(scube[i].t) * sin(scube[i].f);
    ccube[i].z = scube[i].r * cos(scube[i].f);
  }

  return;
}

int findindexc(double y, double *cregions) {
  int i;

  for (i = 0; 1; i++) {
    if (y <= cregions[i]) {
      // printf("amoamogus %lf %lf %i\n", y, cregions[i], i);
      return i - 1;
    }
  }
}

int findindexr(double z, double *rregions) {
  int i;

  /*printf("z: %lf\n", z);
  for (i = 0; i < 39; i++) {
    printf("rregions %lf\n", rregions[i]);
  }*/

  for (i = 0; 1; i++) {
    if (z <= rregions[i]) {
      return i - 1;
    }
  }
}

void printcube(cart *ccube) {
  int i, j;
  int cols, rows;
  char **grid;
  struct winsize w;

  // get the terminal size
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    perror("Error getting terminal size\n");
    exit(1);
  }

  cols = w.ws_col;
  rows = w.ws_row;

  printf("%d x %d\n", cols, rows);

  int csize, rsize;

  // the char proportions are approximately c*r -> 16x37

  if (cols * 16 > rows * 37) {
    csize = (int)round((int)((rows * 16) / 37));
    rsize = rows;
  } else {
    csize = cols;
    rsize = (int)round((int)((cols * 37) / 16));
  }

  grid = (char **)malloc(rsize * sizeof(char *));
  for (i = 0; i < rsize; i++) {
    grid[i] = (char *)calloc(csize, sizeof(char));
  }

  double cstep, rstep;

  cstep = ((double)(2 * sqrt(2) * HEDGE)) / ((double)csize);
  rstep = ((double)(2 * sqrt(2) * HEDGE)) / ((double)rsize);

  double *cregions, *rregions;

  cregions = (double *)malloc((csize + 1) * sizeof(double));
  rregions = (double *)malloc((rsize + 1) * sizeof(double));

  for (i = 0; i <= csize; i++) {
    cregions[i] = (i * cstep) - (HEDGE * sqrt(2));
    printf("amogus%lf\n", cregions[i]);
  }

  printf("\n\n");

  for (i = 0; i <= rsize; i++) {
    rregions[i] = (i * rstep) - (HEDGE * sqrt(2));
    printf("sussy%lf\n", rregions[i]);
  }

  // select the grid values now that I have the regions delimited

  int indexr, indexc;

  for (i = 0; i < 8; i++) {
    indexc = findindexc(ccube[i].y, cregions);
    indexr = findindexr(ccube[i].z, rregions);
    // printf("the when me %lf %lf\n", ccube[i].y, ccube[i].z);
    // printf("me when the %d %d\n", indexc, indexr);
    grid[indexr][indexc] = calcchar(ccube[i].x);
    // printf("%c\n", grid[indexr][indexc]);
  }

  free(cregions);
  free(rregions);

  // time to print

  for (i = 0; i < rsize; i++) {
    for (j = 0; j < csize; j++) {
      if (grid[i][j] != 0) {
        printf("%c", grid[i][j]);
      } else {
        printf(" ");
      }
    }
    printf("\n");
  }

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

  for (i = 0; i < 8; i++) {
    scube[i].t += 37.5;
    scube[i].f -= 23.2;
  }

  sphrtocart(ccube, scube);

  printcube(ccube);

  free(ccube);
  free(scube);

  return 0;
}
