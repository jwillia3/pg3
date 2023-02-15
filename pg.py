import sys
from ctypes import (
    byref,
    cdll,
    c_bool,
    c_char_p,
    c_float,
    c_int,
    c_size_t,
    c_ubyte,
    c_uint,
    c_void_p,
    c_wchar,
    POINTER,
    pointer,
    Structure,
    Union,
)

lib = cdll.LoadLibrary('./libpg3.so')
if not lib:
    print('Cannot find libpg3')
    exit(1)


def enum(cls, *args, **kwargs):
    """Define enumeration."""

    cls.map = {}

    # Define simple enumeration.
    for i in range(len(args)):
        setattr(cls, args[i], i)
        cls.map[i] = args[i]

    # Define numeration with values.
    for i, j in kwargs.items():
        setattr(cls, i, j)
        cls.map[j] = i

    def __repr__(self):
        i = self.value
        if i in cls.map:
            return f'{cls.map[i]}({i})'
        else:
            return str(i)

    def __eq__(self, other):
        return self.value == other

    setattr(cls, '__repr__', __repr__)
    setattr(cls, '__eq__', __eq__)


def fun(name, ret, *args, **kwargs):
    """Define function."""
    setattr(sys.modules[__name__], name, lib[name])
    f = getattr(sys.modules[__name__], name)
    f.restype = ret
    f.argtypes = kwargs.values() if kwargs else args





class PartType(c_int): pass
enum(PartType,
    'MOVE',
    'LINE',
    'CURVE3',
    'CURVE4',
    'CLOSE',
)

class PaintType(c_int): pass
enum(PaintType,
    'SOLID',
    'LINEAR',
)

class ColorSpace(c_int): pass
enum(ColorSpace,
    'SRGB',
    'LCHAB',
    'LAB',
    'XYZ',
)

class LineCap(c_int): pass
enum(LineCap,
    'BUTT',
    'SQUARE',
)

class FillRule(c_int): pass
enum(FillRule,
    'NONZERO',
    'EVEN_ODD',
)

class TextPos(c_int): pass
enum(TextPos,
    'TOP',
    'BOTTOM',
    'BASELINE',
    'CENTER',
)

class FontProp(c_int): pass
enum(FontProp,
    'FORMAT',
    'INDEX',
    'NFONTS',
    'FAMILY',
    'STYLE',
    'FULL_NAME',
    'IS_FIXED',
    'IS_ITALIC',
    'IS_SANS_SERIF',
    'IS_SERIF',
    'WEIGHT',
    'WIDTH_CLASS',
    'STYLE_CLASS',
    'STYLE_SUBCLASS',
    'ANGLE',
    'PANOSE_1',
    'PANOSE_2',
    'PANOSE_3',
    'PANOSE_4',
    'PANOSE_5',
    'PANOSE_6',
    'PANOSE_7',
    'PANOSE_8',
    'PANOSE_9',
    'PANOSE_10',
    'NGLYPHS',
    'UNITS',
    'EM',
    'AVG_WIDTH',
    'ASCENDER',
    'DESCENDER',
    'LINEGAP',
    'XHEIGHT',
    'CAPHEIGHT',
    'UNDERLINE',
    'UNDERLINE_SIZE',
    'SUB_X',
    'SUB_Y',
    'SUB_SX',
    'SUB_SY',
    'SUP_X',
    'SUP_Y',
    'SUP_SX',
    'SUP_SY',
)

