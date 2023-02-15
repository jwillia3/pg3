import sys
import pg

Pg = pg.Pg
Pt = pg.Pt
Paint = pg.Paint
Font = pg.Font
ColorSpace = pg.ColorSpace

ui_font = Font.find("system-ui", 500, False)
bold_font = ui_font.family_member(weight=600)
if not ui_font or not bold_font:
    exit('Could not open system-ui font')
ui_font.scale(11.0 * pg.pg_dpi().y / 72.0)
bold_font.scale(11.0 * pg.pg_dpi().y / 72.0)

class UIStyle:
    global ui_font, bold_font
    font = ui_font
    bold_font = bold_font
    fg = Paint.solid(ColorSpace.LCHAB, 0.125, 0.0, 0.0)
    border_color = fg
    face_paint = Paint.solid(ColorSpace.LCHAB, 0.8, 0.0, 0.0)
    pressed_face_paint = Paint.solid(ColorSpace.LCHAB, 0.5, 0.9, 0.775)
    hovered_face_paint = Paint.solid(ColorSpace.LCHAB, 0.8, 0.9, 0.775)

class Box:
    """
    Box organisation model.

    Properties:
    x, y:                   location of box in parent
    cx, cy:                 content size
    sx, sy:                 size of content including padding
    pad_x, margin_y:        padding size (included in sx, sy)
    margin_x, margin_y:     margin size (not included in sx, sy)
    border:                 border size (not included in sx, sy)
    on_axis:                'normal'|'fill'|'50%'
                            'normal' stacks content as normal (default).
                            'fill' expands to use the remaining space
                            on the major axis. If there are multiple
                            siblings with on_axis:'fill', the space
                            is divided up evenly between them.
                            '50%' (any percent value) reserves that
                            percentage of the remaining space. Using
                            a percent does not share.
    off_axis:               'start'|'center'|'end'|'fill'
                            'start' (default) places the item in the
                            left-most position if vertical, top-most
                            if horizontal (toward the start of the
                            container).
                            'center' centers the item horizontally if
                            the container is vertical or vertically if
                            the container is horizontal.
                            'end' places the item in the right-most
                            position if vertical, bottom-most if
                            horizontal (toward the start of the
                            container).

    """

    def ignore(*ignore):
        pass

    defaults = {
        'x': 0.0,
        'y': 0.0,
        'cx': 0.0,
        'cy': 0.0,
        'sx': 0.0,
        'sy': 0.0,
        'on_axis': 'normal',
        'off_axis': 'start',
        'pinned': False,
        'pad_x': 0.0,
        'pad_y': 0.0,
        'margin_x': 0.0,
        'margin_y': 0.0,
        'border': 0.0,
        'border_color': UIStyle.border_color,
        'border_radius': 4.0,
        'font': UIStyle.font,
        'font_size': 11.0,
        'bg': None,
        'fg': UIStyle.fg,
        'parent': None,
        'children': None,

        'hovered': True,
        'focused': False,
        'is_dirty': True,

        'can_focus': False,

        'on_click': ignore,
        'on_mouse_down': ignore,
        'on_mouse_up': ignore,
        'on_mouse_move': ignore,
        'on_key_down': ignore,
        'on_key_up': ignore,
        'on_char': ignore,

        'on_hover': ignore,
        'on_hover_off': ignore,
        'on_focus': ignore,
        'on_focus_lost': ignore,
        'on_drag_start': ignore,
        'on_drag_move': ignore,
        'on_drag_end': ignore,

        'on_dirty': ignore,
    }


    def __init__(self, **kwargs):
        for name, value in {**Box.defaults, **kwargs}.items():
            setattr(self, name, value)

        if not self.children:
            self.children = []
        for c in self.children:
            c.parent = self


    def size_content(self):
        """Get the size of just the internal content
           and set self.cx and self.cy"""
        cx, cy = 0.0, 0.0
        pass

    def pack(self, avail_x, avail_y):
        """Pack all children into their final place."""
        for c in self.children:
            c.size_content()
            c.sx, c.sy = c.cx + c.pad_x * 2.0, c.cy + c.pad_y * 2.0
            c.pack(c.sx, c.sy)

    def draw(self, g):
        """Draw this control and its children onto the provided canvas."""

        self.is_dirty = False

        def draw_border(c):
            if c.border == 0.0:
                return
            b = c.border
            g.reset_state()
            g.set_line_width(b)
            g.set_stroke(c.border_color)
            g.rounded(c.x + b * 0.5,
                      c.y + b * 0.5,
                      c.sx + b,
                      c.sy + b,
                      c.border_radius,
                      c.border_radius)
            g.stroke()

        def draw_ctrl(c):
            draw_border(c)
            sub = g.subcanvas(c.x + c.border, c.y + c.border, c.sx, c.sy)
            c.draw(sub)
            sub.free()

        if self.bg:
            g.set_clear(self.bg)
            g.clear()

        any_pinned = False
        for c in self.children:
            if c.pinned:
                any_pinned = True
            else:
                draw_ctrl(c)
        for c in self.children:
            if c.pinned: draw_ctrl(c)


    def add(self, child):
        """Add a child to this control."""
        child.parent = self
        self.children.append(child)

    def focus(self):
        """Attempt to focus on this control."""
        sys_set_focus(self)

    def contains_point(self, x, y):
        """Return true if the point is within this box."""
        return x >= self.x and x <= self.x + self.sx and \
               y >= self.y and y <= self.y + self.sy

    def child_under(self, x, y):
        """Return the child and adjusted (x, y) under point (x, y).
           Return (None, 0.0, 0.0) if this point is outside of
           the current control.
        """
        if not self.contains_point(x, y):
            return (None, 0.0, 0.0)

        for c in self.children:
            if c.contains_point(x - self.x, y - self.y):
                return c.child_under(x - self.x, y - self.y)
        return (self, x - self.x, y - self.y)

    def to_screen_point(self, x, y):
        """Return the screen coordinates for this control."""
        if not self.parent:
            return (x + self.x, y + self.y)
        return self.parent.to_screen_point(x + self.x, y + self.y)

    def dirty(self):
        """Mark this control in need of repainting."""
        self.is_dirty = True
        self.handle_dirty()

    def handle_dirty(self):
        if self.parent:
            self.parent.dirty()
        self.on_dirty(self);

    def handle_mouse_down(self, x, y, button, mods):
        if self.can_focus:
            sys_set_focus(self)
        self.on_mouse_down(self, x, y, button, mods)

    def handle_mouse_up(self, x, y, button, mods):
        self.on_mouse_up(self, x, y, button, mods)
        if not self.can_focus or self.focused:
            self.handle_mouse_click(x, y, button, mods)

    def handle_mouse_click(self, x, y, button, mods):
        self.on_click(self, x, y, button, mods)

    def handle_mouse_move(self, x, y, button, mods):
        self.on_mouse_move(self, x, y, button, mods)

    def handle_key_down(self, key, mods):
        if self.on_key_down is Box.ignore:
            self.parent.handle_key_down(key, mods)
        else:
            self.on_key_down(self, key, mods)

    def handle_key_up(self, key, mods):
        if self.on_key_up is Box.ignore:
            self.on_key_up(self, key, mods)
        else:
            self.on_key_up(self, key, mods)

    def handle_char(self, codepoint):
        self.on_char(self, codepoint)

    def handle_drag_start(self, x, y, button, mods):
        self.on_drag_start(self, x, y, button, mods)

    def handle_drag_move(self, x, y, button, mods):
        self.dirty()
        self.on_drag_move(self, x, y, button, mods)

    def handle_drag_end(self, x, y, button, mods):
        self.on_drag_end(self, x, y, button, mods)

    def handle_focus(self):
        self.on_focus(self)

    def handle_focus_lost(self):
        self.on_focus_lost(self)

    def handle_hover(self, x, y):
        self.on_hover(self, x, y)

    def handle_hover_off(self):
        self.on_hover_off(self)




