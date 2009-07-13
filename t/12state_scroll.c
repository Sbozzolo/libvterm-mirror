#include "CUnit/CUnit.h"

#include "vterm.h"

#include <glib.h>

static VTerm *vt;

static VTermPos cursor;
static VTermRect dest;
static VTermRect src;
static VTermRect erase;

static int cb_movecursor(VTerm *_vt, VTermPos pos, VTermPos oldpos, int visible)
{
  cursor = pos;

  return 1;
}

static int cb_copyrect(VTerm *_vt, VTermRect _dest, VTermRect _src)
{
  dest = _dest;
  src  = _src;

  return 1;
}

static int cb_erase(VTerm *_vt, VTermRect rect, void *pen)
{
  erase = rect;

  return 1;
}

static VTermStateCallbacks state_cbs = {
  .movecursor = cb_movecursor,
  .copyrect   = cb_copyrect,
  .erase      = cb_erase,
};

int state_scroll_init(void)
{
  vt = vterm_new(25, 80);
  if(!vt)
    return 1;

  vterm_parser_set_utf8(vt, 1);
  vterm_set_state_callbacks(vt, &state_cbs);

  return 0;
}

static void test_linefeed(void)
{
  vterm_state_initialise(vt);
  vterm_state_get_cursorpos(vt, &cursor);

  dest.start_row = -1;

  // We're currently at (0,0). After 24 linefeeds we won't scroll
  int i;
  for(i = 0; i < 24; i++)
    vterm_push_bytes(vt, "\n", 1);

  CU_ASSERT_EQUAL(dest.start_row, -1);

  CU_ASSERT_EQUAL(cursor.row, 24);

  vterm_push_bytes(vt, "\n", 1);

  CU_ASSERT_EQUAL(dest.start_row,  0);
  CU_ASSERT_EQUAL(dest.end_row,   24);
  CU_ASSERT_EQUAL(src.start_row,  1);
  CU_ASSERT_EQUAL(src.end_row,   25);
  CU_ASSERT_EQUAL(erase.start_row, 24);
  CU_ASSERT_EQUAL(erase.end_row,   25);

  CU_ASSERT_EQUAL(cursor.row, 24);
}

static void test_ind(void)
{
  vterm_state_initialise(vt);
  vterm_state_get_cursorpos(vt, &cursor);

  vterm_push_bytes(vt, "\e[25H", 5);
  vterm_push_bytes(vt, "\eD", 2);

  CU_ASSERT_EQUAL(dest.start_row,  0);
  CU_ASSERT_EQUAL(dest.end_row,   24);
  CU_ASSERT_EQUAL(src.start_row,  1);
  CU_ASSERT_EQUAL(src.end_row,   25);
  CU_ASSERT_EQUAL(erase.start_row, 24);
  CU_ASSERT_EQUAL(erase.end_row,   25);
}

static void test_ri(void)
{
  vterm_state_initialise(vt);
  vterm_state_get_cursorpos(vt, &cursor);

  vterm_push_bytes(vt, "\eM", 2);

  CU_ASSERT_EQUAL(dest.start_row,  1);
  CU_ASSERT_EQUAL(dest.end_row,   25);
  CU_ASSERT_EQUAL(src.start_row,  0);
  CU_ASSERT_EQUAL(src.end_row,   24);
  CU_ASSERT_EQUAL(erase.start_row,  0);
  CU_ASSERT_EQUAL(erase.end_row,    1);

  CU_ASSERT_EQUAL(cursor.row, 0);
}

