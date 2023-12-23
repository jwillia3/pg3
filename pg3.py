# TODO:
# Fix functions that send or return PgPt or PgTM

import sys
from ctypes import cdll, c_bool, c_int, c_uint, c_float, c_char_p, c_void_p, c_size_t, Structure, Union, POINTER, pointer

pg3 = cdll.LoadLibrary('./libpg3.so')
module = sys.modules[__name__]

def func(function_name, ret, **args):
    f = pg3[function_name]
    f.argtypes = list(args.values())
    f.restype = ret
    setattr(module, function_name, f)

def enum(name, *items):
    i = 0
    setattr(module, name, c_int)
    for name in items:
        setattr(module, name, i)
        i += 1

Pg = c_void_p

class PgPt(Structure):
    _fields_ = [('x', c_float),
                ('y', c_float)]

#
# Windows
#
PgWindow = c_void_p
PgWindowEvent = c_void_p

enum('PgWindowEventType',
    'PG_EVENT_IGNORE',
    'PG_EVENT_PAINT',
    'PG_EVENT_RESIZED',
    'PG_EVENT_CLOSED',
    'PG_EVENT_KEY_DOWN',
    'PG_EVENT_KEY_UP',
    'PG_EVENT_TEXT',
    'PG_EVENT_MOUSE_DOWN',
    'PG_EVENT_MOUSE_UP',
    'PG_EVENT_MOUSE_MOVED',
    'PG_EVENT_MOUSE_WHEEL',
    'PG_EVENT_USER')

func("pg_window_open", PgWindow, width=c_uint, height=c_uint, title=c_char_p)
func('pg_window_free', None, win=PgWindow)
func('pg_window_close', None, win=PgWindow)
func('pg_window_get_dpi', PgPt, win=PgWindow)
func('pg_window_get_dpi_x', c_float, win=PgWindow)
func('pg_window_get_dpi_y', c_float, win=PgWindow)

func('pg_window_get_canvas', Pg, win=PgWindow)
func('pg_window_get_size', PgPt, win=PgWindow)
func('pg_window_get_width', c_float, win=PgWindow)
func('pg_window_get_height', c_float, win=PgWindow)
func('pg_window_update', None, win=PgWindow)
func('pg_window_queue_update', None, win=PgWindow)
func('pg_window_event_wait', c_void_p)

func('pg_window_event_get_type', PgWindowEventType, e=PgWindowEvent)
func('pg_window_event_get_window', PgWindow, e=PgWindowEvent)
func('pg_window_event_get_key', c_char_p, e=PgWindowEvent)
func('pg_window_event_get_text', c_char_p, e=PgWindowEvent)
func('pg_window_event_get_mouse_pt', PgPt, e=PgWindowEvent)
func('pg_window_event_get_mouse_x', c_float, e=PgWindowEvent)
func('pg_window_event_get_mouse_y', c_float, e=PgWindowEvent)
func('pg_window_event_get_mouse_wheel', c_float, e=PgWindowEvent)
func('pg_window_event_get_mouse_button', c_char_p, e=PgWindowEvent)
func('pg_window_event_get_resized_width', c_float, e=PgWindowEvent)
func('pg_window_event_get_resized_height', c_float, e=PgWindowEvent)

PgPaint = c_void_p
class PgColor(Structure):
    _fields_ = [('u', c_float),
                ('v', c_float),
                ('w', c_float),
                ('a', c_float)]

enum('PgColorSpace',
    'PG_LINEAR_RGB',
    'PG_LCHAB',
    'PG_LAB',
    'PG_XYZ',
)
enum('PgPaintType',
    'PG_SOLID_PAINT',
    'PG_LINEAR_PAINT',
)
enum('PgColorSpace',
    'PG_LINEAR_RGB',
    'PG_LCHAB',
    'PG_LAB',
    'PG_XYZ',
)

func('pg_paint_from_name', PgPaint, name=c_char_p)

func('pg_paint_new_linear', PgPaint, cspace=PgColorSpace, ax=c_float, ay=c_float, bx=c_float, by=c_float)
func('pg_paint_new_solid', PgPaint, cspace=PgColorSpace, u=c_float, v=c_float, w=c_float, a=c_float)
func('pg_paint_add_stop', None, paint=PgPaint, t=c_float, u=c_float, v=c_float, w=c_float, a=c_float)
func('pg_paint_clear_stops', None, paint=PgPaint)

