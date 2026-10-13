#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hmskin Gallery 皮肤位图生成器 —— 按 DESIGN.md「基准台」token 生成全套控件位图。
输出 demo/assets/image/*.png（含 alpha），文件名与旧 .bmp 一一对应（.png 后缀）。
用法: python3 tools/gen_skin.py
"""
import os
from PIL import Image, ImageDraw

# ---------- DESIGN.md tokens ----------
BG      = (15, 18, 22)        # #0F1216 窗体底
SURFACE = (22, 27, 33)        # #161B21 面板/标题栏
RAISED  = (28, 34, 41)        # #1C2229 凸面
SUNKEN  = (17, 21, 26)        # #11151A 凹陷
LINE    = (38, 44, 52)        # #262C34 发丝线
LINE_S  = (54, 62, 72)        # #363E48 强边框
TEXT    = (232, 236, 241)     # #E8ECF1
MUTED   = (139, 147, 158)     # #8B939E
FAINT   = (91, 99, 110)       # #5B636E
ACCENT  = (255, 92, 31)       # #FF5C1F 信号橙
ACC_HOV = (255, 122, 71)      # #FF7A47
ACC_PRS = (204, 72, 23)       # #CC4817
TRUE    = (61, 220, 151)      # #3DDC97 状态绿
DANGER  = (229, 72, 77)       # #E5484D
INNER   = (255, 255, 255, 15) # 6% 白 alpha 内高光
EDGE    = (84, 98, 108)       # 窗框外描边基色
HOV     = (36, 44, 51)        # #242C33 悬停面
SEL     = (46, 58, 67)        # #2E3A43 选中面
DARKPRS = (21, 26, 31)        # #151A1F 按下面

OUT = os.path.join(os.path.dirname(__file__), '..', 'demo', 'assets', 'image')

def canvas(w, h, fill=(0, 0, 0, 0)):
    im = Image.new('RGBA', (w, h), fill)
    return im, ImageDraw.Draw(im)

def save(im, name):
    path = os.path.join(OUT, name)
    im.save(path)
    print(f"  {name:<26} {im.width}x{im.height}")

def rrect(d, box, r, fill=None, outline=None, width=1):
    d.rounded_rectangle(box, radius=r, fill=fill, outline=outline, width=width)

# ---------- 按钮 69x21（次级样式；主色按钮由 demo 文本/状态位图承载） ----------
def gen_button(name, fill, border):
    im, d = canvas(69, 21)
    rrect(d, (0, 0, 68, 20), 4, fill=fill + (255,), outline=border + (255,), width=1)
    d.line((2, 1, 66, 1), fill=INNER)                       # 顶边内高光
    save(im, name)

def gen_buttons():
    gen_button('btn_nor.png', RAISED, (48, 58, 66))   # #303A42
    gen_button('btn_hov.png', HOV,    LINE_S)
    gen_button('btn_pre.png', DARKPRS, LINE)
    im, d = canvas(69, 21)                              # 禁用
    rrect(d, (0, 0, 68, 20), 4, fill=SURFACE + (255,), outline=(36, 43, 49, 255), width=1)
    save(im, 'btn_dis.png')

# ---------- 复选框 13x13 ----------
def checkbox(name, checked, border, glyph_color=None, box_fill=SUNKEN):
    im, d = canvas(13, 13)
    rrect(d, (0, 0, 12, 12), 3, fill=box_fill + (255,), outline=border + (255,), width=1)
    if checked:
        # 13px 勾：两点折线（手绘像素）
        for x, y in [(3, 6), (4, 7), (5, 8), (6, 7), (7, 6), (8, 5), (9, 4)]:
            d.point((x, y), fill=glyph_color + (255,))
            d.point((x, y + 1), fill=glyph_color + (255,))
    save(im, name)

def gen_checkboxes():
    HOVB = (85, 99, 109)   # 悬停描边 #55636D
    checkbox('NoCheckNor.png',     False, LINE_S)
    checkbox('NoCheckHov.png',     False, HOVB)
    checkbox('NoCheckPre.png',     False, TRUE)
    checkbox('NoCheckDisable.png', False, (36, 43, 49), box_fill=SURFACE)
    checkbox('CheckNor.png',     True, TRUE, box_fill=TRUE, glyph_color=(20, 20, 20))
    checkbox('CheckHov.png',     True, HOVB, box_fill=TRUE, glyph_color=(20, 20, 20))
    checkbox('CheckPre.png',     True, ACC_PRS, box_fill=TRUE, glyph_color=(20, 20, 20))
    im, d = canvas(13, 13)
    rrect(d, (0, 0, 12, 12), 3, fill=SURFACE + (255,), outline=(36, 43, 49, 255), width=1)
    for x, y in [(3, 6), (4, 7), (5, 8), (6, 7), (7, 6), (8, 5), (9, 4)]:
        d.point((x, y), fill=FAINT + (255,)); d.point((x, y + 1), fill=FAINT + (255,))
    save(im, 'CheckDisable.png')

# ---------- Tab 条雪碧图 211x33，切片 [0,57)NOR [58,115)HOV [115,171)PRE [171,211]SEL ----------
def gen_tabheader():
    im, d = canvas(211, 33)
    def cell(x0, x1, fill, top_marker=None):
        if fill:
            d.rectangle((x0, 0, x1 - 1, 32), fill=fill + (255,))
        d.line((x0, 32, x1 - 1, 32), fill=LINE + (255,))          # 底线
        if top_marker:
            d.rectangle((x0, 0, x1 - 1, 1), fill=top_marker + (255,))
    cell(0,   57,  None,    None)               # NOR 透明
    cell(57,  114, HOV,     None)               # HOV 悬停面
    cell(114, 170, DARKPRS, None)               # PRE 按下面
    cell(170, 211, RAISED,  ACCENT)             # SEL 凸面+2px 橙顶标（1px 实现）
    save(im, 'tabheader.png')

# ---------- 非客户区框架 ----------
def gen_nc_frame():
    # caption 531x32：面板色 + 顶部内高光 + 底部发丝线（水平拉伸）
    im, d = canvas(531, 32, SURFACE + (255,))
    d.line((0, 1, 530, 1), fill=INNER)
    d.line((0, 31, 530, 31), fill=LINE + (255,))
    save(im, 'nc_bk_caption.png')
    # left 32x248：x=0 外描边 1px，其余 surface（垂直拉伸）
    im, d = canvas(32, 248, SURFACE + (255,))
    d.line((0, 0, 0, 247), fill=EDGE + (160,))
    save(im, 'nc_bk_left.png')
    # right 32x299：x=31 外描边
    im, d = canvas(32, 299, SURFACE + (255,))
    d.line((31, 0, 31, 298), fill=EDGE + (160,))
    save(im, 'nc_bk_right.png')
    # bottom 531x35：顶线发丝线，底线外描边（水平拉伸）
    im, d = canvas(531, 35, SURFACE + (255,))
    d.line((0, 0, 530, 0), fill=LINE + (255,))
    d.line((0, 34, 530, 34), fill=EDGE + (160,))
    save(im, 'nc_bk_bottom.png')

# ---------- 根菜单栏 ----------
def gen_rootmenu():
    im, d = canvas(343, 27, SURFACE + (255,))
    d.line((0, 26, 342, 26), fill=LINE + (255,))
    save(im, 'root_menu_bk.png')
    for name, fill in [('root_mi_nor.png', None),
                       ('root_mi_hov.png', HOV),
                       ('root_mi_pre.png', SEL)]:
        im, d = canvas(35, 23)
        if fill:
            d.rectangle((0, 0, 34, 22), fill=fill + (255,))
        save(im, name)

# ---------- 滚动条（垂直 18x139 / 水平 139x18） ----------
def gen_scrollbar():
    THUMB = {'nor': (74, 86, 96), 'hov': (93, 107, 117), 'pre': ACCENT}   # #4A5660/#5D6B75/橙
    for st, col in THUMB.items():
        im, d = canvas(18, 139, SUNKEN + (255,))          # 轨道
        rrect(d, (6, 20, 11, 118), 3, fill=col + (255,))  # 6px thumb 居中
        save(im, f'scrollbar_{st}.png')
        im, d = canvas(139, 18, SUNKEN + (255,))
        rrect(d, (20, 6, 118, 11), 3, fill=col + (255,))
        save(im, f'scrollbar_{st}_hor.png')

# ---------- 弹出菜单背景 248x243（九宫格友好：均匀面+四边） ----------
def gen_popmenu():
    im, d = canvas(248, 243, SURFACE + (255,))
    d.rectangle((0, 0, 247, 242), outline=LINE_S + (255,), width=1)
    save(im, 'pop_menu_bk.png')

if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    print('生成「基准台」皮肤位图 →', os.path.normpath(OUT))
    gen_buttons()
    gen_checkboxes()
    gen_tabheader()
    gen_nc_frame()
    gen_rootmenu()
    gen_scrollbar()
    gen_popmenu()
    print('完成。')
