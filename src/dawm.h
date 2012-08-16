#ifndef _DAWM_H_
#define _DAWM_H_

#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef XFT
#include <X11/Xft/Xft.h>
#endif /* XFT */

#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */

#include <libconfig.h>

#define WMNAME "dawm"

/* Macro to determine if the extended sysinfo struct should be used */
#define SYSINFO_EXTENDED __linux__

#define ARRSIZE(x) (int)(sizeof(x) / sizeof(*x))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define STREQ(s1, s2) (strcmp(s1, s2) == 0)
#define INSIDE(x, y, bx, by, bw, bh) ((x >= bx) && x <= (bx + bw) && \
		(y >= by) && y <= (by + bh))

#define DEBUG 1

#if DEBUG
#define DBG(...) dbg(__FILE__, __LINE__, __VA_ARGS__)
#else
#define DBG(...)
#endif

/* Workspace macros */
#define MIN_WS 0
#define MAX_WS 8
#define N_WORKSPACES (MAX_WS - MIN_WS + 1)
#define ALL_WS ULONG_MAX
#define VALID_WORKSPACE(W) ((W >= MIN_WS && W <= MAX_WS) || W == ALL_WS)

/* Layout macros */
#define M_FACT 0.55
#define M_FACTSTEP 0.05
#define N_MASTER 1

#ifdef SYSINFO_EXTENDED
#define BAR_FMT "  %W  [%L]  %T  Uptime: %U  CPU: %P  MEM: %M  BAT: %B"
#else
#define BAR_FMT "  %W  [%L]  %T"
#endif /* SYSINFO_EXTENDED */

#define BAR_FONT "-*-profont-*-*-*-*-10-*-*-*-*-*-*-*"
#define BAR_UPDATE_RATE 5

#define TIME_FMT "%y/%m/%d %H:%M"

/* Client macros */
#define CLIENT_NAME_SIZE 128
#define ISDOCK(c)               (c->wtype & Dock)
#define ISFOCUSABLE(c)          (ISVISIBLE(c) && !ISDOCK(c))
#define ISMOVEABLE(c)           (c->floating && !ISDOCK(c))
#define ISRESIZABLE(c)          (c->floating || !ISARRANGED(c->mon))
#define ISSELECTABLE(c)         (ISVISIBLE(c) && !ISDOCK(c))
#define ISTILED(c)              (!c->floating && ISVISIBLE(c))
#define ISVISIBLE(c)            (c->ws == c->mon->selws_i || c->ws == ALL_WS)
#define WIDTH(c)                ((c)->w + 2 * (c)->bw)
#define HEIGHT(c)               ((c)->h + 2 * (c)->bw)

#define ISARRANGED(M) (true)
#define WS_NAME_SIZE 32
#define DEFAULT_LAYOUT TileHorzLayout

/* Key modifiers */
#define MOD_ALT   Mod1Mask
#define MOD_CTRL  ControlMask
#define MOD_SHIFT ShiftMask
#define MOD_SUPER Mod4Mask
#define MOD_CTRL_SUPER  (MOD_CTRL | MOD_SUPER)
#define MOD_SHIFT_SUPER (MOD_SHIFT | MOD_SUPER)

/* EWMH constants */
#define NET_WM_STATE_ADD 1
#define NET_WM_STATE_TOGGLE 2

/*** Global variables ***/

/* Global XLib variables (see x11.c) */
extern Display *dpy;
extern Window root;
extern int dpy_fd;
extern int screen;
extern int screen_w;
extern int screen_h;

/*** Enums ***/

/* Atoms */
typedef enum {
	WMChangeState,
	WMDelete,
	WMName,
	WMProtocols,
	WMState,
	WMTakeFocus,
	UTF8String,
	LASTAtom
} AtomID;

#ifdef SYSINFO_EXTENDED
/* Battery status */
typedef enum {
	Unknown,
	Charging,
	Discharging,
	Full
} BatteryStatus;
#endif /* SYSINFO_EXTENDED */

typedef enum {
	Normal = 1 << 0,
	Dialog = 1 << 1,
	Dock = 1 << 2
} ClientType;

/* Color IDs */
typedef enum {
	BarBorder,
	BarNormFG,
	BarNormBG,
	BarSelFG,
	BarSelBG,
	WinNormBorder,
	WinSelBorder,
	LASTColor,
	InvalidColor = -1
} ColorID;

/* Cursor IDs */
typedef enum {
	NormalCursor,
	MovementCursor,
	ResizeCursor,
	LASTCursor
} CursorID;

