#!/usr/bin/env python3
import sys

import pg
import box

Pg = pg.Pg
Pt = pg.Pt
Paint = pg.Paint
Font = pg.Font
ColorSpace = pg.ColorSpace

Stack = box.Stack
Label = box.Label
Control = box.Control
Debug = box.Debug
Button = box.Button

quit_program = False

g = Pg.create(1024, 768, b'Demo')
if not g:
    exit('Could not open window')

def root_key_down(self, key, mods):
    if key == ord('W') and mods == pg.Mods.CTRL:
        global quit_program
        quit_program = True

class Tack(Control):
    defaults = {
        'bg': Paint.blue,
        'cx': 8,
        'cy': 8,
    }

    def __init__(self, **kwargs):
        super().__init__(**{**Tack.defaults, **kwargs})

    def handle_mouse_down(self, x, y, button, mods):
        box.sys_begin_drag(self, x, y, button, mods)

class TackGrid(Control):
    defaults = {
        'on_axis': 'fill',
        'off_axis': 'fill',
        'bg': Paint.grey,
    }

    def __init__(self, **kwargs):
        super().__init__(**{**TackGrid.defaults, **kwargs})
        self.cur_x = 0
        self.cur_y = 0
        self.adding = False

    def draw(self, g):
        super().draw(g)

        g.reset_state()
        cur = Tack(x=self.cur_x, y=self.cur_y)
        all_pts = [*self.children, cur] if self.adding else self.children

        if self.children:
            g.move(all_pts[0].x, all_pts[0].y)
        for i in all_pts[1:]:
            g.line(i.x, i.y)

        g.set_fill(Paint.solid(ColorSpace.LCHAB, 0, 0, 0, .2))
        g.set_stroke(Paint.black)
        g.set_line_width(3)
        g.fill_stroke()



    def handle_mouse_click(self, x, y, button, mods):
        self.add(Tack(x=x, y=y, curve=button))
        if not self.adding:
            self.adding = True
        self.dirty()

    def handle_mouse_move(self, x, y, button, mods):
        if self.adding:
            self.cur_x = x
            self.cur_y = y
            self.dirty()

    def handle_key_down(self, key, mods):
        super().handle_key_down(key, mods)
        if not mods and key == pg.Key.ESCAPE:
            self.adding = False
            self.dirty()


sidepanel = Stack(on_axis='25%',
                  off_axis='fill',
                  bg=Paint.blue,
                  fg=Paint.white,
                  pad_x=8,
                  pad_y=8,
                  children=[
                    Label.Header(text="Header",
                          fg=Paint.white),
                    Label.Subheader(text="Subheader",
                          fg=Paint.white),
                    Label.Section(text="Section",
                          fg=Paint.white),
                    Label(text='Just regular text',
                          fg=Paint.white)
                  ])
main = TackGrid()
root = Stack(on_axis='fill',
             off_axis='fill',
             direction='horizontal',
             children=[
                sidepanel,
                main,
             ],
             on_key_down=root_key_down)

# def tack_mouse_down(self, x, y, button, mods):
#     box.sys_begin_drag(self, x, y, button, mods)
# tack = Control(cx=32, cy=32, bg=Paint.black, on_mouse_down=tack_mouse_down)
# main.add(tack)

def redraw(self):
    g.reset_state()
    g.set_clear(Paint.white)
    g.clear()
    avail = g.size()
    b = Stack(on_axis='fill', off_axis='fill', children=[root])
    b.pack(avail.x, avail.y)
    b.draw(g)
    g.update()

root.on_dirty = redraw
root.focus()

e = None
while not quit_program:
    e = box.wait(e, root)
    if not e:
        break
    if e.any.type == pg.EventType.CLOSE:
        break

