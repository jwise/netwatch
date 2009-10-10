#ifndef KEYHELP_H
#define KEYHELP_H

#define KEY_IDT_ENTRY 0x21
#define KEYBOARD_PORT 0x60

/** Return type of process_scancode.  Four bytes wide, which are broken down as
 * follows:
 *
 * - The top two (most significant) bytes are a state field composed of the
 *   KH_*_KEY, KH_*_LOCK, and KH_RESULT_* values below.
 * - The third byte is defined as the "raw" result and gives a cannonical,
 *   unshifted name for the key pressed, if KH_HASDATA() gives true
 *   [KH_RESULT_HASDATA on].
 * - The fourth (least significant byte) gives a "common" value of the result.
 *   This is the representation subject to all modifier keys.
 */
typedef int kh_type;

  /**@{ kh_type bitfield breakdown definitions */
#define KH_STATE_SHIFT 16
#define KH_STATE_SMASK 0x0FF00
#define KH_RMODS_SHIFT 16
#define KH_RMODS_SMASK 0x000F
  /* Note that there are bits in neither of these MASKs; these are reserved for
   * future expanson; see commentary below.
   */
#define KH_RAWCHAR_SHIFT 8
#define KH_RAWCHAR_SMASK 0x0FF
#define KH_CHAR_SHIFT 0
#define KH_CHAR_SMASK 0x0FF
  /**@}*/

  /**@{ Utility macros for parsing the return of process_scancode() */
  /** Extract STATE fields from return type */
#define KH_STATE(k) ((k >> KH_STATE_SHIFT) & KH_STATE_SMASK)
  /** Extract RMODS fields from return type */
#define KH_RMODS(k) ((k >> KH_RMODS_SHIFT) & KH_RMODS_SMASK)
  /** Extract RAWCHAR field from return type */
#define KH_GETRAW(k) ((k >> KH_RAWCHAR_SHIFT) & KH_RAWCHAR_SMASK)
  /** Extract CHARACTER field from return type */
#define KH_GETCHAR(k) ((k >> KH_CHAR_SHIFT) & KH_CHAR_SMASK)

  /** Return nonzero if the CapsLock is on in the status word k */
#define KH_CAPSLOCK(k) (!!((k>>KH_STATE_SHIFT)&KH_CAPS_LOCK))
  /** Return nonzero if either shift is down in the status word k */
#define KH_SHIFT(k) (!!((k>>KH_STATE_SHIFT)&(KH_LSHIFT_KEY|KH_RSHIFT_KEY)))
  /** Return nonzero if either ctl is down in the status word k */
#define KH_CTL(k) (!!((k>>KH_STATE_SHIFT)&(KH_LCONTROL_KEY|KH_RCONTROL_KEY)))
  /** Return nonzero if either alt is down in the status word k */
#define KH_ALT(k) (!!((k>>KH_STATE_SHIFT)&(KH_LALT_KEY|KH_RALT_KEY)))

  /** Return nonzero if the status word k contains a raw character value */
#define KH_HASRAW(k) (!!((k>>KH_RMODS_SHIFT)&KH_RESULT_HASRAW))
  /** Return nonzero if the status word k contains a character value */
#define KH_HASDATA(k) (!!((k>>KH_RMODS_SHIFT)&KH_RESULT_HASDATA))
  /** Return nonzero if the status word k results are from the Numeric Pad.
   * @pre Valid only if KH_HASDATA(k) is nonzero. */
#define KH_NUMPAD(k) (!!((k>>KH_RMODS_SHIFT)&KH_RESULT_NUMPAD))
  /** Return nonzero if the status work k results from a key going down.
   * @pre Valid only if KH_HASDATA(k) is nonzero. */
#define KH_ISMAKE(k) (!!((k>>KH_RMODS_SHIFT)&KH_RESULT_MAKE))

  /** Return nonzero if the status word k character is from the
   * 410 Upper Code Plane.
   *
   * @pre Valid only if KH_HASDATA(k) is nonzero. */
#define KH_ISEXTENDED(k) (!!(KH_GETCHAR(k)&0x80))

  /** Return nonzero if the status word k raw result character is from
   * the 410 Upper Code Plane.
   *
   * @pre Valid only if KH_HASDATA(k) is nonzero. */
#define KH_ISRAWEXTENDED(k) (!!(KH_GETRAW(k)&0x80))
  /**@}*/

/**@{ key_state variables */
  /** Left shift */