static void test_region(void)
{
  vterm_state_initialise(vt);
  vterm_state_get_cursorpos(vt, &cursor);

  CU_ASSERT_EQUAL(cursor.row, 0);

  dest.start_row = -1;

  vterm_push_bytes(vt, "\e[1;10r", 7);

  CU_ASSERT_EQUAL(dest.start_row, -1);

  CU_ASSERT_EQUAL(cursor.row, 0);

  int i;
  for(i = 0; i < 9; i++)
    vterm_push_bytes(vt, "\n", 1);

  CU_ASSERT_EQUAL(dest.start_row, -1);

  CU_ASSERT_EQUAL(cursor.row, 9);

  vterm_push_bytes(vt, "\n", 1);

  CU_ASSERT_EQUAL(dest.start_row,  0);
  CU_ASSERT_EQUAL(dest.end_row,    9);
  CU_ASSERT_EQUAL(src.start_row,  1);
  CU_ASSERT_EQUAL(src.end_row,   10);
  CU_ASSERT_EQUAL(erase.start_row,  9);
  CU_ASSERT_EQUAL(erase.end_row,   10);

  CU_ASSERT_EQUAL(cursor.row, 9);

  dest.start_row = -1;

  vterm_push_bytes(vt, "\e[9;10r", 7);
  vterm_push_bytes(vt, "\eM", 2);

  CU_ASSERT_EQUAL(dest.start_row, -1);

  CU_ASSERT_EQUAL(cursor.row, 8);

  vterm_push_bytes(vt, "\eM", 2);

  CU_ASSERT_EQUAL(dest.start_row,  9);
  CU_ASSERT_EQUAL(dest.end_row,   10);
  CU_ASSERT_EQUAL(src.start_row,  8);
  CU_ASSERT_EQUAL(src.end_row,    9);
  CU_ASSERT_EQUAL(erase.start_row,  8);
  CU_ASSERT_EQUAL(erase.end_row,    9);

  CU_ASSERT_EQUAL(cursor.row, 8);

  vterm_push_bytes(vt, "\e[25H", 5);

  dest.start_row = -1;

  CU_ASSERT_EQUAL(cursor.row, 24);

  vterm_push_bytes(vt, "\n", 1);

  CU_ASSERT_EQUAL(dest.start_row, -1);

  CU_ASSERT_EQUAL(cursor.row, 24);
}

static void test_sdsu(void)
{
  vterm_state_initialise(vt);
  vterm_state_get_cursorpos(vt, &cursor);

  vterm_push_bytes(vt, "\e[S", 3);

  CU_ASSERT_EQUAL(dest.start_row,  0);
  CU_ASSERT_EQUAL(dest.end_row,   24);
  CU_ASSERT_EQUAL(src.start_row,  1);
  CU_ASSERT_EQUAL(src.end_row,   25);
  CU_ASSERT_EQUAL(erase.start_row, 24);
  CU_ASSERT_EQUAL(erase.end_row,   25);

  CU_ASSERT_EQUAL(cursor.row, 0);

  vterm_push_bytes(vt, "\e[2S", 4);

  CU_ASSERT_EQUAL(dest.start_row,  0);
  CU_ASSERT_EQUAL(dest.end_row,   23);
  CU_ASSERT_EQUAL(src.start_row,  2);
  CU_ASSERT_EQUAL(src.end_row,   25);
  CU_ASSERT_EQUAL(erase.start_row, 23);
  CU_ASSERT_EQUAL(erase.end_row,   25);

  CU_ASSERT_EQUAL(cursor.row, 0);

  vterm_push_bytes(vt, "\e[T", 3);

  CU_ASSERT_EQUAL(dest.start_row,  1);
  CU_ASSERT_EQUAL(dest.end_row,   25);
  CU_ASSERT_EQUAL(src.start_row,  0);
  CU_ASSERT_EQUAL(src.end_row,   24);
  CU_ASSERT_EQUAL(erase.start_row,  0);
  CU_ASSERT_EQUAL(erase.end_row,    1);

  CU_ASSERT_EQUAL(cursor.row, 0);

  vterm_push_bytes(vt, "\e[2T", 4);

  CU_ASSERT_EQUAL(dest.start_row,  2);
  CU_ASSERT_EQUAL(dest.end_row,   25);
  CU_ASSERT_EQUAL(src.start_row,  0);
  CU_ASSERT_EQUAL(src.end_row,   23);
  CU_ASSERT_EQUAL(erase.start_row,  0);
  CU_ASSERT_EQUAL(erase.end_row,    2);

  CU_ASSERT_EQUAL(cursor.row, 0);
}

#include "12state_scroll.inc"