class Stack(Box):
    """Stacking box:
        Stack children either going vertical (default) or horizontal.
        The stack can be aligned to either hte beginning or with
        spaces between children.
    """

    defaults = {
        'direction': 'vertical',
        'justify': False,
    }


    def __init__(self, **kwargs):
        super().__init__(**{**Stack.defaults, **kwargs})


    def size_content(self):
        cx = 0.0
        cy = 0.0
        for c in self.children:
            c.size_content()
            total_x = c.cx + (c.pad_x + c.border + c.margin_x) * 2.0
            total_y = c.cy + (c.pad_y + c.border + c.margin_y) * 2.0
            if self.direction == 'vertical':
                cx = max(cx, total_x)
                cy += total_y
            elif self.direction == 'horizontal':
                cx += total_x
                cy = max(cy, total_y)
        self.cx = cx + self.pad_x * 2.0
        self.cy = cy + self.pad_y * 2.0


    def pack(self, avail_x, avail_y):
        def set_pos(c, value, vert):
            if vert:
                c.y = value
            else:
                c.x = value

        def inner_size(c, vert):
            return (c.cy + c.pad_y * 2.0) if vert else (c.cx + c.pad_x * 2.0)

        def set_inner_size(c, value, vert):
            if vert:
                c.sy = value
            else:
                c.sx = value

        def extra_size(c, vert):
            return (c.border + (c.margin_y if vert else c.margin_x)) * 2.0

        def total_size(c, vert):
            return inner_size(c, vert) + extra_size(c, vert)

        def share_axis(total_avail):
            parts = 0
            avail = total_avail
            for c in self.children:
                c.size_content()
                if c.on_axis == 'fill':
                    parts += 1
                elif c.on_axis.endswith('%'):
                    percent = float(c.on_axis.replace('%', ''))
                    size = avail * percent / 100.0
                    setattr(c, 'percent_value', size)
                    avail -= size
                else:
                    avail -= total_size(c, on_axis)
            if parts:
                return (avail / parts, 0.0)
            if not self.justify:
                return (0.0, 0.0)
            return (0.0, avail / max(1, len(self.children) - 1))

        if not self.children:
            return

        vert = self.direction == 'vertical'
        on_axis = vert
        off_axis = not on_axis
        avail_x -= self.pad_x * 2.0
        avail_y -= self.pad_y * 2.0
        (share, just) = share_axis(avail_y if vert else avail_x)

        # Lay out children.
        x = self.pad_x
        y = self.pad_y
        for c in self.children:
            if c.pinned:
                continue

            c.x = x + c.margin_x
            c.y = y + c.margin_y
            c.sx = getattr(c, 'percent_value', 0.0) or inner_size(c, False)
            c.sy = getattr(c, 'percent_value', 0.0) or inner_size(c, True)

            if c.on_axis == 'fill':
                set_inner_size(c, share - extra_size(c, on_axis), on_axis)

            space = (avail_x if vert else avail_y) - extra_size(c, off_axis)
            size = c.sx if vert else c.sy

            if c.off_axis == 'fill':
                set_inner_size(c, space, off_axis)
            elif c.off_axis == 'start':
                pass
            elif c.off_axis == 'center':
                set_pos(c, (space - size) * 0.5, off_axis)
            elif c.off_axis == 'end':
                set_pos(c, space - size, off_axis)

            if vert:
                y = just + c.y + c.sy + c.border * 2.0 + c.margin_y
            else:
                x = just + c.x + c.sx + c.border * 2.0 + c.margin_x

            c.pack(c.sx, c.sy)