typedef enum {
	KillAction,          /* kill client */
	MoveWindowAction,    /* move client to another workspace */
	QuitAction,          /* close the window manager */
	RestartAction,       /* restart the window manager */
	SetLayoutAction,     /* set the layout for the current workspace */
	SelectAction,        /* select a client */
	SetMasterAction,     /* move the client to the master position */
	SetMasterFactAction, /* set the master size */
	SetMasterNumAction,  /* set the number of master clients */
	SetWsAction,         /* set the current workspace */
	SpawnAction,         /* spawn a program in the background */
	SwapAction,          /* swap windows */
	ToggleBarAction,     /* show/hide the bar on the current monitor */
	ToggleFloatAction,   /* float/arrange the selected client */
	ToggleFsAction,      /*  */
	ToggleWsAction,      /*  */
	LASTAction,
	InvalidAction = -1
} KeyAction;

typedef enum {
	TileHorzLayout,
	TileVertLayout,
	MaxLayout,
	LASTLayout
} LayoutID;

typedef enum {
	NoMotion,
	ResizeMotion,
	MovementMotion
} MotionType;

/* EWMH atoms */
typedef enum {
	NetActiveWindow,
	NetClientList,
	NetCurrentDesktop,
	NetDesktop,
	NetDesktops,
	NetDesktopNames,
	NetSupported,
	NetWMFullscreen,
	NetWMName,
	NetWMState,
	NetWMStrut,
	NetWMStrutPartial,
	NetWMWindowType,
	NetWMWindowTypeDesktop,
	NetWMWindowTypeDialog,
	NetWMWindowTypeDock,
	LASTNetAtom
} NetAtomID;

struct layout_pos {
	int x, y, w, h;
};

struct layout {
	struct layout_pos *pos;
	unsigned n;
	int mw, mh;
	int ww, wh;
	float mfact;
	unsigned nmaster;
	LayoutID id;
};

/* Workspace struct */
struct ws {
	char name[WS_NAME_SIZE]; /* name */
	struct layout *layout;   /* layout */
};

/* Workspace settings struct */
struct ws_settings {
	char name[WS_NAME_SIZE]; /* name */
	float mfact;             /*  */
	unsigned nmaster;        /*  */
	LayoutID layout;         /*  */
};

struct client;
struct monitor {
	int num;                    /* monitor number, 0... */
	struct bar *bar;            /* bar */
	int mx, my, mw, mh;         /* monitor geometry */
	int wx, wy, ww, wh;         /* window geometry */
	struct client *clients;     /* client list */
	struct client *cstack;      /* client stack */
	struct client *sel;         /* selected client */
	struct ws *selws;           /* selected workspace */
	unsigned long selws_i;      /* selected workspace index */
	unsigned long prevws_i;     /* previous workspace index */
	struct ws ws[N_WORKSPACES]; /* workspace information */
	struct monitor *next;       /* next monitor */
};

/* Size hints struct */
struct size_hints {
	int basew, baseh;
	int incw, inch;
	int maxw, maxh;
	int minw, minh;
	float mina, maxa;
	bool fixed;
};

/* Strut data struct */
struct strut_data {
	/* _NET_WM_STRUT and _NET_WM_STRUT_PARTIAL */
	unsigned long left;
	unsigned long right;
	unsigned long top;
	unsigned long bottom;

	/* _NET_WM_STRUT_PARTIAL specific data */
	unsigned long left_start_y, left_end_y;
	unsigned long right_start_y, right_end_y;
	unsigned long top_start_x, top_end_x;
	unsigned long bottom_start_x, bottom_end_x;
};

/* Bar struct */
struct bar {
	Window win;     /* bar window */
	bool topbar;    /* top/bottom of the screen */
	bool showbar;   /* show/hide the bar */
	int x, y, w, h; /* bar geometry */
};

/* Client struct */
struct client {
	char name[CLIENT_NAME_SIZE]; /* title */
	int x, y, w, h;              /* current position and size */
	int ox, oy, ow, oh;          /* old position and size */
	bool floating;               /* floating state */
	bool fullscreen;             /* fullscreen state */
	bool neverfocus;             /*    */
	int ostate;                  /* old state */
	int bw;                      /* border size */
	int obw;                     /* old border size */
	int wtype;                   /* window type bitmask */
	Window win;                  /* window that belongs to the client */
	struct size_hints *shints;   /* size hints data */
	struct strut_data *strut;    /* strut data */
	struct monitor *mon;         /* monitor that the client is on */
	unsigned long ws;            /* workspace that the client is on */
	struct client *prev;         /* previous client in list */
	struct client *next;         /* next client in list */
	struct client *snext;        /* next client in stack */
};

/* Format data struct */
struct format_data {
	const char *layout;    /* layout symbol */
	const char *workspace; /* workspace name */
};

