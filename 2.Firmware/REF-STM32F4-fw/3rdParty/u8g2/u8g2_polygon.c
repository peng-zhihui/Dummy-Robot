/*

  u8g22_polygon.c

*/	


#include "u8g2.h"




/*===========================================*/
/* local definitions */

typedef int16_t pg_word_t;


struct pg_point_struct
{
  pg_word_t x;
  pg_word_t y;
};

typedef struct _pg_struct pg_struct;	/* forward declaration */

struct pg_edge_struct
{
  pg_word_t x_direction;	/* 1, if x2 is greater than x1, -1 otherwise */
  pg_word_t height;
  pg_word_t current_x_offset;
  pg_word_t error_offset;
  
  /* --- line loop --- */
  pg_word_t current_y;
  pg_word_t max_y;
  pg_word_t current_x;
  pg_word_t error;

  /* --- outer loop --- */
  uint8_t (*next_idx_fn)(pg_struct *pg, uint8_t i);
  uint8_t curr_idx;
};

/* maximum number of points in the polygon */
/* can be redefined, but highest possible value is 254 */
#define PG_MAX_POINTS 6

/* index numbers for the pge structures below */
#define PG_LEFT 0
#define PG_RIGHT 1


struct _pg_struct
{
  struct pg_point_struct list[PG_MAX_POINTS];
  uint8_t cnt;
  uint8_t is_min_y_not_flat;
  pg_word_t total_scan_line_cnt;
  struct pg_edge_struct pge[2];	/* left and right line draw structures */
};


/*===========================================*/
/* procedures, which should not be inlined (save as much flash ROM as possible */

#define PG_NOINLINE U8G2_NOINLINE

static uint8_t pge_Next(struct pg_edge_struct *pge) PG_NOINLINE;
static uint8_t pg_inc(pg_struct *pg, uint8_t i) PG_NOINLINE;
static uint8_t pg_dec(pg_struct *pg, uint8_t i) PG_NOINLINE;
static void pg_expand_min_y(pg_struct *pg, pg_word_t min_y, uint8_t pge_idx) PG_NOINLINE;
static void pg_line_init(pg_struct * const pg, uint8_t pge_index) PG_NOINLINE;

/*===========================================*/
/* line draw algorithm */

static uint8_t pge_Next(struct pg_edge_struct *pge)
{
  if ( pge->current_y >= pge->max_y )
    return 0;
  
  pge->current_x += pge->current_x_offset;
  pge->error += pge->error_offset;
  if ( pge->error > 0 )
  {
    pge->current_x += pge->x_direction;
    pge->error -= pge->height;
  }  
  
  pge->current_y++;
  return 1;
}

/* assumes y2 > y1 */
static void pge_Init(struct pg_edge_struct *pge, pg_word_t x1, pg_word_t y1, pg_word_t x2, pg_word_t y2)
{
  pg_word_t dx = x2 - x1;
  pg_word_t width;

  pge->height = y2 - y1;
  pge->max_y = y2;
  pge->current_y = y1;
  pge->current_x = x1;

  if ( dx >= 0 )
  {
    pge->x_direction = 1;
    width = dx;
    pge->error = 0;
  }
  else
  {
    pge->x_direction = -1;
    width = -dx;
    pge->error = 1 - pge->height;
  }
  
  pge->current_x_offset = dx / pge->height;
  pge->error_offset = width % pge->height;
}

/*===========================================*/
/* convex polygon algorithm */

static uint8_t pg_inc(pg_struct *pg, uint8_t i)
{
    i++;
    if ( i >= pg->cnt )
      i = 0;
    return i;
}

static uint8_t pg_dec(pg_struct *pg, uint8_t i)
{
    i--;
    if ( i >= pg->cnt )
      i = pg->cnt-1;
    return i;
}

static void pg_expand_min_y(pg_struct *pg, pg_word_t min_y, uint8_t pge_idx)
{
  uint8_t i = pg->pge[pge_idx].curr_idx;
  for(;;)
  {
    i = pg->pge[pge_idx].next_idx_fn(pg, i);
    if ( pg->list[i].y != min_y )
      break;	
    pg->pge[pge_idx].curr_idx = i;
  }
}

static uint8_t pg_prepare(pg_struct *pg)
{
  pg_word_t max_y;
  pg_word_t min_y;
  uint8_t i;

  /* setup the next index procedures */
  pg->pge[PG_RIGHT].next_idx_fn = pg_inc;
  pg->pge[PG_LEFT].next_idx_fn = pg_dec;
  
  /* search for highest and lowest point */
  max_y = pg->list[0].y;
  min_y = pg->list[0].y;
  pg->pge[PG_LEFT].curr_idx = 0;
  for( i = 1; i < pg->cnt; i++ )
  {
    if ( max_y < pg->list[i].y )
    {
      max_y = pg->list[i].y;
    }
    if ( min_y > pg->list[i].y )
    {
      pg->pge[PG_LEFT].curr_idx = i;
      min_y = pg->list[i].y;
    }
  }

  /* calculate total number of scan lines */
  pg->total_scan_line_cnt = max_y;
  pg->total_scan_line_cnt -= min_y;
  
  /* exit if polygon height is zero */
  if ( pg->total_scan_line_cnt == 0 )
    return 0;
  
  /* if the minimum y side is flat, try to find the lowest and highest x points */
  pg->pge[PG_RIGHT].curr_idx = pg->pge[PG_LEFT].curr_idx;  
  pg_expand_min_y(pg, min_y, PG_RIGHT);
  pg_expand_min_y(pg, min_y, PG_LEFT);
  
  /* check if the min side is really flat (depends on the x values) */
  pg->is_min_y_not_flat = 1;
  if ( pg->list[pg->pge[PG_LEFT].curr_idx].x != pg->list[pg->pge[PG_RIGHT].curr_idx].x )
  {
    pg->is_min_y_not_flat = 0;
  }
  else
  {
    pg->total_scan_line_cnt--;
    if ( pg->total_scan_line_cnt == 0 )
      return 0;
  }

  return 1;
}

