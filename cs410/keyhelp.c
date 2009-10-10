/**
 * The 15-410 reference kernel keyboard handling code.
 *
 * @author Steve Muckle <smuckle@andrew.cmu.edu>
 *
 * @author Edited by zra for the 2003-2004 season.
 *
 * @author Edited by mpa for spring 2004
 *
 * @author Rewritten by nwf for spring 2007.
 *
 * Functions for turning keyboard scancodes
 * into chars.
 *
 * Notice that we use Scancode Set 1
 */
/*@{*/

#include "keyhelp.h"

/**
 * This is returned as the upper bits of the result of
 * process_scancode and may be interrogated more readily by
 * the KH_ macros in keyhelp.h
 *
 * WARNING:
 * The bottom bits overlap with the KH_RESULT_ codes in the
 * return value.
 */
static short key_state = 0;

  /** Currently processing a PRINT SCREEN BREAK sequence */
#define KH_PRSCR_UP_SCAN    0x0008
  /** Currently processing a PRINT SCREEN MAKE sequence */
#define KH_PRSCR_DOWN_SCAN  0x0004
  /** Currently processing a PAUSE/BREAK (MAKE) sequence */
#define KH_PAUSE_SCAN       0x0002
  /** Currently processing an extended sequence (E0 prefix) */
#define KH_EXTENDED_SCAN    0x0001
static short key_internal_state = 0;

static int key_sequence = 0;

#define KHS_SHIFT_CORE (key_state & (KH_LSHIFT_KEY | KH_RSHIFT_KEY)) 
#define KHS_CTL_CORE (key_state & (KH_LCONTROL_KEY | KH_RCONTROL_KEY)) 

#define KHS_SHIFT(c,r,s,u) \
  { c = KHS_SHIFT_CORE ? s : u; r = u; }
#define KHS_SHIFTCTL(c,r,cc,s,u) \
  { c = KHS_CTL_CORE ? cc : (KHS_SHIFT_CORE ? s : u); r = u; }
#define KHS_SHIFTCAPSCTL(c,r,cc,s,u) \
  { c = KHS_CTL_CORE ? cc : (((KHS_SHIFT_CORE || (key_state & KH_CAPS_LOCK)) && \
    !(KHS_SHIFT_CORE && (key_state & KH_CAPS_LOCK))) ? s : u) ; r = u; }

/**
 * This function performs the mapping
 * from simple scancodes to chars.
 *
 * @param scancode a simple scancode.
 * @param pressed 0 if released, nonzero if pressed.
 *
 * @return A partially constructed kh_type.
 */