func('pg_paint_clone', PgPaint, paint=PgPaint)
func('pg_paint_free', None, paint=PgPaint)

func('pg_paint_get_type', PgPaintType, paint=PgPaint)
func('pg_paint_get_colorspace', PgColorSpace, paint=PgPaint)
func('pg_paint_get_nstops', c_uint, paint=PgPaint)
func('pg_paint_get_color', PgColor, paint=PgPaint, n=c_uint)
func('pg_paint_get_stop', c_float, paint=PgPaint, n=c_uint)
func('pg_paint_get_a', PgPt, paint=PgPaint)
func('pg_paint_get_b', PgPt, paint=PgPaint)
func('pg_paint_get_ra', c_float, paint=PgPaint)
func('pg_paint_get_rb', c_float, paint=PgPaint)


PgFont = c_void_p
PgFamily = c_void_p
PgFace = c_void_p
enum('PgFontProp',
    'PG_FONT_FORMAT',
    'PG_FONT_INDEX',
    'PG_FONT_NFONTS',
    'PG_FONT_FAMILY',
    'PG_FONT_STYLE',
    'PG_FONT_FULL_NAME',
    'PG_FONT_IS_FIXED',
    'PG_FONT_IS_ITALIC',
    'PG_FONT_IS_SANS_SERIF',
    'PG_FONT_IS_SERIF',
    'PG_FONT_WEIGHT',
    'PG_FONT_WIDTH_CLASS',
    'PG_FONT_STYLE_CLASS',
    'PG_FONT_STYLE_SUBCLASS',
    'PG_FONT_ANGLE',
    'PG_FONT_PANOSE_1',
    'PG_FONT_PANOSE_2',
    'PG_FONT_PANOSE_3',
    'PG_FONT_PANOSE_4',
    'PG_FONT_PANOSE_5',
    'PG_FONT_PANOSE_6',
    'PG_FONT_PANOSE_7',
    'PG_FONT_PANOSE_8',
    'PG_FONT_PANOSE_9',
    'PG_FONT_PANOSE_10',
    'PG_FONT_NGLYPHS',
    'PG_FONT_UNITS',
    'PG_FONT_EM',
    'PG_FONT_AVG_WIDTH',
    'PG_FONT_ASCENDER',
    'PG_FONT_DESCENDER',
    'PG_FONT_LINEGAP',
    'PG_FONT_XHEIGHT',
    'PG_FONT_CAPHEIGHT',
    'PG_FONT_UNDERLINE',
    'PG_FONT_UNDERLINE_SIZE',
    'PG_FONT_SUB_X',
    'PG_FONT_SUB_Y',
    'PG_FONT_SUB_SX',
    'PG_FONT_SUB_SY',
    'PG_FONT_SUP_X',
    'PG_FONT_SUP_Y',
    'PG_FONT_SUP_SX',
    'PG_FONT_SUP_SY',
)

func('pg_font_find', PgFont, family=c_char_p, weight=c_uint, italic=c_bool)
func('pg_font_clone', PgFont, font=PgFont)

func('pg_font_free', None, font=PgFont)

func('pg_font_scale', PgFont, font=PgFont, sx=c_float, sy=c_float)

func('pg_font_get_height', c_float, font=PgFont)
func('pg_font_measure_char', c_float, font=PgFont, codepoint=c_uint)
func('pg_font_measure_chars', c_float, font=PgFont, str=c_char_p, nbytes=c_size_t)
func('pg_font_measure_glyph', c_float, font=PgFont, glyph=c_uint)
func('pg_font_measure_string', c_float, font=PgFont, str=c_char_p)
func('pg_font_fit_chars', c_uint, font=PgFont, s=c_char_p, nbytes=c_size_t, width=c_float)
func('pg_font_fit_string', c_uint, font=PgFont, str=c_char_p, width=c_float)

func('pg_font_char_to_glyph', c_uint, font=PgFont, codepoint=c_uint)

func('pg_font_get_scale', PgPt, font=PgFont)
func('pg_font_get_em', PgPt, font=PgFont)
func('pg_font_prop_string', c_char_p, font=PgFont, id=PgFontProp)
func('pg_font_prop_float', c_float, font=PgFont, id=PgFontProp)
func('pg_font_prop_int', c_int, font=PgFont, id=PgFontProp)

