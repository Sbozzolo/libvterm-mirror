#ifndef __VTERM_INTERNAL_H__
#define __VTERM_INTERNAL_H__

#include "vterm.h"

#include <glib.h>

struct _VTermState
{
  VTerm *vt;

  /* [0] for user code, [1] for buffer engine */
  const VTermStateCallbacks *callbacks[2];
  void *cbdata[2];

  /* Current cursor position */
  VTermPos pos;
  /* Saved cursor position under DEC mode 1048/1049 */
  VTermPos saved_pos;

  int scrollregion_start;
  int scrollregion_end;

  /* Mouse state */
  int mouse_col, mouse_row;
  int mouse_buttons;

  /* Last glyph output, for Unicode recombining purposes */
  uint32_t *combine_chars;
  size_t combine_chars_size;
  int combine_width; // The width of the glyph above
  VTermPos combine_pos;   // Position before movement

  struct {
    int keypad:1;
    int cursor:1;
    int autowrap:1;
    int cursor_visible:1;
    int alt_screen:1;
    int saved_cursor:1;
  } mode;
};

struct _VTerm
{
  int rows;
  int cols;

  int is_utf8;

  const VTermParserCallbacks *parser_callbacks;
  void *cbdata;

  GString *inbuffer;
  GString *outbuffer;
  VTermState *state;
};

size_t vterm_parser_interpret_bytes(VTerm *vt, const char bytes[], size_t len);

void vterm_push_output_bytes(VTerm *vt, const char *bytes, size_t len);
void vterm_push_output_vsprintf(VTerm *vt, const char *format, va_list args);
void vterm_push_output_sprintf(VTerm *vt, const char *format, ...);

void vterm_state_setpen(VTermState *state, const long args[], int argcount);

int vterm_unicode_width(int codepoint);
int vterm_unicode_is_combining(int codepoint);

#endif