static kh_type
process_simple_scan(int scancode, int pressed)
{
  unsigned char code = 0x80;
  unsigned char rcode = 0x80;
  kh_type res = 0;

  switch(scancode & 0x7F)
  {
  case 0x1:
    /* Escape key. */
    rcode = code = 0x1B;
    break;
  case 0x2:
    /* 1 or ! */
    KHS_SHIFT(code, rcode, '!', '1');
    break;
  case 0x3:
    /* 2 or @ */
    KHS_SHIFTCTL(code, rcode, 0x00, '@', '2');
    break;
  case 0x4:
    /* 3 or # */
    KHS_SHIFT(code, rcode, '#', '3');
    break;
  case 0x5:
    /* 4 or $ */
    KHS_SHIFT(code, rcode, '$', '4');
    break;
  case 0x6:
    /* 5 or % */
    KHS_SHIFT(code, rcode, '%', '5');
    break;
  case 0x7:
    /* 6 or ^ */
    KHS_SHIFTCTL(code, rcode, 0x1E, '^', '6');
    break;
  case 0x8:
    /* 7 or & */
    KHS_SHIFT(code, rcode, '&', '7');
    break;
  case 0x9:
    /* 8 or * */
    KHS_SHIFT(code, rcode, '*', '8');
    break;
  case 0xA:
    /* 9 or ( */
    KHS_SHIFT(code, rcode, '(', '9');
    break;
  case 0xB:
    /* 0 or ) */
    KHS_SHIFT(code, rcode, ')', '0');
    break;
  case 0xC:
    /* - or _ */
    KHS_SHIFTCTL(code, rcode, 0x1F, '_', '-');
    break;
  case 0xD:
    /* = or + */
    KHS_SHIFT(code, rcode, '+', '=');
    break;
  case 0xE:
    /* Backspace key. */
    rcode = code = '\b';
    break;
  case 0xF:
    /* Tab key. */
    rcode = code = '\t';
    break;
  case 0x10:
    /* q or Q. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x11, 'Q', 'q');
    break;
  case 0x11:
    /* w or W. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x17, 'W', 'w');
    break;
  case 0x12:
    /* e or E. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x05, 'E', 'e');
    break;
  case 0x13:
    /* r or R. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x12, 'R', 'r');
    break;
  case 0x14:
    /* t or T. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x14, 'T', 't');
    break;
  case 0x15:
    /* y or Y. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x19, 'Y', 'y');
    break;
  case 0x16:
    /* u or U. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x15, 'U', 'u');
    break;
  case 0x17:
    /* i or I. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x09, 'I', 'i');
    break;
  case 0x18:
    /* o or O. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x0F, 'O', 'o');
    break;
  case 0x19:
    /* p or P. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x10, 'P', 'p');
    break;
  case 0x1A:
    /* [ or {. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x1B, '{', '[');
    break;
  case 0x1B:
    /* ] or }. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x1D, '}', ']');
    break;
  case 0x1C:
    /* Enter key. */
    rcode=code='\n';
    break;
  case 0x1D:
    if((key_internal_state & KH_PAUSE_SCAN) && (key_sequence == 0))
    {
      /* Stage 1 of a pause sequence */
      key_sequence++;
      return 0;
    } else {
      key_internal_state &= ~KH_PAUSE_SCAN;
      key_sequence = 0;
    }
    rcode=KHE_LCTL;
    if(pressed)
      key_state |= KH_LCONTROL_KEY;
    else
      key_state &= ~KH_LCONTROL_KEY;
    break;
  case 0x1E:
    /* a or A. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x01, 'A', 'a');
    break;
  case 0x1F:
    /* s or S. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x13, 'S', 's');
    break;
  case 0x20:
    /* d or D. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x04, 'D', 'd');
    break;
  case 0x21:
    /* f or F. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x06, 'F', 'f');
    break;
  case 0x22:
    /* g or G. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x07, 'G', 'g');
    break;
  case 0x23:
    /* h or H. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x08, 'H', 'h');
    break;
  case 0x24:
    /* j or J. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x0A, 'J', 'j');
    break;
  case 0x25:
    /* k or K. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x0B, 'K', 'k');
    break;
  case 0x26:
    /* l or L. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x0C, 'L', 'l');
    break;
  case 0x27:
    /* ; or :. */
    KHS_SHIFT(code, rcode, ':', ';');
    break;
  case 0x28:
    /* ' or " */
    KHS_SHIFT(code, rcode, '\"', '\'');
    break;
  case 0x29:
    KHS_SHIFT(code, rcode, '~', '`');
    break;
  case 0x2A:
    rcode = KHE_LSHIFT;
    if(pressed)
      key_state |= KH_LSHIFT_KEY;
    else
      key_state &= ~KH_LSHIFT_KEY;
    break;
  case 0x2B:
    /* \ or |. */
    KHS_SHIFTCTL(code, rcode, 0x1C, '|', '\\');
    break;
  case 0x2C:
    /* z or Z. */ 
    KHS_SHIFTCAPSCTL(code, rcode, 0x1A, 'Z', 'z');
    break;
  case 0x2D:
    /* x or X. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x18, 'X', 'x');
    break;
  case 0x2E:
    /* c or C. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x03, 'C', 'c');
    break;
  case 0x2F:
    /* v or V. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x16, 'V', 'v');
    break;
  case 0x30:
    /* b or B. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x02, 'B', 'b');
    break;
  case 0x31:
    /* n or N. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x0E, 'N', 'n');
    break;
  case 0x32:
    /* m or M. */
    KHS_SHIFTCAPSCTL(code, rcode, 0x0D, 'M', 'm');
    break;
  case 0x33:
    /* , or <. */
    KHS_SHIFT(code, rcode, '<', ',');
    break;
  case 0x34:
    /* . or >. */
    KHS_SHIFT(code, rcode, '>', '.');
    break;
  case 0x35:
    /* / or ? */
    KHS_SHIFT(code, rcode, '?', '/');
    break;
  case 0x36:
    rcode = KHE_RSHIFT;
    if(pressed)
      key_state |= KH_RSHIFT_KEY;
    else
      key_state &= ~KH_RSHIFT_KEY;
    break;
  case 0x37:
    /* NP * */
    rcode = code = '*';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x38:
    rcode = KHE_LALT;
    if(pressed)
      key_state |= KH_LALT_KEY;
    else
      key_state &= ~KH_LALT_KEY;
    break;
  case 0x39:
    /* Space bar. */
    rcode = code =' ';
    break;
  case 0x3A:
    rcode = KHE_CAPSLOCK;
    if(pressed)
    {
      if(key_state & KH_CAPS_LOCK)
        key_state &= ~KH_CAPS_LOCK;
      else
        key_state |= KH_CAPS_LOCK;
    }
    break;
  case 0x3B:
    /* F1 key. */
    rcode = code = KHE_F1;
    break;
  case 0x3C:
    /* F2 key. */
    rcode = code = KHE_F2;
    break;
  case 0x3D:
    /* F3 key. */
    rcode = code = KHE_F3;
    break;
  case 0x3E:
    /* F4 key. */
    rcode = code = KHE_F4;
    break;
  case 0x3F:
    /* F5 key. */
    rcode = code = KHE_F5;
    break;
  case 0x40:
    /* F6 key. */
    rcode = code = KHE_F6;
    break;
  case 0x41:
    /* F7 key. */
    rcode = code = KHE_F7;
    break;
  case 0x42:
    /* F8 key. */
    rcode = code = KHE_F8;
    break;
  case 0x43:
    /* F9 key. */
    rcode = code = KHE_F9;
    break;
  case 0x44:
    /* F10 key. */
    rcode = code = KHE_F10;
    break;
  case 0x45:
    if((key_internal_state & KH_PAUSE_SCAN) && (key_sequence == 1))
    {
      /* Stage 2 of a pause sequence */
      key_sequence++;
      return 0;
    } else {
      key_internal_state &= ~KH_PAUSE_SCAN;
      key_sequence = 0;
    }
    rcode = KHE_NUMLOCK;
    if(pressed)
    {
      if(key_state & KH_NUM_LOCK)
        key_state &= ~KH_NUM_LOCK;
      else
        key_state |= KH_NUM_LOCK;
    }
    break;
  case 0x47:
    rcode = code = '7';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x48:
    rcode = code = '8';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x49:
    rcode = code = '9';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x4A:
    rcode = code = '-';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x4B:
    rcode = code = '4';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x4C:
    rcode = code = '5';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x4D:
    rcode = code = '6';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x4E:
    rcode = code = '+';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x4F:
    rcode = code = '1';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x50:
    rcode = code = '2';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x51:
    rcode = code = '3';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x52:
    rcode = code = '0';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x53:
    rcode = code = '.';
    res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
    break;
  case 0x57:
    /* F11 key. */
    rcode = code = KHE_F11;
    break;
  case 0x58:
    /* F12 key. */
    rcode = code = KHE_F12;
    break;
  case 0xE1 & 0x7F:
    if(!(key_internal_state & KH_PAUSE_SCAN))
    {
      /* Stage 0 of a pause sequence */
      key_internal_state |= KH_PAUSE_SCAN;
      key_sequence = 0;
      return 0;
    } else if ((key_internal_state & KH_PAUSE_SCAN) && (key_sequence == 2)) {
      key_sequence++;
      return 0;
    } else {
      key_internal_state &= ~KH_PAUSE_SCAN;
      key_sequence = 0;
    }
    /* FALLTHROUGH */
  default:
    rcode = code = KHE_UNDEFINED;
    break;
  }

  if ( rcode != KHE_UNDEFINED && code != KHE_UNDEFINED )
    res |= (KH_RESULT_HASDATA << KH_RMODS_SHIFT);
  else
    code = 0x00;

  return res | (code << KH_CHAR_SHIFT)
            | (rcode << KH_RAWCHAR_SHIFT)
            | (KH_RESULT_HASRAW << KH_RMODS_SHIFT);
}