func('pg_font_from_data', PgFont, path=c_char_p, data=c_void_p, size=c_size_t, index=c_uint)
func('pg_font_from_file', PgFont, path=c_char_p, index=c_uint)
func('pg_font_from_data_otf', PgFont, data=c_void_p, size=c_size_t, index=c_uint)

func('pg_font_list', PgFamily)
func('pg_font_list_get_count', c_uint)
func('pg_font_list_get_family', PgFamily, n=c_uint);
func('pg_font_family_get_name', c_char_p, family=PgFamily);
func('pg_font_family_get_face', PgFace, family=PgFamily, n=c_uint);
func('pg_font_family_get_face_count', c_uint, family=PgFamily);
func('pg_font_face_get_family', c_char_p, face=PgFace);
func('pg_font_face_get_style', c_char_p, face=PgFace);
func('pg_font_face_get_path', c_char_p, face=PgFace);
func('pg_font_face_get_index', c_uint, face=PgFace);
func('pg_font_face_get_width_class', c_uint, face=PgFace);
func('pg_font_face_get_weight', c_uint, face=PgFace);
func('pg_font_face_get_is_fixed', c_bool, face=PgFace);
func('pg_font_face_get_is_italic', c_bool, face=PgFace);
func('pg_font_face_get_is_serif', c_bool, face=PgFace);
func('pg_font_face_get_is_sans_serif', c_bool, face=PgFace);
func('pg_font_face_get_style_class', c_uint, face=PgFace);
func('pg_font_face_get_style_subclass', c_uint, face=PgFace);


PgPath = c_void_p

#
#
#
class PgTM(Structure):
    _fields_ = [('a', c_float),
                ('b', c_float),
                ('c', c_float),
                ('d', c_float),
                ('e', c_float),
                ('f', c_float),
                ]

enum('PgLineCap',
    'PG_BUTT_CAP',
    'PG_SQUARE_CAP',
)

enum('PgFillRule',
    'PG_NONZERO_RULE',
    'PG_EVEN_ODD_RULE',
)

func('pg_canvas_new_opengl', Pg, width=c_uint, height=c_uint)
func('pg_canvas_new_subcanvas', Pg, parent=Pg, x=c_float, y=c_float, sx=c_float, sy=c_float)

func('pg_canvas_free', None, g=Pg)

func('pg_canvas_clear', None, g=Pg)
func('pg_canvas_fill', None, g=Pg)
func('pg_canvas_stroke', None, g=Pg)
func('pg_canvas_fill_stroke', None, g=Pg)
func('pg_canvas_commit', None, g=Pg)

func('pg_canvas_show_char', c_float, g=Pg, font=PgFont, x=c_float, y=c_float, codepoint=c_uint)
func('pg_canvas_show_chars', c_float, g=Pg, font=PgFont, x=c_float, y=c_float, str=c_char_p, nbytes=c_size_t)
func('pg_canvas_show_string', c_float, g=Pg, font=PgFont, x=c_float, y=c_float, str=c_char_p)
func('pg_canvas_show_glyph', c_float, g=Pg, font=PgFont, x=c_float, y=c_float, glyph=c_uint)

func('pg_canvas_trace_char', c_float, g=Pg, font=PgFont, x=c_float, y=c_float, codepoint=c_uint)
func('pg_canvas_trace_chars', c_float, g=Pg, font=PgFont, x=c_float, y=c_float, str=c_char_p, nbytes=c_size_t)
func('pg_canvas_trace_string', c_float, g=Pg, font=PgFont, x=c_float, y=c_float, str=c_char_p)
func('pg_canvas_trace_glyph', c_float, g=Pg, font=PgFont, x=c_float, y=c_float, glyph=c_uint)

func('pg_canvas_path_clear', None, g=Pg)
func('pg_canvas_move', None, g=Pg, x=c_float, y=c_float)
func('pg_canvas_rmove', None, g=Pg, x=c_float, y=c_float)
func('pg_canvas_line', None, g=Pg, x=c_float, y=c_float)
func('pg_canvas_rline', None, g=Pg, x=c_float, y=c_float)
func('pg_canvas_curve3', None, g=Pg, bx=c_float, by=c_float, cx=c_float, cy=c_float)
func('pg_canvas_rcurve3', None, g=Pg, bx=c_float, by=c_float, cx=c_float, cy=c_float)
func('pg_canvas_curve4', None, g=Pg, bx=c_float, by=c_float, cx=c_float, cy=c_float, dx=c_float, dy=c_float)
func('pg_canvas_rcurve4', None, g=Pg, bx=c_float, by=c_float, cx=c_float, cy=c_float, dx=c_float, dy=c_float)
func('pg_canvas_rectangle', None, g=Pg, x=c_float, y=c_float, sx=c_float, sy=c_float)
func('pg_canvas_rounded_rectangle', None, g=Pg, x=c_float, y=c_float, sx=c_float, sy=c_float, r=c_float)
func('pg_canvas_close_path', None, g=Pg)
func('pg_canvas_append_path', None, g=Pg, src=PgPath)