class Label(Box):
    defaults = {}
    header_defaults = {
        'font_size': Box.defaults['font_size'] * 2.0,
        'margin_y': Box.defaults['font_size'],
    }
    subheader_defaults = {
        'font_size': Box.defaults['font_size'] * 1.5,
        'margin_y': Box.defaults['font_size'] * 0.75,
    }
    section_defaults = {
        'font_size': Box.defaults['font_size'] * 1.25,
        'margin_y': Box.defaults['font_size'] * 1.0,
    }

    def __init__(self, **kwargs):
        super().__init__(**{**Label.defaults, **kwargs})
        self.text = kwargs.get('text') or '(sample text)'
        self.font = kwargs.get('font') or UIStyle.font
        self.fg = kwargs.get('fg') or UIStyle.fg

    def Header(**kwargs):
        return Label(**{**Label.header_defaults, **kwargs})

    def Subheader(**kwargs):
        return Label(**{**Label.subheader_defaults, **kwargs})

    def Section(**kwargs):
        return Label(**{**Label.section_defaults, **kwargs})

    def size_content(self):
        self.font.scale(self.font_size * pg.pg_dpi().y / 72.0, 0.0)
        self.cx = self.font.measure(self.text)
        self.cy = self.font.get_height()

    def draw(self, g):
        self.font.scale(self.font_size * g.dpi().y / 72.0, 0.0)
        g.set_fill(self.fg)
        g.print(self.font,
                self.sx * 0.5 - (self.cx * 0.5),
                self.sy * 0.5 - (self.cy * 0.5),
                self.text)


class Control(Box):
    defaults = {
        'can_focus': True,
    }

    def __init__(self, **kwargs):
        super().__init__(**{**Control.defaults, **kwargs})


class Debug(Control):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

    def draw(self, g):
        g.set_fill(Paint.grey)
        g.rectangle(0, 0, self.sx, self.sy)
        g.fill()
        g.set_fill(Paint.black)
        g.rectangle(self.pad_x, self.pad_y, self.cx, self.cy)
        g.fill()