/**
 * Processes extended scan codes.  Notably, this includes
 * the arrow keys as well as some of the more unusual keys
 * on the keyboard.
 *
 * @param keypress the extended scancode.
 * @param 0 if released. non-zero if pressed.
 *
 * @return A partially constructed kh_type.
 */
kh_type
process_extended_scan(int keypress, int pressed)
{
  unsigned char code = 0x80;
  unsigned char rcode = 0x80;
  kh_type res = 0;

  /* Intermediate states in multiple byte scancodes should return
   * zero from this function, rather than returning a RESULT code.
   */

  switch(keypress & 0x7F)
  {
    case 0x1C:
      /* NP '\n' */
      rcode = code = '\n';
      res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
      break;
    case 0x1D:
      /* Right control key */
      rcode = KHE_RCTL;
      if(pressed)
        key_state |= KH_RCONTROL_KEY;
      else
        key_state &= ~KH_RCONTROL_KEY;
      break;
    case 0x2A:
      /* Stage 0 of PRINT SCREEN MAKE and Stage 1 of PRINT SCREEN BREAK */
      if(key_internal_state & KH_PRSCR_UP_SCAN)
      {
        rcode = code = KHE_PRINT_SCREEN;
        key_internal_state &= ~KH_PRSCR_UP_SCAN;
      } else if (!(key_internal_state & KH_PRSCR_UP_SCAN)) {
        key_internal_state |= KH_PRSCR_DOWN_SCAN;
        key_internal_state &= ~KH_EXTENDED_SCAN;
        return 0;
      } else {
        rcode = code = KHE_UNDEFINED;
      }
      break;
    case 0x35:
      /* NP / */
      rcode = code = '/';
      res |= KH_RESULT_NUMPAD << KH_RMODS_SHIFT;
      break;
    case 0x37:
      /* Stage 1 of PRINT SCREEN MAKE and Stage 0 of PRINT SCREEN BREAK */
      if(key_internal_state & KH_PRSCR_DOWN_SCAN)
      {
        rcode = code = KHE_PRINT_SCREEN;
        key_internal_state &= ~KH_PRSCR_DOWN_SCAN;
      } else if (!(key_internal_state & KH_PRSCR_DOWN_SCAN)) {
        key_internal_state |= KH_PRSCR_UP_SCAN;
        key_internal_state &= ~KH_EXTENDED_SCAN;
        return 0;
      } else {
        rcode = code = KHE_UNDEFINED;
      }
      break;
    case 0x38:
      /* Right alt key */
      rcode = KHE_RALT;
      if(pressed)
        key_state |= KH_RALT_KEY;
      else
        key_state &= ~KH_RALT_KEY;
      break;
    case 0x48:
      /* UP */
      rcode = code=KHE_ARROW_UP;
      break;
    case 0x4b:
      /* LEFT */
      rcode = code=KHE_ARROW_LEFT;
      break;
    case 0x4d:
      /* RIGHT */
      rcode = code=KHE_ARROW_RIGHT;
      break;
    case 0x50:
      /* DOWN */
      rcode = code = KHE_ARROW_DOWN;
      break;
    case 0x53:
      /* DEL */
      rcode = code = 0x7F;
      break;
    default:
      rcode = code = KHE_UNDEFINED;
      break;
  }

  key_internal_state &= ~KH_EXTENDED_SCAN;

  if ( rcode != KHE_UNDEFINED && code != KHE_UNDEFINED )
    res |= (KH_RESULT_HASDATA << KH_RMODS_SHIFT);
  else
    code = 0x00;

  return res | (code << KH_CHAR_SHIFT)
            | (rcode << KH_RAWCHAR_SHIFT)
            | (KH_RESULT_HASRAW << KH_RMODS_SHIFT);
}

  /** The entrypoint to the keyboard processing library.
   *
   * @param keypress A raw scancode as returned by the keyboard hardware.
   * @return A kh_type indicating the keyboard modifier key states, result
   *         modifier bits, and potentially ASCII/410 Upper Code Plane
   *         translations.
   */