func('pg_canvas_get_size', PgPt, g=Pg)
func('pg_canvas_get_width', c_float, g=Pg)
func('pg_canvas_get_height', c_float, g=Pg)
func('pg_canvas_set_size', None, g=Pg, width=c_float, height=c_float)

func('pg_canvas_state_restore', c_bool, g=Pg)
func('pg_canvas_state_save', c_bool, g=Pg)
func('pg_canvas_state_reset', None, g=Pg)

func('pg_canvas_identity', None, g=Pg)
func('pg_canvas_translate', None, g=Pg, x=c_float, y=c_float)
func('pg_canvas_rotate', None, g=Pg, rads=c_float)
func('pg_canvas_scale', None, g=Pg, x=c_float, y=c_float)

func('pg_canvas_set_mat', None, g=Pg, tm=PgTM)
func('pg_canvas_set_fill', None, g=Pg, paint=PgPaint)
func('pg_canvas_set_stroke', None, g=Pg, paint=PgPaint)
func('pg_canvas_set_clear', None, g=Pg, paint=PgPaint)
func('pg_canvas_set_line_width', None, g=Pg, line_width=c_float)
func('pg_canvas_set_line_cap', None, g=Pg, line_cap=PgLineCap)
func('pg_canvas_set_flatness', None, g=Pg, flatness=c_float)
func('pg_canvas_set_gamma', None, g=Pg, gamma=c_float)
func('pg_canvas_set_fill_rule', None, g=Pg, fill_rule=PgFillRule)
func('pg_canvas_set_scissor', None, g=Pg, x=c_float, y=c_float, sx=c_float, sy=c_float)
func('pg_canvas_set_underline', None, g=Pg, underline=c_bool)
func('pg_canvas_scissor_reset', None, g=Pg)

func('pg_canvas_get_mat', PgTM, g=Pg)
func('pg_canvas_get_fill', PgPaint, g=Pg)
func('pg_canvas_get_stroke', PgPaint, g=Pg)
func('pg_canvas_get_clear', PgPaint, g=Pg)
func('pg_canvas_get_line_width', c_float, g=Pg)
func('pg_canvas_get_line_cap', PgLineCap, g=Pg)
func('pg_canvas_get_flatness', c_float, g=Pg)
func('pg_canvas_get_gamma', c_float, g=Pg)
func('pg_canvas_get_fill_rule', PgFillRule, g=Pg)
func('pg_canvas_get_scissor_start', PgPt, g=Pg)
func('pg_canvas_get_scissor_size', PgPt, g=Pg)
func('pg_canvas_get_scissor_start_x', c_float, g=Pg)
func('pg_canvas_get_scissor_start_y', c_float, g=Pg)
func('pg_canvas_get_scissor_size_x', c_float, g=Pg)
func('pg_canvas_get_scissor_size_y', c_float, g=Pg)
func('pg_canvas_get_underline', c_bool, g=Pg)

# func('pg_mat_apply', PgPt, ctm=PgTM, p=PgPt)
func('pg_mat_multiply', PgTM, x=PgTM, y=PgTM)
func('pg_mat_identity', PgTM)
func('pg_mat_translate', PgTM, m=PgTM, x=c_float, y=c_float)
func('pg_mat_scale', PgTM, m=PgTM, x=c_float, y=c_float)
func('pg_mat_rotate', PgTM, m=PgTM, rad=c_float)