class Key(c_int): pass
enum(Key,
    ENTER          = -0X28,
    ESCAPE         = -0X29,
    BACKSPACE      = -0X2A,
    TAB            = -0X2B,
    CAPS_LOCK      = -0X39,
    F1             = -0X3A,
    F2             = -0X3B,
    F3             = -0X3C,
    F4             = -0X3D,
    F5             = -0X3E,
    F6             = -0X3F,
    F7             = -0X40,
    F8             = -0X41,
    F9             = -0X42,
    F10            = -0X43,
    F11            = -0X44,
    F12            = -0X45,
    PRINT_SCREEN   = -0X46,
    SCROLL_LOCK    = -0X47,
    PAUSE          = -0X48,
    INSERT         = -0X49,
    HOME           = -0X4A,
    PAGE_UP        = -0X4B,
    DELETE         = -0X4C,
    END            = -0X4D,
    PAGE_DOWN      = -0X4E,
    RIGHT          = -0X4F,
    LEFT           = -0X50,
    DOWN           = -0X51,
    UP             = -0X52,
    NUM_LOCK       = -0X53,
    KP_DIVIDE      = -0X54,
    KP_MULTIPLY    = -0X55,
    KP_SUBTRACT    = -0X56,
    KP_ADD         = -0X57,
    KP_ENTER       = -0X58,
    KP_1           = -0X59,
    KP_2           = -0X5A,
    KP_3           = -0X5B,
    KP_4           = -0X5C,
    KP_5           = -0X5D,
    KP_6           = -0X5E,
    KP_7           = -0X5F,
    KP_8           = -0X60,
    KP_9           = -0X61,
    KP_0           = -0X62,
    KP_DECIMAL     = -0X63,
    ISO_SLASH      = -0X64,
    APPLICATION    = -0X65,
    POWER          = -0X66,
    KP_EQUAL       = -0X67,
    F13            = -0X68,
    F14            = -0X69,
    F15            = -0X6A,
    F16            = -0X6B,
    F17            = -0X6C,
    F18            = -0X6D,
    F19            = -0X6E,
    F20            = -0X6F,
    F21            = -0X70,
    F22            = -0X71,
    F23            = -0X72,
    F24            = -0X73,
    EXECUTE        = -0x74,
    HELP           = -0X75,
    MENU           = -0X76,
    SELECT         = -0X77,
    STOP           = -0X78,
    REDO           = -0X79,
    UNDO           = -0X7A,
    CUT            = -0X7B,
    COPY           = -0X7C,
    PASTE          = -0X7D,
    FIND           = -0X7E,
    MUTE           = -0X7F,
    VOLUME_UP      = -0X80,
    VOLUME_DOWN    = -0X81,
    KP_SEPARATOR   = -0X85,
    KANA_LONG      = -0X87,
    KANA           = -0X88,
    YEN            = -0X89,
    CONVERT        = -0X8A,
    NO_CONVERT     = -0X8B,
    HALF_WIDTH     = -0X8C,
    HANGUL         = -0X8E,
    HANJA          = -0X8F,
    HANKAKU        = -0X90,
    SYS_REQ        = -0X9A,
    CTRL_LEFT      = -0XE0,
    SHIFT_LEFT     = -0XE1,
    ALT_LEFT       = -0XE2,
    WIN_LEFT       = -0XE3,
    CTRL_RIGHT     = -0XE4,
    SHIFT_RIGHT    = -0XE5,
    ALT_RIGHT      = -0XE6,
    WIN_RIGHT      = -0XE7,
)

class Mods(c_int): pass
enum(Mods,
    SHIFT =  0x01,
    CTRL =   0x02,
    ALT =    0x04,
    WIN =    0x08,
    CAPS =   0x10,
    NUM =    0x20,
)

class EventType(c_int): pass
enum(EventType,
    'NONE',
    'OPEN',
    'CLOSE',
    'RESIZE',
    'REDRAW',
    'KEY_DOWN',
    'KEY_UP',
    'CHAR',
    'MOUSE_DOWN',
    'MOUSE_UP',
    'MOUSE_MOVE',
    'MOUSE_SCROLL',
    'USER',
    USER_LAST = 256,
)

class Color(Structure):
    _fields_ = [("x", c_float),
                ("y", c_float),
                ("z", c_float),
                ("a", c_float)]

    def pg_lch_to_lab(lch):
        return pg_lch_to_lab(lch)

    def pg_lab_to_xyz(lab):
        return pg_lab_to_xyz(lab)

    def pg_xyz_to_rgb(xyz):
        return pg_xyz_to_rgb(xyz)

    def pg_gamma_correct(rgb, gamma):
        return pg_gamma_correct(rgb, gamma)

    def pg_convert_color_to_srgb(color_space, color, gamma):
        return pg_convert_color_to_srgb(color_space, color, gamma)

