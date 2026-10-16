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

# ---------- 按钮 150x42（§1 自绘按钮实际显示尺寸，1:1 绘制零重采样） ----------
BTN_W, BTN_H, BTN_R = 150, 42, 8
SS = 4  # 超采样倍数：4x 绘制再 LANCZOS 缩回，圆角抗锯齿干净

def gen_button(name, fill, border, highlight=True):
    im, d = canvas(BTN_W * SS, BTN_H * SS)
    rrect(d, (0, 0, BTN_W * SS - 1, BTN_H * SS - 1), BTN_R * SS,
          fill=fill + (255,), outline=border + (255,), width=SS)
    if highlight:
        d.line((2 * SS, SS, (BTN_W - 2) * SS, SS), fill=INNER, width=SS)  # 顶边内高光
    save(im.resize((BTN_W, BTN_H), Image.LANCZOS), name)

def gen_buttons():
    gen_button('btn_nor.png', RAISED, (48, 58, 66))   # #303A42
    gen_button('btn_hov.png', HOV,    LINE_S)
    gen_button('btn_pre.png', DARKPRS, LINE)
    gen_button('btn_pri_nor.png', ACCENT,  (201, 127, 12), highlight=False)  # 主按钮：橙面深字
    gen_button('btn_pri_hov.png', ACC_HOV, (201, 127, 12), highlight=False)
    gen_button('btn_pri_pre.png', ACC_PRS, (160, 100, 8), highlight=False)
    gen_button('btn_dis.png', SURFACE, (36, 43, 49), highlight=False)      # 禁用

# ---------- 复选框 15x15（dock thickness 15，1:1） ----------
CHK = 15

def checkbox(name, checked, border, glyph_color=None, box_fill=SUNKEN):
    W = CHK * SS
    im, d = canvas(W, W)
    rrect(d, (0, 0, W - 1, W - 1), 4 * SS, fill=box_fill + (255,), outline=border + (255,), width=SS)
    if checked:
        # 勾：折线一笔（4x 画布上抗锯齿由缩回完成）
        d.line([(W * 0.22, W * 0.52), (W * 0.42, W * 0.70), (W * 0.78, W * 0.30)],
               fill=glyph_color + (255,), width=int(2.2 * SS), joint='curve')
    save(im.resize((CHK, CHK), Image.LANCZOS), name)

def gen_checkboxes():
    HOVB = (85, 99, 109)   # 悬停描边 #55636D
    checkbox('NoCheckNor.png',     False, LINE_S)
    checkbox('NoCheckHov.png',     False, HOVB)
    checkbox('NoCheckPre.png',     False, TRUE)
    checkbox('NoCheckDisable.png', False, (36, 43, 49), box_fill=SURFACE)
    checkbox('CheckNor.png',     True, TRUE, box_fill=TRUE, glyph_color=(20, 20, 20))
    checkbox('CheckHov.png',     True, HOVB, box_fill=TRUE, glyph_color=(20, 20, 20))
    checkbox('CheckPre.png',     True, ACC_PRS, box_fill=TRUE, glyph_color=(20, 20, 20))
    checkbox('CheckDisable.png', True, (36, 43, 49), box_fill=SURFACE, glyph_color=FAINT)

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
    cell(170, 211, RAISED,  ACCENT)             # SEL 凸面+橙顶标
    d.rectangle((170, 1, 210, 2), fill=ACCENT + (255,))  # 顶标加厚到 2px
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

# ---------- 标题栏 caption 按钮 46x32（Win11 风格：常态仅 glyph，悬停亮面，close 悬停红） ----------
CAP_W, CAP_H = 46, 32