class Window:
    "Window."
    all_windows = {}

    class Event:
        "A window event."

        IGNORE = PG_EVENT_IGNORE
        PAINT = PG_EVENT_PAINT
        RESIZED = PG_EVENT_RESIZED
        CLOSED = PG_EVENT_CLOSED
        KEY_DOWN = PG_EVENT_KEY_DOWN
        KEY_UP = PG_EVENT_KEY_UP
        TEXT = PG_EVENT_TEXT
        MOUSE_DOWN = PG_EVENT_MOUSE_DOWN
        MOUSE_UP = PG_EVENT_MOUSE_UP
        MOUSE_MOVED = PG_EVENT_MOUSE_MOVED
        MOUSE_WHEEL = PG_EVENT_MOUSE_WHEEL
        USER = PG_EVENT_USER

        def __init__(self, native):
            self.native = native
            self.type = pg_window_event_get_type(self.native)
            native_window = pg_window_event_get_window(self.native)
            self.window = Window.from_native(native_window)

        def from_native(native):
            return Window.Event(native) if native is not None else None

        @property
        def key(self):
            if s := pg_window_event_get_key(self.native):
                return str(s, 'utf8')

        @property
        def text(self):
            if s := pg_window_event_get_text(self.native):
                return str(s, 'utf8')

        @property
        def mouse_point(self):
            return (pg_window_event_get_mouse_x(self.native),
                    pg_window_event_get_mouse_y(self.native))

        @property
        def mouse_wheel(self):
            return pg_window_event_get_mouse_wheel(self.native)

        @property
        def mouse_button(self):
            if s := pg_window_event_get_mouse_button(self.native):
                return str(s, 'utf8')

        @property
        def size(self):
            "Get size from resize event."
            return (pg_window_event_get_resized_width(self.native),
                    pg_window_event_get_resized_height(self.native))


    def __init__(self, width=0, height=0, title=''):
        self.native = pg_window_open(width, height, bytes(title, 'utf8'))
        Window.all_windows[self.native] = self

    def from_native(native):
        "Get wrapper window from native window handle."
        if native is None:
            return None
        return Window.all_windows.get(native)

    @property
    def width(self):
        return pg_window_get_width(self.native)

    @property
    def height(self):
        return pg_window_get_height(self.native)

    @property
    def size(self):
        return (self.width, self.height)

    @property
    def dpi(self):
        return (pg_window_get_dpi_x(self.native),
                pg_window_get_dpi_y(self.native))

    @property
    def canvas(self):
        return Canvas.from_native(pg_window_get_canvas(self.native))

    def close(self):
        "Close window."
        pg_window_close(self.native)

    def update(self):
        "Notify window system this window needs to be updated."
        pg_window_update(self.native)

    def queue_update(self):
        "Notify the window system to send an update event."
        pg_window_queue_update(self.native)

    def wait_event():
        "Wait for an event."
        e = pg_window_event_wait()
        return Window.Event.from_native(e)