class Pt(Structure):
    _fields_ = [("x", c_float),
                ("y", c_float)]

    zero = None

    def __add__(self, other):
        return Pt(self.x + other.x, self.y + other.y)

    def __sub__(self, other):
        return Pt(self.x - other.x, self.y - other.y)

    def __mul__(self, other):
        if isinstance(other, Pt):
            return Pt(self.x * other.x, self.y * other.y)
        return Pt(self.x * other, self.y * other)

    def __div__(self, other):
        if isinstance(other, Pt):
            return Pt(self.x / other.x, self.y / other.y)
        return Pt(self.x / other, self.y / other)

    def __rem__(self, other):
        if isinstance(other, Pt):
            return Pt(self.x % other.x, self.y % other.y)
        return Pt(self.x % other, self.y % other)

    def __eq__(self, other):
        self.x == other.x and self.y == other.y

    def __le__(self, other):
        return self.dist() <= other.dist()

    def __lt__(self, other):
        return self.dist() < other.dist()

    def __ge__(self, other):
        return self.dist() >= other.dist()

    def __gt__(self, other):
        return self.dist() > other.dist()

    def dist(self, other=zero):
        dx = self.x - other.x
        dy = self.y - other.y
        return pow(dx * dx + dy * dy, 0.5)

    def apply(self, tm):
        return tm.apply(self)

Pt.zero = Pt(0, 0)

class TM(Structure):
    _fields_ = [('a', c_float),
                ('b', c_float),
                ('c', c_float),
                ('d', c_float),
                ('e', c_float),
                ('f', c_float)]

    identity = None

    def __mul__(self, other):
        return pg_mul_tm(self, other)

    def apply(self, tm):
        return pg_apply_tm(self, tm)

    def translate(self, x, y):
        return pg_translate_tm(self, x, y)

    def scale(self, x, y):
        return pg_scale_tm(self, x, y)

    def rotate(self, rads):
        return pg_rotate_tm(self, rads)

TM.identity = TM(1, 0, 0, 1, 0, 0)

class Part(Structure):
    _fields_ = [('type', PartType),
                ('pt', Pt * 3)]

class Path(Structure):
    _fields_ = [('nparts', c_uint),
                ('parts', POINTER(Part)),
                ('cur', Pt)]

    def pg_partcount(self):
        return pg_partcount(self)

    def pg_path_close(self):
        pg_path_close(self)

    def pg_path_curve3(self, bx, by, cx, cy):
        pg_path_curve3(self, bx, by, cx, cy)

    def pg_path_curve4(self, bx, by, cx, cy, dx, dy):
        pg_path_curve4(self, bx, by, cx, cy, dx, dy)

    def pg_path_line(self, bx, by):
        pg_path_line(self, bx, by)

    def pg_path_rline(self, x, y):
        pg_path_rline(self, x, y)

    def pg_path_move(self, x, y):
        pg_path_move(self, x, y)

    def pg_path_rmove(self, x, y):
        pg_path_rmove(self, x, y)

    def pg_path_rcurve3(self, bx, by, cx, cy):
        pg_path_rcurve3(self, bx, by, cx, cy)

    def pg_path_rcurve4(self, bx, by, cx, cy, dx, dy):
        pg_path_rcurve4(self, bx, by, cx, cy, dx, dy)

    def pg_path_rectangle(self, ax, ay, sx, sy):
        pg_path_rectangle(self, ax, ay, sx, sy)

    def pg_path_rounded(self, ax, ay, sx, sy, rx, ry):
        pg_path_rounded(self, ax, ay, sx, sy, rx, ry)

    def pg_path_reset(self):
        pg_path_reset(self)

    def pg_path_append(self, path):
        pg_path_append(self, path)

class Paint(Structure):
    _fields_ = [('type', PaintType),
                ('cspace', ColorSpace),
                ('colors', Color * 8),
                ('stops', c_float * 8),
                ('nstops', c_uint),
                ('a', Pt),
                ('b', Pt),
                ('ra', c_float),
                ('rb', c_float)]

    def solid(color_space, u, v, w, a=1.0):
        return pg_solid(color_space, u, v, w, a)

    def linear(color_space, ax, ay, bx, by, stops=[]):
        paint = pg_linear(color_space, ax, ay, bx, by)
        paint.add_stops(stops)
        return paint

    def add_stop(self, t, u, v, w, a=1.0):
        return pg_add_stop(self, t, u, v, w, a)

    def add_stops(self, list):
        for (t, colour) in list:
            self.add_stop(t, *colour)

class State(Structure):
    _fields_ = [('ctm', TM),
                ('fill', POINTER(Paint)),
                ('stroke', POINTER(Paint)),
                ('clear', POINTER(Paint)),
                ('line_width', c_float),
                ('line_cap', LineCap),
                ('flatness', c_float),
                ('fill_rule', FillRule),
                ('gamma', c_float),
                ('clip_x', c_float),
                ('clip_y', c_float),
                ('clip_sx', c_float),
                ('clip_sy', c_float),
                ('text_pos', TextPos),
                ('underline', c_bool)]

