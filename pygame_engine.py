import sys
import time
import pygame
from pygame.locals import *

# Colors
class Color(object):
    def __init__(self, hex_col=None, rgb=None):
        # Default Color (Black)
        self.red = 0
        self.green = 0
        self.blue = 0
        self.rgb = (0, 0, 0)
        self.hex = "#000000"
        if (rgb is not None):
            self.red = rgb[0]
            self.green = rgb[1]
            self.blue = rgb[2]
            self.rgb = rgb
            pre_hex = self.red*16**2 + self.green*16 + self.blue
            self.hex = hex(pre_hex)[2:]
            if (len(self.hex) < 6):
                self.hex = '0'*(6-len(self.hex)) + self.hex
            self.hex = '#' + self.hex
        if (hex_col is not None):
            # Check formatting
            if (hex_col[0] == '#' and len(hex_col) == 7):
                self.red = int(hex_col[1:3], 16)
                self.green = int(hex_col[3:5], 16)
                self.blue = int(hex_col[5:7], 16)
                self.rgb = (self.red, self.green, self.blue)
                self.hex = hex_col
            else:
                print("Hex encoding: {} invalid".format(hex_col))

    def col(self, hex_col):
        if (hex_col == True):
            return self.hex
        return self.rgb

    def darker(self):
        return Color(rgb=(int(0.9*self.red), int(0.9*self.green), int(0.9*self.blue)))

class PyGameEngine(object):
    # Default Colors
    LIGHTGRAY = Color(hex_col="#cccccc")
    MEDIUMGRAY = Color(hex_col="#666666")
    DARKGRAY = Color(hex_col="#333333")
    WHITE = Color(hex_col="#ffffff")
    BLACK = Color(hex_col="#000000")
    RED = Color(hex_col="#ff3333")
    GREEN = Color(hex_col="#33ff33")
    BLUE = Color(hex_col="#3333ff")
    
    # Convenient ADJs [NESW | NW, NE, SW, SE]
    ADJS = ((0, -1), (1, 0), (0, 1), (-1, 0), (-1, -1), (1, -1), (-1, 1), (1, 1))

    def __init__(self):
        # GUI Constants
        self.FPS = 30       # Screen refresh rate
        self.HEADER = 50    # Header padding size
        self.FOOTER = 100   # Footer padding size
        self.FONT = None    # Chosen font for game
        
        # Game State flags
        self.AUTO = False   # AUTO-play flag (to freeze user inputs)
        self.PAUSED = False # PAUSED flag (pause )

        self.DISPLAY = None # pygame display object
        self.WIN_X = 400    # Window X size
        self.WIN_Y = 300    # Window Y size
            
    # Drawing Utilities
    def draw_text(self, text, font, color, surface, x, y, background=None, margin=0):
        textobj = font.render(text, True, color)
        textrect = textobj.get_rect()
        textrect.centerx = x
        textrect.centery = y
        if (background is not None):
            pygame.draw.rect(DISPLAY, background, (textrect.centerx - textrect.width/2 - margin, textrect.centery - textrect.height/2 - margin, textrect.width + 2*margin, textrect.height + 2*margin))
        surface.blit(textobj, textrect)

    def draw_text_right(self, text, font, color, surface, x, y, background=None, margin=0):
        textobj = font.render(text, True, color)
        textrect = textobj.get_rect()
        textrect.centerx = x - textrect.width/2
        textrect.centery = y
        if (background is not None):
            pygame.draw.rect(DISPLAY, background, (textrect.centerx - textrect.width/2 - margin, textrect.centery - textrect.height/2 - margin, textrect.width + 2*margin, textrect.height + 2*margin))
        surface.blit(textobj, textrect)

    def draw_text_left(self, text, font, color, surface, x, y, background=None, margin=0):
        textobj = font.render(text, True, color)
        textrect = textobj.get_rect()
        textrect.centerx = x + textrect.width/2
        textrect.centery = y
        if (background is not None):
            pygame.draw.rect(DISPLAY, background, (textrect.centerx - textrect.width/2 - margin, textrect.centery - textrect.height/2 - margin, textrect.width + 2*margin, textrect.height + 2*margin))
        surface.blit(textobj, textrect)

    def draw_rect(self, l, w, c, x, y, outline=None):
        # (display surface, color, tuple(left, top, length, width))
        if (outline is None):
            pygame.draw.rect(DISPLAY, c, (x-w/2, y-l/2, l, w))
        elif (len(outline) == 2):
            outline_color, outline_width = outline
            pygame.draw.rect(DISPLAY, c, (x-w/2, y-l/2, l, w))
            pygame.draw.rect(DISPLAY, outline_color, (x-w/2, y-l/2, l, w), outline_width)

    def drawButton(self, text, color, bgcolor, center_x, center_y):
        # similar to drawText but text has bg color and returns obj & rect
        butSurf = self.FONT.render(text, True, color, bgcolor)
        butRect = butSurf.get_rect()
        butRect.centerx = center_x
        butRect.centery = center_y + butRect.height/2
        self.DISPLAY.blit(butSurf, butRect)
        return (butSurf, butRect)

    def highlightRect(self, rect, col):
        pygame.draw.rect(self.DISPLAY, col, (rect.centerx - rect.width/2, rect.centery - rect.height/2, rect.width, rect.height), 3)