/* Key struct */
struct key {
	unsigned int mod; /* modifier mask */
	KeySym keysym;    /* keysym (XK_) */
	KeyAction action; /* action */
	char *args;       /* action arguments (can be NULL) */
	struct key *next; /* next key in key list */
};

/* System information struct */
struct sysinfo {
	time_t time;    /* current time */
#ifdef SYSINFO_EXTENDED
	int cpu;        /* cpu usage, -1 if unable to calculate cpu */
	int bat_level;  /* battery level, -1 if unable to get value */
	int bat_status; /* battery status */
	long uptime;    /* uptime in seconds, -1 if unable to get value */
	long mem_used;  /* used memory (in kb), -1 if unable to get value */
	long mem_total; /* total memory (in kb), -1 if unable to get value */
#endif /* SYSINFO_EXTENDED */
};

/* Settings struct */
struct settings {
	bool topbar;             /* non-zero if bars should be in the top */
	bool showbar;            /* non-zero if bars should be shown */
	struct key *keys;        /* key bindings */
	char *barfmt;            /* bar format string */
	char *barfont;           /* bar font */
	char *colors[LASTColor]; /* color values, in "#XXXXXX" form */
	struct ws_settings ws[N_WORKSPACES];
	int bw;
};

/* Motion struct */
struct motion {
	MotionType type;        /* motion type */
	XButtonEvent start;     /* event when the motion was started */
	XWindowAttributes attr; /* window attr. when the motion was started */
};

/* Regex struct for matching a rule */
struct rule_match {
	char *class;       /* class name regex */
	char *instance;    /* instance name regex */
	char *title;       /* client title regex */
};

/* Applicable rule settings */
struct rule_settings {
	int mn;            /* monitor number, -1 to avoid this rule */
	unsigned long ws;  /* workspace number */
	bool set_ws;        /* set the workspace if set_ws is non-zero*/
	bool switch_to_ws;  /* switch to the clients' workspace */
	bool floating;      /* float the client */
	bool fullscreen;    /* put the client in fullscreen mode */
	bool ignore_hints;  /* ignore size hints */
};

/*  */
struct rule {
	struct rule_match *match;
	struct rule_settings *settings;
	struct rule *next;
};

/*** Function prototypes ***/

/* atoms.c */
Atom atom(AtomID);
void atoms_init(void);
void atom_append_window(Window, Atom, Window);
void atom_delete(Window, Atom);
int atom_get_atom(Window, Atom, Atom *);
int atom_get_atoms(Window, Atom, Atom **, unsigned *);
int atom_get_cardinal(Window, Atom, unsigned long *);
int atom_get_cardinals(Window, Atom, unsigned long **, unsigned *);
int atom_get_string(Window, Atom, char *, unsigned int);
void atom_set_atom(Window, Atom, Atom);
void atom_set_atoms(Window, Atom, Atom *, unsigned);
void atom_set_cardinal(Window, Atom, unsigned long);
void atom_set_string(Window, Atom, char *);
void atom_set_utf8array(Window, Atom, unsigned char *, unsigned);
void atom_set_window(Window, Atom, Window);

/* bar.c */
struct bar *bar_create(bool, bool, int, int, int);
void bar_draw(struct bar *, const char *str);
void bar_free(struct bar *);
void bars_init(const char *fontstr);
void bars_free(void);

/* client.c */
struct client *client_create(Window, XWindowAttributes *);
void client_free(struct client *);
void client_kill(struct client *);
void client_map_window(struct client *);
void client_move_resize(struct client *, int, int, int, int);
void client_raise(struct client *);
void client_set_floating(struct client *, bool);
void client_set_focus(struct client *, bool);
void client_set_fullscreen(struct client *, bool);
void client_set_state(struct client *, long);
void client_set_ws(struct client *, unsigned long);
void client_setup(struct client *, struct monitor *,
		struct monitor *, struct client *);
void client_show(struct client *, bool);
void client_unmap(struct client *);
void client_update_size_hints(struct client *);
void client_update_title(struct client *);
void client_update_window_type(struct client *);
void client_update_wm_hints(struct client *, bool);

/* colors.c */
unsigned long color(ColorID);
void colors_init(char * const[LASTColor]);
const char *color_id2str(ColorID);
int color_str2id(const char *);

/* cursors.c */
Cursor cursor(CursorID);
void cursor_set(Window, CursorID);
void cursors_init(void);
void cursors_free(void);

/* ewmh.c */
Atom netatom(NetAtomID id);