#define KH_LSHIFT_KEY       0x8000
  /** Right shift */
#define KH_RSHIFT_KEY       0x4000
  /** Left control */
#define KH_LCONTROL_KEY     0x2000
  /** Right control */
#define KH_RCONTROL_KEY     0x1000
  /** Left alt */
#define KH_LALT_KEY         0x0800
  /** Right alt */
#define KH_RALT_KEY         0x0400
  /** Caps lock */
#define KH_CAPS_LOCK        0x0200
  /** Num lock */
#define KH_NUM_LOCK         0x0100
/**@}*/

  /* Reserved bits in range 0x00F0; these are currently NOT inside either of the
   * MASKS above.
   *
   * @bug Currently the LGUI and RGUI modifiers are ignored.
   * @bug Currently ScrollLock is not treated as a modifier.
   */

/**@{*/
  /** Result contains a meaningful answer beyond the modifier bits.  If this
   * bit is OFF in the returned word, the calling implementation SHOULD NOT
   * make access to either the result or the raw result.
   *
   * (As a debugging hint, the current implementation should be returning zeros
   * in those fields in this case.  If lots of zeros seem to be going to
   * screen, check one's handling of this case!)
   */
#define KH_RESULT_HASRAW            0x08

  /** Result contains a meaningful answer beyond the modifier bits and raw
   * code.  That is to say that this contains a translated code that you
   * probably care about.  
   */
#define KH_RESULT_HASDATA           0x04

  /** Signals that the result is from the Numeric Pad.  This implementation
   * will return ASCII values regardless of NumLock status, leaving to callers
   * to do more meaningful things for the non-ASCII meanings. */
#define KH_RESULT_NUMPAD           0x02

  /** If KH_RESULT_HASDATA, this bit is asserted for MAKE events and clear for
   * BREAK events.  Traditional parsing logic probably only wants to emit
   * chars when KH_RESULT_MAKE is turned on.
   */
#define KH_RESULT_MAKE   0x01
/**@}*/

enum kh_extended_e {
  /* Some control codes are returned within the ASCII encoding scheme, where
   * the ASCII definition is sufficiently close to what we're after.
   */
  KHE_ESCAPE = 0x1B,
  KHE_BACKSPACE = '\b',
  KHE_TAB = '\t',
  KHE_ENTER = '\n',

  /*
   * The remainder of ASCII, from 0x20 - 0x7F, is returned as ASCII.  But then
   * we run out of mappings and, rather than inventing a mapping to the unused
   * control characters, we invent our own upper code plane, which we'll call
   * the 410 Upper Code Plane.
   */

  KHE_UNDEFINED = 0x80, /**< A non-ASCII character for which we don't have an
                            escape sequence.  Traditional results would have
                            been to return a '?' but this leaves the behavior
                            to the implementation */

  KHE_ARROW_UP,
  KHE_ARROW_LEFT,
  KHE_ARROW_DOWN,
  KHE_ARROW_RIGHT,

  /* Modifiers */
  KHE_LALT,
  KHE_RALT,
  KHE_LCTL,
  KHE_RCTL,
  KHE_LSHIFT,
  KHE_RSHIFT,
  /* KHE_LGUI, */
  /* KHE_RGUI, */
  KHE_CAPSLOCK,
  KHE_NUMLOCK,

  /* F keys */
  KHE_F1,
  KHE_F2,
  KHE_F3,
  KHE_F4,
  KHE_F5,
  KHE_F6,
  KHE_F7,
  KHE_F8,
  KHE_F9,
  KHE_F10,
  KHE_F11,
  KHE_F12,

  /* Sequenced characters */
  KHE_PAUSE,
  KHE_PRINT_SCREEN,

  /** 
   * @bug Coming soon...
   * Well now, THIS is going to be a fun case.  Since we SHOULD move to
   * processing the Numeric Pad in the driver, we need to know what KP5 without
   * NumLock Turned on is... it's this key, which is best defined as
   * "KP5 without NumLock," known as "BEGIN".
   */
   /* KHE_BEGIN, */

  /**
   * @bug We do not handle these at the moment.
   */
  /* KHE_HOME, */
  /* KHE_END, */
  /* KHE_INSERT, */
  /* KHE_PAGE_UP, */
  /* KHE_PAGE_DOWN, */
  /* KHE_APPS, */ /* The "context menu" key */
};

kh_type process_scancode(int keypress);

#endif