kh_type process_scancode(int keypress) {
  kh_type res;
  int pressed = !(keypress & 0x80);
  int keycode = keypress & 0x7F;
  
  if (key_internal_state & KH_EXTENDED_SCAN)
    res = process_extended_scan(keycode, pressed);
  else
  {
    switch(keypress & 0xFF)
    {
      case 0x9D:
        if ((key_internal_state & KH_PAUSE_SCAN) && (key_sequence == 3))
        {
          key_sequence++;
          return 0;
        } else {
          key_internal_state &= ~KH_PAUSE_SCAN;
          key_sequence = 0;
        }
        goto deflt;
      case 0xC5:
        if ((key_internal_state & KH_PAUSE_SCAN) && (key_sequence == 4))
        {
          key_internal_state &= ~KH_PAUSE_SCAN;
          /* Pause sequence completed */
          res = (KHE_PAUSE << KH_CHAR_SHIFT)
                | (KHE_PAUSE << KH_RAWCHAR_SHIFT)
                | (KH_RESULT_HASDATA << KH_RMODS_SHIFT);
          break;
        }
        key_internal_state &= ~KH_PAUSE_SCAN;
        key_sequence = 0;
        goto deflt;
      case 0xE0:
        key_internal_state |= KH_EXTENDED_SCAN;
        /* Return no result for this intermediate state */
        return key_state << KH_STATE_SHIFT;
      default:
deflt:
        key_internal_state &= ~(KH_PRSCR_UP_SCAN | KH_PRSCR_DOWN_SCAN);
        res = process_simple_scan(keycode, pressed);
    }
  }

  if(pressed)
    res |= KH_RESULT_MAKE << KH_RMODS_SHIFT;

  res |= (key_state & KH_STATE_SMASK) << KH_STATE_SHIFT;

  return res;
}

/*@}*/