void ewmh_init(void);
void ewmh_root_client_list_add(Window);
void ewmh_root_client_list_clear(void);
void ewmh_root_set_active_window(Window);
void ewmh_root_set_current_desktop(unsigned long);
void ewmh_root_set_name(char *);
void ewmh_root_set_number_of_desktops(unsigned long);
void ewmh_root_set_desktop_names(unsigned char *, unsigned);
int ewmh_client_get_desktop(Window, unsigned long *);
int ewmh_client_get_state(Window, Atom *);
int ewmh_client_get_strut(Window, struct strut_data *);
int ewmh_client_get_strut_partial(Window, struct strut_data *);
int ewmh_client_get_window_types(Window, Atom **, unsigned *);
void ewmh_client_set_desktop(Window, unsigned long);
void ewmh_client_set_state(Window, Atom);

/* keys.c */
const char *key_action2str(KeyAction);
struct key *key_append(struct key *, struct key *);
struct key *key_copy(struct key *);
struct key *key_create(unsigned int, KeySym, KeyAction,
		const char *, struct key *);
struct key *key_default_keys(void);
struct key *key_free_all(struct key *);
void key_grab_all(struct key *);
void key_init(void);
int key_pressed(struct key *, KeyCode, unsigned int);
KeyAction key_str2action(const char *);
int key_str2mod(const char *);

/* layouts.c */
struct layout *layout_init(LayoutID, int, int, int, int, int, float);
void layout_add_client(struct layout *);
int layout_pos_index(struct layout *, int, int);
void layout_remove_client(struct layout *);
void layout_set(struct layout *, LayoutID);
void layout_set_clients(struct layout *, unsigned);
void layout_set_geom(struct layout *, int, int);
void layout_set_mfact(struct layout *, float);
void layout_set_nmaster(struct layout *, unsigned);
int layout_str2id(const char *);
const char *layout_symbol(const struct layout *);

/* monitor.c */
#define monitor_toggle_bar(M) monitor_show_bar(M, !M->bar->showbar);
void monitor_add_client(struct monitor *, struct client *);
struct monitor *monitor_append(struct monitor *, struct monitor *);
void monitor_arrange(struct monitor *);
int monitor_count(struct monitor *);
struct monitor *monitor_create(int, int, int, int, int);
void monitor_draw_bar(struct monitor *);
void monitor_float_selected(struct monitor *, bool);
void monitor_focus(struct monitor *, struct client *);
struct monitor *monitor_free(struct monitor *);
void monitor_remove_client(struct monitor *, struct client *);
void monitor_select_client(struct monitor *, struct client *, bool);
void monitor_select_next_client(struct monitor *);
void monitor_select_prev_client(struct monitor *);
void monitor_selected_to_master(struct monitor *);
void monitor_set_layout(struct monitor *, int);
void monitor_set_ws(struct monitor *, unsigned long);
void monitor_show_bar(struct monitor *, bool);
void monitor_swap(struct monitor *, struct client *, struct client *);
void monitor_swap_next_client(struct monitor *);
void monitor_swap_prev_client(struct monitor *);
void monitor_unfocus_selected(struct monitor *);

struct client *find_client_by_trans(struct monitor *, Window);
struct client *find_client_by_window(struct monitor *, Window);
struct client *find_nth_tiled_client(struct monitor *, int);

struct monitor *find_monitor_by_num(struct monitor *, int);
struct monitor *find_monitor_by_pos(struct monitor *, int, int);
struct monitor *find_monitor_by_ws(struct monitor *, unsigned);

/* rules.h */
struct rule *rule_create(const char *, const char *, const char *);
void rules_add(struct rule *);
void rules_apply(struct client *);
void rules_free(void);

/* settings.c */
const struct settings *settings(void);
char *settings_default_path(void);
void settings_init(void);
void settings_free(void);
void settings_read(const char *);

/* sysinfo.c */
const struct sysinfo *sysinfo(void);
void sysinfo_format(const char *, char *, unsigned, struct format_data *);
void sysinfo_init(void);
void sysinfo_update(void);

/* utils.c */
void dbg(const char *, int, const char *, ...);
void die(const char *, ...);
void error(const char *, ...);
void spawn(const char *);
char *strfvs(char **, char);
int strmatch(const char *, const char *);
char *strtr(char *, const char *);
char *strtrb(char *, const char *);
char *strtrf(char *, const char *);
void *xcalloc(size_t, size_t);
void *xmalloc(size_t);
void *xrealloc(void *, size_t);
char *xstrdup(const char *);

/* wm.c */
void init(const char *);
void eventloop(void);
void destroy(void);

/* x11.c */
void x11_init(void);
void x11_destroy(void);

/* xutils.c */
long get_state(Window);
int send_event(Window, Atom);
int xerror_dummy(Display *, XErrorEvent *);

#endif