class Canvas:
    "Drawing canvas."

    def __init__(self, native):
        self.native = native

    def from_native(native):
        return Canvas(native) if native else None

    @property
    def width(self):
        return pg_canvas_get_width(self.native)

    @property
    def height(self):
        return pg_canvas_get_height(self.native)

    @property
    def size(self):
        return (self.width, self.height)

    def subcanvas(self, x, y, sx, sy):
        return Canvas.from_native(pg_canvas_new_subcanvas(self.native, x, y, sx, sy))

    def free(self):
        pg_canvas_free(self.native)

    def clear(self):
        pg_canvas_clear(self.native)

    def fill(self):
        "Fill path with fill paint and clear path."
        pg_canvas_fill(self.native)

    def stroke(self):
        "Stroke path with stroke paint and clear path."
        pg_canvas_stroke(self.native)

    def fill_stroke(self):
        "Stroke, fill, and clear path."
        pg_canvas_fill_stroke(self.native)

    def commit(self):
        "Commit canvas changes."
        pg_canvas_commit(self.native)

    def show(self, font, x, y, string, nbytes=None):
        "Show string. Return x position after string."
        if nbytes is None:
            nbytes = len(string)
        if nbytes < 0:
            nbytes += len(string)
        if nbytes < 0:
            return 0
        if nbytes > len(string):
            nbytes = len(string)

        return pg_canvas_show_chars(self.native, font.native, x, y,
                                    bytes(string, 'utf8'),
                                    nbytes)

    def trace(self, font, x, y, string, nbytes=None):
        "Trace string outline. Return x position after string."
        if nbytes is None:
            nbytes = len(string)
        if nbytes < 0:
            nbytes += len(string)
        if nbytes < 0:
            return 0
        if nbytes > len(string):
            nbytes = len(string)

        return pg_canvas_trace_chars(self.native, font.native, x, y,
                                     bytes(string, 'utf8'),
                                     nbytes)

    def path_clear(self):
        "Clear path."
        pg_canvas_path_clear(self.native)

    def move(self, x, y):
        "Move path to."
        pg_canvas_move(self.native, x, y)

    def rmove(self, x, y):
        "Move path relative."
        pg_canvas_rmove(self.native, x, y)

    def line(self, x, y):
        "Add line to path."
        pg_canvas_line(self.native, x, y)

    def rline(self, x, y):
        "Add relative line to path."
        pg_canvas_rline(self.native, x, y)

    def curve3(self, bx, by):
        "Add quadratic curve to path."
        pg_canvas_curve3(self.native, bx, by)

    def rcurve3(self, bx, by):
        "Add relative quadratic curve to path."
        pg_canvas_rcurve3(self.native, bx, by)

    def curve4(self, bx, by, cx, cy, dx, dy):
        "Add cubic curve to path."
        pg_canvas_curve4(self.native, bx, by, cx, cy, dx, dy)

    def rcurve4(self, bx, by, cx, cy, dx, dy):
        "Add relative cubic curve to path."
        pg_canvas_rcurve4(self.native, bx, by, cx, cy, dx, dy)

    def rectangle(self, x, y, sx, sy):
        "Add a clockwise rectangle to path."
        pg_canvas_rectangle(self.native, x, y, sx, sy)

    def rounded_rectangle(self, x, y, sx, sy, r):
        "Add a clockwise rounded rectangle to path."
        pg_canvas_rounded_rectangle(self.native, x, y, sx, sy, r)

    def close_path(self):
        "Close path."
        pg_canvas_close_path(self.native)

    def append_path(self, path):
        "Append path."
        pg_canvas_append_path(self.native, path)

    def set_size(self, x, y):
        "Set canvas size."
        pg_canvas_set_size(self.native, x, y)

    def restore_state(self):
        "Restore last saved graphics state."
        return pg_canvas_state_restore(self.native)

    def save_state(self):
        "Save current graphics state."
        return pg_canvas_state_save(self.native)

    def reset_state(self):
        "Reset graphics state."
        return pg_canvas_state_reset(self.native)

    def identity(self):
        "Set the current transform matrix to identity."
        return pg_canvas_identity(self.native)

    def translate(self, x, y):
        "Translate current transform matrix."
        return pg_canvas_translate(self.native, x, y)

    def rotate(self, radians):
        "Rotate current transform matrix."
        return pg_canvas_rotate(self.native, radians)

    def scale(self, x, y):
        "Scale current transform matrix."
        return pg_canvas_scale(self.native, x, y)

    def set_fill(self, paint):
        "Set fill paint."
        return pg_canvas_set_fill(self.native, paint.native)

    def set_stroke(self, paint):
        "Set stroke paint."
        return pg_canvas_set_stroke(self.native, paint.native)

    def set_clear(self, paint):
        "Set clear paint."
        return pg_canvas_set_clear(self.native, paint.native)

    def set_line_width(self, line_width):
        "Set line width."
        return pg_canvas_set_line_width(self.native, line_width)

    def set_line_cap(self, line_cap):
        "Set line cap."
        return pg_canvas_set_line_cap(self.native, line_cap)

    def set_flatness(self, flatness):
        "Set flatness."
        return pg_canvas_set_flatness(self.native, flatness)

    def set_gamma(self, gamma):
        "Set gamma."
        return pg_canvas_set_gamma(self.native, gamma)

    def set_fill_rule(self, fill_rule):
        "Set fill rule."
        return pg_canvas_set_fill_rule(self.native, fill_rule)

    def set_scissor(self, x, y, sx, sy):
        "Set scissor."
        return pg_canvas_set_scissor(self.native, x, y, sx, sy)

    def set_underline(self, underline):
        "Set underline."
        return pg_canvas_set_underline(self.native, underline)

    def set_underline(self, underline):
        "Set underline."
        return pg_canvas_set_underline(self.native, underline)

    def reset_scissor(self):
        "Reset scissor."
        return pg_canvas_scissor_reset(self.native)

    def get_fill(self):
        return Paint.from_native(pg_canvas_get_fill(self.native))

    def get_stroke(self):
        return Paint.from_native(pg_canvas_get_stroke(self.native))

    def get_clear(self):
        return Paint.from_native(pg_canvas_get_clear(self.native))

    def get_line_width(self):
        return pg_canvas_get_line_width(self.native)

    def get_line_cap(self):
        return pg_canvas_get_line_cap(self.native)

    def get_flatness(self):
        return pg_canvas_get_flatness(self.native)

    def get_gamma(self):
        return pg_canvas_get_gamma(self.native)

    def get_fill_rule(self):
        return pg_canvas_get_fill_rule(self.native)

    def get_scissor_start(self):
        return (pg_canvas_get_scissor_start_x(self.native),
                pg_canvas_get_scissor_start_y(self.native))

    def get_scissor_size(self):
        return (pg_canvas_get_scissor_size_x(self.native),
                pg_canvas_get_scissor_size_y(self.native))

    def get_underline(self):
        return pg_canvas_get_underline(self.native)