class Button(Control):
    defaults = {
        'text': '',
        'border': 2.0,
        'pad_x': 12.0,
        'pad_y': 8.0,
        'face_paint': UIStyle.face_paint,
        'pressed_face_paint': UIStyle.pressed_face_paint,
        'hovered_face_paint': UIStyle.hovered_face_paint,
        'pressed': False,
        'hovered': False,
    }

    def __init__(self, **kwargs):
        super().__init__(**{**Button.defaults, **kwargs})

    def size_content(self):
        if self.children:
            for c in self.children:
                c.size_content()
            self.cx, self.cy = self.content.cx, self.content.cy
        else:
            self.cx = self.font.measure(self.text)
            self.cy = self.font.get_height()

    def draw(self, g):
        if self.pressed:
            g.set_clear(self.pressed_face_paint)
        elif self.hovered:
            g.set_clear(self.hovered_face_paint)
        else:
            g.set_clear(self.face_paint)
        g.clear()
        super().draw(g)
        if self.text:
            g.set_fill(self.fg)
            g.print(self.font,
                    self.sx * 0.5 - (self.cx * 0.5),
                    self.sy * 0.5 - (self.cy * 0.5),
                    self.text)


hovered = None
focused = None
drag_x, drag_y = 0, 0
drag_ctl = None

def sys_begin_drag(ctl, x, y, button, mods):
    """Begin dragging a control.
    It must be focusable.
    There must not be any other control being dragged.
    """
    global drag_x, drag_y
    global drag_ctl

    if drag_ctl:
        return
    if not ctl.can_focus:
        return

    drag_x, drag_y = x, y
    drag_ctl = ctl
    drag_ctl.pinned = True
    drag_ctl.handle_drag_start(x, y, button, mods)

def sys_drag_move(x, y, button, mods):
    "Move the dragged control the the mouse location."
    drag_ctl.x = x - drag_x
    drag_ctl.y = y - drag_y
    drag_ctl.handle_drag_move(x, y, button, mods)

def sys_end_drag(x, y, button, mods):
    "Clear any dragged control."
    global drag_ctl
    if drag_ctl:
        drag_ctl.handle_drag_end(x, y, button, mods)
    drag_ctl = None

# Manage the global hover.
# Return true if hover status changed.
def sys_set_hover(ctl, x, y):
    global hovered
    if hovered is not ctl:
        if hovered:
            hovered.hovered = False
            hovered.handle_hover_off()
        hovered = ctl
        if hovered:
            hovered.hovered = True
            hovered.handle_hover(x, y)
        return True
    return False

# Mange the global focus.
# Return true if focus status changed.
def sys_set_focus(ctl):
    global focused
    if focused is not ctl:
        if focused:
            focused.focused = False
            focused.handle_focus_lost()
        focused = ctl
        if focused:
            focused.focused = True
            focused.handle_focus()
        return True
    return False


def sys_mouse_move(e, root):
    global hovered
    global drag_ctl

    # Dragging takes precedence over anything else.
    if drag_ctl:
        px, py = drag_ctl.parent.to_screen_point(0.0, 0.0)
        x, y = e.mouse.pos.x - px, e.mouse.pos.y - py
        sys_drag_move(x, y, e.mouse.button, e.mouse.mods)
        return

    ctl, x, y = root.child_under(e.mouse.pos.x, e.mouse.pos.y)
    if hovered is not ctl:
        if sys_set_hover(ctl, x, y):
            root.dirty()
    if ctl:
        ctl.handle_mouse_move(x, y, e.mouse.button, e.mouse.mods)

def sys_mouse_down(e, root):
    ctl, x, y = root.child_under(e.mouse.pos.x, e.mouse.pos.y)
    if ctl:
        ctl.handle_mouse_down(x, y, e.mouse.button, e.mouse.mods)

def sys_mouse_up(e, root):
    global drag_ctl

    if drag_ctl:
        sys_end_drag(e.mouse.pos.x,
                     e.mouse.pos.y,
                     e.mouse.button,
                     e.mouse.mods)
        return

    ctl, x, y = root.child_under(e.mouse.pos.x, e.mouse.pos.y)
    if ctl:
        ctl.handle_mouse_up(x, y, e.mouse.button, e.mouse.mods)

def sys_key_down(e, root):
    if focused:
        focused.handle_key_down(e.key.key, e.key.mods)

def sys_key_up(e, root):
    if focused:
        focused.handle_key_up(e.key.key, e.key.mods)

def sys_char(e, root):
    if focused:
        focused.handle_char(e.char.codepoint)

def wait(evt, root):
    evt = pg.wait(evt)

    type = evt.any.type
    if type == pg.EventType.RESIZE:
        root.dirty()
    elif type == pg.EventType.REDRAW:
        root.dirty()
    elif type == pg.EventType.MOUSE_DOWN:
        sys_mouse_down(evt, root)
    elif type == pg.EventType.MOUSE_UP:
        sys_mouse_up(evt, root)
    elif type == pg.EventType.MOUSE_MOVE:
        sys_mouse_move(evt, root)
    elif type == pg.EventType.KEY_DOWN:
        sys_key_down(evt, root)
    elif type == pg.EventType.KEY_UP:
        sys_key_up(evt, root)
    elif type == pg.EventType.CHAR:
        sys_char(evt, root)

    return evt