def capbtn(name, glyph, fill=None, glyph_color=MUTED):
    W, H = CAP_W * SS, CAP_H * SS
    im, d = canvas(W, H)
    if fill:
        d.rectangle((0, 0, W - 1, H - 1), fill=fill + (255,))
    else:
        # NOR must be OPAQUE ( caption surface color ): state switching draws
        # the new bitmap over the old one without erasing, so a transparent
        # normal state leaves the previous hover/press fill stuck on screen.
        d.rectangle((0, 0, W - 1, H - 1), fill=SURFACE + (255,))
    g = glyph_color + (255,)
    if glyph == 'min':      # 12x2 横线，垂直略偏下（视觉居中）
        d.rectangle((17 * SS, 16 * SS, 29 * SS, 17 * SS), fill=g)
    elif glyph == 'max':    # 10x10 方框
        d.rectangle((18 * SS, 11 * SS, 28 * SS, 21 * SS), outline=g, width=SS)
    elif glyph == 'close':  # x 交叉
        d.line((18 * SS, 11 * SS, 28 * SS, 21 * SS), fill=g, width=int(1.8 * SS))
        d.line((28 * SS, 11 * SS, 18 * SS, 21 * SS), fill=g, width=int(1.8 * SS))
    elif glyph == 'restore':  # 还原：重叠双方块（后框右下偏移 3px）
        d.rectangle((20 * SS, 13 * SS, 29 * SS, 22 * SS), outline=g, width=SS)
        d.rectangle((17 * SS, 10 * SS, 26 * SS, 19 * SS), outline=g, width=SS)
    save(im.resize((CAP_W, CAP_H), Image.LANCZOS), name)

def gen_captionbtns():
    for g in ('min', 'max', 'restore', 'close'):
        capbtn(f'capbtn_{g}_nor.png', g)                                          # 常态：仅 glyph
        bClose = (g == 'close')
        capbtn(f'capbtn_{g}_hov.png', g,
               fill=DANGER if bClose else HOV,
               glyph_color=(255, 255, 255) if bClose else TEXT)                   # 悬停（close 红）
        capbtn(f'capbtn_{g}_pre.png', g,
               fill=(196, 58, 62) if bClose else DARKPRS,
               glyph_color=(255, 255, 255) if bClose else TEXT)                   # 按下

# ---------- §4 滚动条：轨道 18x139 / thumb 18x48 / 上下端钮 18x20 ----------
# 尺寸必须等于引擎实际绘制尺寸（thumb 厚度 = demo setTrackThumbLen(48)，
# 端钮厚度 = hm_posdecoder_dock 默认 20）：位图被整体拉伸进 district，
# 非原生尺寸会让圆角变形发糊（ 6x24 拉到 18x48 的教训）。
SB_W = 18

def gen_scrollbar_v2():
    # 轨道（单态，凹陷面）
    im, d = canvas(SB_W, 139, SUNKEN + (255,))
    d.line((0, 0, SB_W - 1, 0), fill=LINE + (255,))
    save(im, 'sbtrack_nor.png')
    # thumb（三态圆角条，原生 18x48）
    for name, col in [('nor', (74, 86, 96)), ('hov', (93, 107, 117)), ('pre', ACCENT)]:
        im, d = canvas(SB_W * SS, 48 * SS)
        rrect(d, (0, 0, SB_W * SS - 1, 48 * SS - 1), 4 * SS, fill=col + (255,))
        save(im.resize((SB_W, 48), Image.LANCZOS), f'sbthumb_{name}.png')
    # 上下端钮（三态小圆角钮 + 箭头 glyph，原生 18x20）
    for name, fill, border in [('nor', RAISED, (48, 58, 66)),
                               ('hov', HOV, LINE_S),
                               ('pre', DARKPRS, LINE)]:
        im, d = canvas(SB_W * SS, 20 * SS)
        rrect(d, (0, 0, SB_W * SS - 1, 20 * SS - 1), 3 * SS,
              fill=fill + (255,), outline=border + (255,), width=SS)
        g = MUTED if name != 'pre' else TEXT
        gc = g + (255,)
        # 上箭头 ∧（3px 线宽，20 高里垂直居中）
        d.line([(5 * SS, 12 * SS), (9 * SS, 8 * SS), (13 * SS, 12 * SS)], fill=gc, width=SS, joint='curve')
        save(im.resize((SB_W, 20), Image.LANCZOS), f'sbbtn_up_{name}.png')
        im, d = canvas(SB_W * SS, 20 * SS)
        rrect(d, (0, 0, SB_W * SS - 1, 20 * SS - 1), 3 * SS,
              fill=fill + (255,), outline=border + (255,), width=SS)
        d.line([(5 * SS, 8 * SS), (9 * SS, 12 * SS), (13 * SS, 8 * SS)], fill=gc, width=SS, joint='curve')
        save(im.resize((SB_W, 20), Image.LANCZOS), f'sbbtn_down_{name}.png')