static void pg_hline(pg_struct *pg, u8g2_t *u8g2)
{
  pg_word_t x1, x2, y;
  x1 = pg->pge[PG_LEFT].current_x;
  x2 = pg->pge[PG_RIGHT].current_x;
  y = pg->pge[PG_RIGHT].current_y;
  
  if ( y < 0 )
    return;
  if ( y >= u8g2_GetDisplayHeight(u8g2) )  // does not work for 256x64 display???
    return;
  if ( x1 < x2 )
  {
    if ( x2 < 0 )
      return;
    if ( x1 >= u8g2_GetDisplayWidth(u8g2) )
      return;
    if ( x1 < 0 )
      x1 = 0;
    if ( x2 >= u8g2_GetDisplayWidth(u8g2) )
      x2 = u8g2_GetDisplayWidth(u8g2);
    u8g2_DrawHLine(u8g2, x1, y, x2 - x1);
  }
  else
  {
    if ( x1 < 0 )
      return;
    if ( x2 >= u8g2_GetDisplayWidth(u8g2) )
      return;
    if ( x2 < 0 )
      x1 = 0;
    if ( x1 >= u8g2_GetDisplayWidth(u8g2) )
      x1 = u8g2_GetDisplayWidth(u8g2);
    u8g2_DrawHLine(u8g2, x2, y, x1 - x2);
  }
}

static void pg_line_init(pg_struct * const pg, uint8_t pge_index)
{
  struct pg_edge_struct  *pge = pg->pge+pge_index;
  uint8_t idx;  
  pg_word_t x1;
  pg_word_t y1;
  pg_word_t x2;
  pg_word_t y2;

  idx = pge->curr_idx;  
  y1 = pg->list[idx].y;
  x1 = pg->list[idx].x;
  idx = pge->next_idx_fn(pg, idx);
  y2 = pg->list[idx].y;
  x2 = pg->list[idx].x; 
  pge->curr_idx = idx;
  
  pge_Init(pge, x1, y1, x2, y2);
}

static void pg_exec(pg_struct *pg, u8g2_t *u8g2)
{
  pg_word_t i = pg->total_scan_line_cnt;

  /* first line is skipped if the min y line is not flat */
  pg_line_init(pg, PG_LEFT);		
  pg_line_init(pg, PG_RIGHT);
  
  if ( pg->is_min_y_not_flat != 0 )
  {
    pge_Next(&(pg->pge[PG_LEFT])); 
    pge_Next(&(pg->pge[PG_RIGHT]));
  }

  do
  {
    pg_hline(pg, u8g2);
    while ( pge_Next(&(pg->pge[PG_LEFT])) == 0 )
    {
      pg_line_init(pg, PG_LEFT);
    }
    while ( pge_Next(&(pg->pge[PG_RIGHT])) == 0 )
    {
      pg_line_init(pg, PG_RIGHT);
    }
    i--;
  } while( i > 0 );
}

/*===========================================*/
/* API procedures */

static void pg_ClearPolygonXY(pg_struct *pg)
{
  pg->cnt = 0;
}

static void pg_AddPolygonXY(pg_struct *pg, int16_t x, int16_t y)
{
  if ( pg->cnt < PG_MAX_POINTS )
  {
    pg->list[pg->cnt].x = x;
    pg->list[pg->cnt].y = y;
    pg->cnt++;
  }
}

static void pg_DrawPolygon(pg_struct *pg, u8g2_t *u8g2)
{
  if ( pg_prepare(pg) == 0 )
    return;
  pg_exec(pg, u8g2);
}

pg_struct u8g2_pg;

void u8g2_ClearPolygonXY(void)
{
  pg_ClearPolygonXY(&u8g2_pg);
}

void u8g2_AddPolygonXY(U8X8_UNUSED u8g2_t *u8g2, int16_t x, int16_t y)
{
  pg_AddPolygonXY(&u8g2_pg, x, y);
}

void u8g2_DrawPolygon(u8g2_t *u8g2)
{
  pg_DrawPolygon(&u8g2_pg, u8g2);
}

void u8g2_DrawTriangle(u8g2_t *u8g2, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
  u8g2_ClearPolygonXY();
  u8g2_AddPolygonXY(u8g2, x0, y0);
  u8g2_AddPolygonXY(u8g2, x1, y1);
  u8g2_AddPolygonXY(u8g2, x2, y2);
  u8g2_DrawPolygon(u8g2);
}