class Pg(Structure):
    _fields_ = [('v', c_void_p),
                ('sx', c_float),
                ('sy', c_float),
                ('path', POINTER(Path)),
                ('s', State),
                ('saved', State * 16),
                ('nsaved', c_uint)]

    def free(self):
        pg_free(self)

    def create(width, height, title):
        return pg_window(width, height, title).contents

    def save(self):
        return pg_save(self)

    def restore(self):
        return pg_restore(self)

    def reset_state(self):
        return pg_reset_state(self)

    def dpi(self):
        return pg_dpi()

    def update(self):
        pg_update(self)

    def resize(self, x, y):
        pg_resize(self, x, y)

    def size(self):
        return pg_size(self)

    def subcanvas(self, x, y, sx, sy):
        return pg_subcanvas(self, x, y, sx, sy).contents

    def reset_path(self):
        pg_reset_path(self)

    def move(self, x, y):
        pg_move(self, x, y)

    def rmove(self, x, y):
        pg_rmove(self, x, y)

    def line(self, x, y):
        pg_line(self, x, y)

    def rline(self, x, y):
        pg_rline(self, x, y)

    def curve3(self, bx, by, cx, cy):
        pg_curve3(self, bx, by, cx, cy)

    def rcurve3(self, bx, by, cx, cy):
        pg_rcurve3(self, bx, by, cx, cy)

    def curve4(self, bx, by, cx, cy, dx, dy):
        pg_curve4(self, bx, by, cx, cy, dx, dy)

    def rcurve4(self, bx, by, cx, cy, dx, dy):
        pg_rcurve4(self, bx, by, cx, cy, dx, dy)

    def rectangle(self, x, y, sx, sy):
        pg_rectangle(self, x, y, sx, sy)

    def rounded(self, x, y, sx, sy, rx, ry):
        pg_rounded(self, x, y, sx, sy, rx, ry)

    def close(self):
        pg_close(self)

    def append(self, path):
        pg_append(self, path)

    def clear(self):
        pg_clear(self)

    def fill(self):
        pg_fill(self)

    def stroke(self):
        pg_stroke(self)

    def fill_stroke(self):
        pg_fill_stroke(self)

    def identity(self):
        pg_identity(self)

    def translate(self, x, y):
        pg_translate(self, x, y)

    def rotate(self, rads):
        pg_rotate(self, rads)

    def scale(self, x, y):
        pg_scale(x, y)

    def set_tm(self, tm):
        pg_set_tm(self, tm)

    def set_fill(self, paint):
        self.tmp_fill = paint
        pg_set_fill(self, self.tmp_fill)

    def set_stroke(self, paint):
        self.tmp_stroke = paint
        pg_set_stroke(self, self.tmp_stroke)

    def set_clear(self, paint):
        self.tmp_clear = paint
        pg_set_clear(self, self.tmp_clear)

    def set_line_width(self, line_width):
        pg_set_line_width(self, line_width)

    def set_line_cap(self, line_cap):
        pg_set_line_cap(self, line_cap)

    def set_flatness(self, flatness):
        pg_set_flatness(self, flatness)

    def set_fill_rule(self, fill_rule):
        pg_set_fill_rule(self, fill_rule)

    def set_clip(self, ax, ay, bx, by):
        pg_set_clip(self, ax, ay, bx, by)

    def set_text_pos(self, text_pos):
        pg_set_text_pos(self, text_pos)

    def set_underline(self, underline):
        pg_set_underline(self, underline)

    def reset_clip(self):
        pg_reset_clip(self)

    def get_tm(self):
        return pg_get_tm(self)

    def get_fill(self):
        return pg_get_fill(self)

    def get_stroke(self):
        return pg_get_stroke(self)

    def get_clear(self):
        return pg_get_clear(self)

    def get_line_width(self):
        return pg_get_line_width(self)

    def get_line_cap(self):
        return pg_get_line_cap(self)

    def get_flatness(self):
        return pg_get_flatness(self)

    def get_fill_rule(self):
        return pg_get_fill_rule(self)

    def get_clip_start(self):
        return pg_get_clip_start(self)

    def get_clip_size(self):
        return pg_get_clip_size(self)

    def get_text_pos(self):
        return pg_get_text_pos(self)

    def glyph_path(self, font, x, y, glyph):
        return pg_glyph_path(self, font, x, y, glyph)

    def string_path(self, font, x, y, text):
        if isinstance(text, str):
            text = text.encode('utf-8', 'ignore')
        return pg_string_path(self, font, x, y, text)

    def print(self, font, x, y, text):
        self.string_path(font, x, y, text)
        self.fill()