# ---------- 弹出菜单背景 248x243（九宫格友好：均匀面+四边） ----------
def gen_popmenu():
    im, d = canvas(248, 243, SURFACE + (255,))
    d.rectangle((0, 0, 247, 242), outline=LINE_S + (255,), width=1)
    save(im, 'pop_menu_bk.png')

# ---------- §1 开关标本 TGL-10：36x20 药丸 8 态 ----------
# 复用 hm_wmp_checkbox（unchecked=OFF / checked=ON），位图按实际绘制尺寸 1:1。
# district 厚度 = 36（demo manifest 里 dock LEFT），改尺寸需两边同步。
TGL_W, TGL_H, KNOB = 36, 20, 16

def gen_toggle():
    # (名字, 药丸填充, 药丸描边, 滑钮颜色, 滑钮在右)
    states = [
        ('off_nor', SUNKEN, LINE,     FAINT,   False),
        ('off_hov', SUNKEN, LINE_S,   MUTED,   False),
        ('off_pre', DARKPRS, LINE,    MUTED,   False),
        ('off_dis', SUNKEN, LINE,     (48, 54, 62), False),
        ('on_nor',  ACCENT, ACC_PRS,  BG,      True),
        ('on_hov',  ACC_HOV, ACC_PRS, BG,      True),
        ('on_pre',  ACC_PRS, ACC_PRS, BG,      True),
        ('on_dis',  (74, 60, 45), LINE, (48, 54, 62), True),
    ]
    for name, pill, border, knob, on in states:
        im, d = canvas(TGL_W * SS, TGL_H * SS)
        rrect(d, (0, 0, TGL_W * SS - 1, TGL_H * SS - 1), (TGL_H // 2) * SS,
              fill=pill + (255,), outline=border + (255,), width=SS)
        kx = (TGL_W - KNOB - 2) * SS if on else 2 * SS
        d.ellipse((kx, 2 * SS, kx + KNOB * SS, (2 + KNOB) * SS), fill=knob + (255,))
        save(im.resize((TGL_W, TGL_H), Image.LANCZOS), f'tgl_{name}.png')

# ---------- §1 单选标本 RAD-11：15x15 圆 8 态 ----------
# 复用 hm_wmp_checkbox（unchecked=空圈 / checked=绿点圈），选中=状态绿圆点
# （勾选族语义一致：选中=TRUE 绿；开关才是信号橙）。与复选框同尺寸 15px。
RAD = CHK

def gen_radio():
    HOVB = (85, 99, 109)   # 悬停描边（与复选框同）
    states = [
        # (名字, 选中, 描边, 圈填充, 点颜色)
        ('off_nor', False, LINE_S, SUNKEN, None),
        ('off_hov', False, HOVB,   SUNKEN, None),
        ('off_pre', False, TRUE,   SUNKEN, None),
        ('off_dis', False, (36, 43, 49), SURFACE, None),
        ('on_nor',  True,  LINE_S, SUNKEN, TRUE),
        ('on_hov',  True,  HOVB,   SUNKEN, TRUE),
        ('on_pre',  True,  ACC_PRS, SUNKEN, ACC_PRS),
        ('on_dis',  True,  (36, 43, 49), SURFACE, FAINT),
    ]
    for name, on, border, ring, dot in states:
        W = RAD * SS
        im, d = canvas(W, W)
        d.ellipse((0, 0, W - 1, W - 1), fill=ring + (255,), outline=border + (255,), width=SS)
        if on:
            pad = int(W * 0.28)
            d.ellipse((pad, pad, W - 1 - pad, W - 1 - pad), fill=dot + (255,))
        save(im.resize((RAD, RAD), Image.LANCZOS), f'rad_{name}.png')

if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    print('生成「基准台」皮肤位图 →', os.path.normpath(OUT))
    gen_buttons()
    gen_checkboxes()
    gen_tabheader()
    gen_nc_frame()
    gen_rootmenu()
    gen_captionbtns()
    gen_scrollbar()
    gen_scrollbar_v2()
    gen_popmenu()
    gen_toggle()
    gen_radio()
    print('完成。')