class PyGameEngineGrid(PyGameEngine):
    def __init__(self):
        super().__init__()
        self.SIZE = 30      # Size of 1 cell
        self.MARGIN = 15    # Margin at edge of grid

        self.RESTART_BTN
        self.AUTO_BTN

    def highlightBox(c, r):
        pygame.draw.rect(DISPLAY, GREEN.rgb, (c*self.SIZE+self.MARGIN, r*self.SIZE+self.MARGIN+self.HEADER, self.SIZE, self.SIZE), 3)

    def draw_grid(r, c):
        for y in range(r):
            for x in range(c):
                draw_rect(self.SIZE, self.SIZE, MEDIUMGRAY.rgb, (x+0.5)*self.SIZE+self.MARGIN, (y+0.5)*self.SIZE+self.MARGIN+self.HEADER, outline=(BLACK.rgb, 3))

    def update_LOSE():
        draw_text(":(", self.FONT, WHITE.rgb, DISPLAY, WIN_X//2, self.HEADER - 10, background=RED.rgb, margin=3)

    def update_WIN():
        draw_text(":)", self.FONT, WHITE.rgb, DISPLAY, WIN_X//2, self.HEADER - 10, background=GREEN.rgb, margin=3)

    def update_UI(mines):
        # Buttons
        self.RESTART_BTN = drawButton('RESTART', BLACK.rgb, WHITE.rgb, WIN_X/2, WIN_Y - self.FOOTER)
        self.AUTO_BTN = drawButton('AUTO_PLAY', BLACK.rgb if not AUTO else WHITE.rgb, WHITE.rgb if not AUTO else GREEN.darker().darker().darker().rgb, WIN_X/2, RESTART_BTN[1].centery + self.MARGIN)
        self.DEBUG_NEXT_BTN = drawButton('NEXT MOVE', BLACK.rgb, WHITE.rgb, WIN_X/2, AUTO_BTN[1].centery + self.MARGIN)

        # Text Handles
        num_mines_left = mines
        c = len(GRID)
        r = len(GRID[0])
        for x in range(c):
            for y in range(r):
                if (PLAY_GRID[x][y] == FLAGGED):
                    num_mines_left -= 1
        draw_text_left(str(num_mines_left), self.FONT, RED.rgb, DISPLAY, self.MARGIN, self.HEADER - 10)
        draw_text_right(format_time(int(time.time() - GAME_TIME)), self.FONT, WHITE.rgb, DISPLAY, WIN_X - self.MARGIN, self.HEADER - 10, background=RED.rgb, margin=3)

    def update_grid(r, c):
        global PEEK_UNDER
        DISPLAY.fill(LIGHTGRAY.rgb)

        half = int(self.SIZE*0.5)
        quarter = int(self.SIZE*0.25)
        eighth = int(self.SIZE*0.125)
        for x in range(c):
            for y in range(r):
                if (PEEK_UNDER or PLAY_GRID[x][y] == OPEN):
                    draw_rect(self.SIZE, self.SIZE, LIGHTGRAY.rgb, (x+0.5)*self.SIZE+self.MARGIN, (y+0.5)*self.SIZE+self.MARGIN+self.HEADER, outline=(BLACK.rgb, 3))
                    txt = ''
                    if (GRID[x][y] == MINE):
                        txt = 'X'
                        left = int((x+0.5)*self.SIZE+self.MARGIN - self.SIZE/2)
                        top = int((y+0.5)*self.SIZE+self.MARGIN+self.HEADER - self.SIZE/2)
                        pygame.draw.circle(DISPLAY, DARKGRAY.rgb, (left+half, top+half), quarter)
                        pygame.draw.circle(DISPLAY, WHITE.rgb, (left+half, top+half), eighth)
                        pygame.draw.line(DISPLAY, DARKGRAY.rgb, (left+eighth, top+half), (left+half+quarter+eighth, top+half))
                        pygame.draw.line(DISPLAY, DARKGRAY.rgb, (left+half, top+eighth), (left+half, top+half+quarter+eighth))
                        pygame.draw.line(DISPLAY, DARKGRAY.rgb, (left+quarter, top+quarter), (left+half+quarter, top+half+quarter))
                        pygame.draw.line(DISPLAY, DARKGRAY.rgb, (left+quarter, top+half+quarter), (left+half+quarter, top+quarter))
                        continue
                    elif (GRID[x][y] > 0):
                        txt = str(GRID[x][y])
                    draw_text(txt, self.FONT, BLUE.rgb, DISPLAY, (x+0.5)*self.SIZE+self.MARGIN, (y+0.5)*self.SIZE+self.MARGIN+self.HEADER)
                elif (PLAY_GRID[x][y] == FLAGGED):
                    draw_rect(self.SIZE, self.SIZE, MEDIUMGRAY.rgb, (x+0.5)*self.SIZE+self.MARGIN, (y+0.5)*self.SIZE+self.MARGIN+self.HEADER, outline=(BLACK.rgb, 3))
                    draw_text('F', self.FONT, RED.rgb, DISPLAY, (x+0.5)*self.SIZE+self.MARGIN, (y+0.5)*self.SIZE+self.MARGIN+self.HEADER)
                else:
                    draw_rect(self.SIZE, self.SIZE, MEDIUMGRAY.rgb, (x+0.5)*self.SIZE+self.MARGIN, (y+0.5)*self.SIZE+self.MARGIN+self.HEADER, outline=(BLACK.rgb, 3))

    # Helpers
    def format_time(t_secs):
        time_str = ""
        mins = t_secs//60
        secs = t_secs%60
        if (mins < 10):
            time_str = '0' + str(mins) + ':'
        else:
            time_str = str(mins) + ':'
        if (secs < 10):
            time_str += '0' + str(secs)
        else:
            time_str += str(secs)
        return time_str

    def getBox(mouse_x, mouse_y, row, col):
        bx = mouse_x - self.MARGIN
        by = mouse_y - self.MARGIN - self.HEADER
        br = by//self.SIZE
        bc = bx//self.SIZE
        if (br >= 0 and br < row and bc >= 0 and bc < col):
            return (bc, br)
        else:
            return (None, None)

# Game Logic
def gen_grid(row, col, mines):
    new_grid = [[0 for r in range(row)] for c in range(col)]
    play_grid = [[HIDDEN for r in range(row)] for c in range(col)]
    list_mines = []
    while(mines > 0):
        rx = random.randint(0, col-1)
        ry = random.randint(0, row-1)
        print(rx, ry, file=sys.stderr)
        if (new_grid[rx][ry] != 0):
            continue
        list_mines.append((rx, ry))
        new_grid[rx][ry] = MINE
        mines -= 1
    for rx, ry in list_mines:
        for adj in ADJS:
            nx = rx+adj[0]
            ny = ry+adj[1]
            if (nx >= 0 and nx < col and ny >= 0 and ny < row and new_grid[nx][ny] != MINE):
                new_grid[nx][ny] += 1
    for x in range(col):
        for y in range(row):
            if (new_grid[x][y] == MINE):
                print('X', file=sys.stderr, end="")
            else:
                print(new_grid[x][y], file=sys.stderr, end="")
        print("", file=sys.stderr)
    return (new_grid, play_grid)

    # Event Handlers
    def terminate():
        pygame.quit()
        print("bye-bye")
        sys.stdout.flush()
        sys.exit()

    def update_keyboard_events():
        global PEEK_UNDER
        if (len(pygame.event.get(QUIT)) > 0):
            terminate()
        keyUpEvents = pygame.event.get(KEYUP)
        keyDownEvents = pygame.event.get(KEYDOWN)
        if (len(keyUpEvents) == 0 and len(keyDownEvents) == 0):
            return (None, None)
        if (len(keyUpEvents) > 0):
            if (keyUpEvents[0].key == K_ESCAPE):
                terminate()
            if (keyUpEvents[0].key == K_BACKSLASH):
                PEEK_UNDER = False
            return (False, keyUpEvents[0].key)
        if (len(keyDownEvents) > 0):
            if (keyDownEvents[0].key == K_BACKSLASH):
                PEEK_UNDER = True
            return (True, keyDownEvents[0].key)

    def setup(row, col, mines):
        pygame.init()
        pygame.display.set_caption("Auto-Minesweeper")
        # Game Window Settings
        self.WIN_X = col*self.SIZE + self.MARGIN*2
        self.WIN_Y = row*self.SIZE + self.MARGIN*2 + self.HEADER + self.FOOTER
        self.DISPLAY = pygame.display.set_mode((self.WIN_X, self.WIN_Y))
        self.FPSCLOCK = pygame.time.Clock()
        # Setup Font
        self.FONT = pygame.font.Font("Roboto-Regular.ttf", 20)
        # Setup Button States
        self.AUTO = False
        # Draw Grid
        self.draw_grid(row, col)
        self.GRID, self.PLAY_GRID = gen_grid(row, col, mines)
        self.GAME_TIME = time.time()
        self.GAME_LOSE = False
        self.GAME_WIN = False
        self.LAST_GAME_TIME = time.time()
        self.LAST_AUTO_TIME = time.time()

    def update(self, row, col, mines):
        update_grid(row, col)
        update_UI(mines)

# Main Game
def game(row, col, mines):
    #########
    # SETUP #
    #########
    gui = PyGameEngine()
    gui.setup(row, col, mines)

    #############
    # GAME LOOP #
    #############
    mouse_x = 0
    mouse_y = 0

    while True:
        # Refresh Display
        gui.update()

        # User Interaction
        mouseClicked = False
        spacePressed = False
        key_down, key_event = gui.update_keyboard_events()
        key_up = not key_down

        if (key_down is not None):
            print(key_down, key_event, file=sys.stderr)

        # Keyboard Events
        if (key_down and key_event == K_SPACE):
            spacePressed = True

        for event in pygame.event.get():
            if (event.type == QUIT):
                terminate()
            elif (event.type == MOUSEMOTION):
                mouse_x, mouse_y = event.pos
            elif (event.type == MOUSEBUTTONDOWN):
                mouse_x, mouse_y = event.pos
                mouseClicked = True

        #######
        pygame.display.update()
        FPSCLOCK.tick(FPS)