class Event(Union):
    class Any(Structure):
        _fields_ = [('type', EventType),
                    ('g', POINTER(Pg))]

    class Resized(Structure):
        _fields_ = [('type', EventType),
                    ('g', POINTER(Pg)),
                    ('width', c_int),
                    ('height', c_int)]

    class Key(Structure):
        _fields_ = [('type', EventType),
                    ('g', POINTER(Pg)),
                    ('key', Key),
                    ('mods', Mods)]

    class Mouse(Structure):
        _fields_ = [('type', EventType),
                    ('g', POINTER(Pg)),
                    ('pos', Pt),
                    ('scroll', Pt),
                    ('button', c_int),
                    ('mods', Mods)]

    class Char(Structure):
        _fields_ = [('type', EventType),
                    ('g', POINTER(Pg)),
                    ('codepoint', c_wchar)]

    _fields_ = [('any', Any),
                ('resized', Resized),
                ('key', Key),
                ('mouse', Mouse),
                ('char', Char),
                ('resized', Resized)]

class Font(Structure):
    _fields_ = [('_', c_void_p),
                ('data', c_void_p),
                ('size', c_size_t),
                ('index', c_uint)]

    def find(family, weight, is_italic):
        if isinstance(family, str):
            family = family.encode('utf-8', 'ignore')
        font = pg_find_font(family, weight, is_italic)
        if font:
            return font.contents
        return None

    def list():
        output = []
        input = pg_list_fonts()
        i = 0
        while input[i].name:
            output.append(input[i])
            i += 1
        return output

    def family_member(self, weight=None, is_italic=None):
        if not weight: weight = self.weight
        if not is_italic: is_italic = self.is_italic()

        return Font.find(self.string_prop(FontProp.FAMILY),
                         weight,
                         is_italic)

    def scale(self, sx, sy=0.0):
        return pg_scale_font(self, sx, sy)

    def get_height(self):
        return pg_font_height(self)

    def measure(self, string):
        if isinstance(string, str):
            string = string.encode('utf-8', 'ignore')
        return pg_measure_string(self, string)

    def measure_glyph(self, glyph):
        return pg_measure_glyph(self, glyph)

    def fit(self, string):
        if isinstance(string, str):
            string = string.encode('utf-8', 'ignore')
        return pg_fit_string(self, string)

    def get_glyph(self, codepoint):
        return pg_get_glyph(self, codepoint)

    def free(self):
        pg_free_font(self)

    def string_prop(self, prop):
        out = pg_font_string(self, prop)
        if out:
            return out.decode('utf-8', 'ignore')
        return None

    def number_prop(self, prop):
        return pg_font_number(self, prop)

    def int_prop(self, prop):
        return pg_font_int(self, prop)

    def family(self):
        return self.string_prop(FontProp.FAMILY)

    def weight(self):
        return self.int_prop(FontProp.WEIGHT)

    def is_bold(self):
        return self.int_prop(FontProp.WEIGHT) >= 600

    def is_italic(self):
        return self.int_prop(FontProp.IS_ITALIC)

    def is_sans_serif(self):
        return self.int_prop(FontProp.IS_SANS_SERIF)

    def is_serif(self):
        return self.int_prop(FontProp.IS_SERIF)

    def is_fixed(self):
        return self.int_prop(FontProp.IS_FIXED)


class Face(Structure):
    _fields_ = [('family', c_char_p),
                ('style', c_char_p),
                ('path', c_char_p),
                ('index', c_uint),
                ('width', c_uint),
                ('height', c_uint),
                ('is_fixed', c_bool),
                ('is_italic', c_bool),
                ('is_serif', c_bool),
                ('is_sans_serif', c_bool),
                ('style_class', c_ubyte),
                ('style_subclass', c_ubyte),
                ('panose', c_ubyte * 10)]

class Family(Structure):
    _fields_ = [('name', c_char_p),
                ('nfaces', c_uint),
                ('faces', POINTER(Face))]