class Font:
    "Font."

    PROP_FORMAT = PG_FONT_FORMAT
    PROP_INDEX = PG_FONT_INDEX
    PROP_NFONTS = PG_FONT_NFONTS
    PROP_FAMILY = PG_FONT_FAMILY
    PROP_STYLE = PG_FONT_STYLE
    PROP_FULL_NAME = PG_FONT_FULL_NAME
    PROP_IS_FIXED = PG_FONT_IS_FIXED
    PROP_IS_ITALIC = PG_FONT_IS_ITALIC
    PROP_IS_SANS_SERIF = PG_FONT_IS_SANS_SERIF
    PROP_IS_SERIF = PG_FONT_IS_SERIF
    PROP_WEIGHT = PG_FONT_WEIGHT
    PROP_WIDTH_CLASS = PG_FONT_WIDTH_CLASS
    PROP_STYLE_CLASS = PG_FONT_STYLE_CLASS
    PROP_STYLE_SUBCLASS = PG_FONT_STYLE_SUBCLASS
    PROP_ANGLE = PG_FONT_ANGLE
    PROP_PANOSE_1 = PG_FONT_PANOSE_1
    PROP_PANOSE_2 = PG_FONT_PANOSE_2
    PROP_PANOSE_3 = PG_FONT_PANOSE_3
    PROP_PANOSE_4 = PG_FONT_PANOSE_4
    PROP_PANOSE_5 = PG_FONT_PANOSE_5
    PROP_PANOSE_6 = PG_FONT_PANOSE_6
    PROP_PANOSE_7 = PG_FONT_PANOSE_7
    PROP_PANOSE_8 = PG_FONT_PANOSE_8
    PROP_PANOSE_9 = PG_FONT_PANOSE_9
    PROP_PANOSE_10 = PG_FONT_PANOSE_10
    PROP_NGLYPHS = PG_FONT_NGLYPHS
    PROP_UNITS = PG_FONT_UNITS
    PROP_EM = PG_FONT_EM
    PROP_AVG_WIDTH = PG_FONT_AVG_WIDTH
    PROP_ASCENDER = PG_FONT_ASCENDER
    PROP_DESCENDER = PG_FONT_DESCENDER
    PROP_LINEGAP = PG_FONT_LINEGAP
    PROP_XHEIGHT = PG_FONT_XHEIGHT
    PROP_CAPHEIGHT = PG_FONT_CAPHEIGHT
    PROP_UNDERLINE = PG_FONT_UNDERLINE
    PROP_UNDERLINE_SIZE = PG_FONT_UNDERLINE_SIZE
    PROP_SUB_X = PG_FONT_SUB_X
    PROP_SUB_Y = PG_FONT_SUB_Y
    PROP_SUB_SX = PG_FONT_SUB_SX
    PROP_SUB_SY = PG_FONT_SUB_SY
    PROP_SUP_X = PG_FONT_SUP_X
    PROP_SUP_Y = PG_FONT_SUP_Y
    PROP_SUP_SX = PG_FONT_SUP_SX
    PROP_SUP_SY = PG_FONT_SUP_SY

    class Family:
        def __init__(self, native):
            self.native = native

        def from_native(native):
            return Font.Family(native) if native is not None else None

        @property
        def name(self):
            return str(pg_font_family_get_name(self.native), 'utf8')

        @property
        def faces(self):
            n = pg_font_family_get_face_count(self.native)
            list = []
            for i in range(n):
                native = pg_font_family_get_face(self.native, i)
                list.append(Font.Face.from_native(native))
            return list

        def list():
            n = pg_font_list_get_count()
            list = []
            for i in range(n):
                native = pg_font_list_get_family(i)
                list.append(Font.Family.from_native(native))
            return list

    class Face:
        def __init__(self, native):
            self.native = native

        def from_native(native):
            return Font.Face(native) if native is not None else None

        @property
        def family(self):
            return str(pg_font_face_get_family(self.native), 'utf8')

        @property
        def style(self):
            return str(pg_font_face_get_style(self.native), 'utf8')

        @property
        def path(self):
            return str(pg_font_face_get_path(self.native), 'utf8')

        @property
        def index(self):
            return pg_font_face_get_index(self.native)

        @property
        def width_class(self):
            return pg_font_face_get_width_class(self.native)

        @property
        def weight(self):
            return pg_font_face_get_weight(self.native)

        @property
        def is_fixed(self):
            return pg_font_face_get_is_fixed(self.native)

        @property
        def is_italic(self):
            return pg_font_face_get_is_italic(self.native)

        @property
        def is_serif(self):
            return pg_font_face_get_is_serif(self.native)

        @property
        def is_sans_serif(self):
            return pg_font_face_get_is_sans_serif(self.native)

        @property
        def style_class(self):
            return pg_font_face_get_style_class(self.native)

        @property
        def style_subclass(self):
            return pg_font_face_get_style_subclass(self.native)


    def __init__(self, native):
        self.native = native

    @property
    def height(self):
        return pg_font_get_height(self.native)

    @property
    def em(self):
        return pg_font_get_em(self.native)

    def find(family, weight=400, italic=False):
        "Find font with the given family, weight, and italics."
        if native := pg_font_find(bytes(family, 'utf8'), weight, italic):
            return Font(native)

    def clone(self):
        "Clone font."
        if clone_native := pg_font_clone(self.native):
            return Font(clone_native)

    def free(self):
        "Free font."
        pg_font_free(self.native)
        self.native = None

    def scale(self, sx, sy=None):
        "Scale font to (sx, sy) pixels."
        if sy is None:
            sy = sx
        pg_font_scale(self.native, sx, sy)
        return self

    def measure(self, string, nbytes=None):
        "Measure the width of a given string."
        if nbytes is None:
            nbytes = len(string)
        if nbytes < 0:
            nbytes += len(string)
        if nbytes < 0:
            return 0
        if nbytes > len(string):
            nbytes = len(string)
        return pg_font_measure_chars(self.native, bytes(string, 'utf8'), nbytes)

    def fit(self, width, string, nbytes=None):
        "Determine how many characters from the string will fit in this width."
        if nbytes is None:
            nbytes = len(string)
        if nbytes < 0:
            nbytes += len(string)
        if nbytes < 0:
            return 0
        if nbytes > len(string):
            nbytes = len(string)
        return pg_font_fit_chars(self.native, bytes(string, 'utf8'), nbytes, width)

    def char_to_glyph(self, codepoint):
        "Get glyph number for codepoint."
        return pg_font_char_to_glyph(self.native, codepoint)

    def get_string_prop(self, id):
        "Get font property as string."
        if s := pg_font_prop_string(self.native, id):
            return str(s, 'utf8')

    def get_number_prop(self, id):
        "Get font property as number."
        return pg_font_prop_float(self.native, id)

class Paint:
    # Type
    SOLID = PG_SOLID_PAINT
    LINEAR = PG_LINEAR_PAINT

    # Color space.
    LINEAR_RGB = PG_LINEAR_RGB
    LCHAB = PG_LCHAB
    LAB = PG_LAB
    XYZ = PG_XYZ

    def __init__(self, native):
        self.native = native
        return None

    def from_native(native):
        return Paint(native) if native else None

    def from_name(name):
        "Find solid paint from CSS named colours."
        return Paint.from_native(pg_paint_from_name(bytes(name, 'utf8')))

    def solid(cspace, u, v, w, a=1.0):
        return Paint.from_native(pg_paint_new_solid(cspace, u, v, w, a))

    def solid_lchab(l, c, h, a=1.0):
        return Paint.from_native(pg_paint_new_solid(Paint.LCHAB, l, c, h, a))

    def clone(self):
        "Clone paint."
        return Paint.from_native(pg_paint_clone(self.native))

    def free(self):
        "Free paint."
        pg_paint_free(self.native)
        self.native = None