fun('pg_shutdown', None)
fun('pg_opengl', POINTER(Pg), c_float, c_float)
fun('pg_subcanvas', POINTER(Pg), POINTER(Pg), c_float, c_float, c_float, c_float)
fun('pg_free', None, POINTER(Pg))
fun('pg_resize', None, POINTER(Pg), c_float, c_float)
fun('pg_size', Pt, POINTER(Pg))
fun('pg_restore', c_bool, POINTER(Pg))
fun('pg_save', c_bool, POINTER(Pg))
fun('pg_reset_state', None, POINTER(Pg))

fun('pg_reset_path', None, POINTER(Pg))
fun('pg_move', None, POINTER(Pg), c_float, c_float)
fun('pg_rmove', None, POINTER(Pg), c_float, c_float)
fun('pg_line', None, POINTER(Pg), c_float, c_float)
fun('pg_rline', None, POINTER(Pg), c_float, c_float)
fun('pg_curve3', None, POINTER(Pg), c_float, c_float, c_float, c_float)
fun('pg_curve4', None, POINTER(Pg), c_float, c_float, c_float, c_float, c_float, c_float)
fun('pg_rcurve3', None, POINTER(Pg), c_float, c_float, c_float, c_float)
fun('pg_rcurve4', None, POINTER(Pg), c_float, c_float, c_float, c_float, c_float, c_float)
fun('pg_rectangle', None, POINTER(Pg), c_float, c_float, c_float, c_float)
fun('pg_rounded', None, POINTER(Pg), c_float, c_float, c_float, c_float, c_float, c_float)
fun('pg_close', None, POINTER(Pg))
fun('pg_append', None, POINTER(Pg), POINTER(Path))

fun('pg_clear', None, POINTER(Pg))
fun('pg_fill', None, POINTER(Pg))
fun('pg_stroke', None, POINTER(Pg))
fun('pg_fill_stroke', None, POINTER(Pg))

fun('pg_identity', None, POINTER(Pg))
fun('pg_translate', None, POINTER(Pg), c_float, c_float)
fun('pg_rotate', None, POINTER(Pg), c_float)
fun('pg_scale', None, POINTER(Pg), c_float, c_float)

fun('pg_set_tm', None, POINTER(Pg), TM)
fun('pg_set_fill', None, POINTER(Pg), POINTER(Paint))
fun('pg_set_stroke', None, POINTER(Pg), POINTER(Paint))
fun('pg_set_clear', None, POINTER(Pg), POINTER(Paint))
fun('pg_set_line_width', None, POINTER(Pg), c_float)
fun('pg_set_line_cap', None, POINTER(Pg), LineCap)
fun('pg_set_flatness', None, POINTER(Pg), c_float)
fun('pg_set_fill_rule', None, POINTER(Pg), FillRule)
fun('pg_set_clip', None, POINTER(Pg), c_float, c_float, c_float, c_float)
fun('pg_set_text_pos', None, POINTER(Pg), TextPos)
fun('pg_set_underline', None, POINTER(Pg), c_bool)
fun('pg_reset_clip', None, POINTER(Pg))

fun('pg_get_tm', TM, POINTER(Pg))
fun('pg_get_fill', POINTER(Paint), POINTER(Pg))
fun('pg_get_stroke', POINTER(Paint), POINTER(Pg))
fun('pg_get_clear', POINTER(Paint), POINTER(Pg))
fun('pg_get_line_width', c_float, POINTER(Pg))
fun('pg_get_line_cap', LineCap, POINTER(Pg))
fun('pg_get_flatness', c_float, POINTER(Pg))
fun('pg_get_fill_rule', FillRule, POINTER(Pg))
fun('pg_get_clip_start', Pt, POINTER(Pg))
fun('pg_get_clip_size', Pt, POINTER(Pg))
fun('pg_get_text_pos', TextPos, POINTER(Pg))

fun('pg_init_gui', c_bool)
fun('pg_dpi', Pt)
fun('pg_update', None, POINTER(Pg))
fun('pg_root_canvas', POINTER(Pg))
fun('pg_window', POINTER(Pg), c_uint, c_uint, c_char_p)
fun('pg_wait', c_bool, POINTER(Event))
fun('pg_enqueue', c_bool, Event)

fun('pg_path', POINTER(Path))
fun('pg_path_free', None, POINTER(Path))
fun('pg_partcount', c_uint, POINTER(Path))
fun('pg_path_close', None, POINTER(Path))
fun('pg_path_curve3', None, POINTER(Path), c_float, c_float, c_float, c_float)
fun('pg_path_curve4', None, POINTER(Path), c_float, c_float, c_float, c_float, c_float, c_float)
fun('pg_path_line', None, POINTER(Path), c_float, c_float)
fun('pg_path_rline', None, POINTER(Path), c_float, c_float)
fun('pg_path_move', None, POINTER(Path), c_float, c_float)
fun('pg_path_rmove', None, POINTER(Path), c_float, c_float)
fun('pg_path_rcurve3', None, POINTER(Path), c_float, c_float, c_float, c_float)
fun('pg_path_rcurve4', None, POINTER(Path), c_float, c_float, c_float, c_float, c_float, c_float)
fun('pg_path_rectangle', None, POINTER(Path), c_float, c_float, c_float, c_float)
fun('pg_path_rounded', None, POINTER(Path), c_float, c_float, c_float, c_float, c_float, c_float)
fun('pg_path_reset', None, POINTER(Path))
fun('pg_path_append', None, POINTER(Path), POINTER(Path))

fun('pg_linear', Paint, c_int, c_float, c_float, c_float, c_float)
fun('pg_solid', Paint, c_int, c_float, c_float, c_float, c_float)
fun('pg_add_stop', None, POINTER(Paint), c_float, c_float, c_float, c_float, c_float)

fun('pg_lch_to_lab', Color, Color)
fun('pg_lab_to_xyz', Color, Color)
fun('pg_xyz_to_rgb', Color, Color)
fun('pg_gamma_correct', Color, Color, c_float)
fun('pg_convert_color_to_srgb', Color, ColorSpace, Color, c_float)


fun('pg_find_font', POINTER(Font), c_char_p, c_uint, c_bool)
fun('pg_list_fonts', POINTER(Family))
fun('pg_free_font', None, POINTER(Font))

fun('pg_scale_font', POINTER(Font), POINTER(Font), c_float, c_float)
fun('pg_font_height', c_float, POINTER(Font))
fun('pg_measure_char', c_float, POINTER(Font), c_uint)
fun('pg_measure_chars', c_float, POINTER(Font), c_char_p, c_size_t)
fun('pg_measure_glyph', c_float, POINTER(Font), c_uint)
fun('pg_measure_string', c_float, POINTER(Font), c_char_p)
fun('pg_fit_chars', c_uint, POINTER(Font), c_char_p, c_size_t, c_float)
fun('pg_fit_string', c_uint, POINTER(Font), c_char_p, c_float)

fun('pg_get_glyph', c_uint, POINTER(Font), c_uint)
fun('pg_char_path', c_float, POINTER(Pg), POINTER(Font), c_float, c_float, c_uint)
fun('pg_chars_path', c_float, POINTER(Pg), POINTER(Font), c_float, c_float, c_char_p, c_size_t)
fun('pg_glyph_path', c_float, POINTER(Pg), POINTER(Font), c_float, c_float, c_uint)
fun('pg_string_path', c_float, POINTER(Pg), POINTER(Font), c_float, c_float, c_char_p)

fun('pg_font_string', c_char_p, POINTER(Font), FontProp)
fun('pg_font_number', c_float, POINTER(Font), FontProp)
fun('pg_font_int', c_int, POINTER(Font), FontProp)

fun('pg_apply_tm', TM, TM, Pt)
fun('pg_mul_tm', TM, TM, TM)
fun('pg_ident_tm', TM)
fun('pg_translate_tm', TM, c_float, c_float)
fun('pg_scale_tm', TM, c_float, c_float)
fun('pg_rotate_tm', TM, c_float)

pg_init_gui()

def wait(evt):
    if not evt:
        evt = Event()
    pg_wait(byref(evt))
    return evt


Paint.white = Paint.solid(ColorSpace.LCHAB, 1.0, 0.0, 0.0)
Paint.black = Paint.solid(ColorSpace.LCHAB, 0.0, 0.0, 0.0)
Paint.grey = Paint.solid(ColorSpace.LCHAB, 0.5, 0.0, 0.0)
Paint.gray = Paint.grey
Paint.blue = Paint.solid(ColorSpace.LCHAB, 0.125, 0.5, 0.75)
Paint.red = Paint.solid(ColorSpace.LCHAB, 0.125, 0.5, 0.05)
Paint.none = Paint.solid(ColorSpace.LCHAB, 0.0, 0.0, 0.0, 0.0)
