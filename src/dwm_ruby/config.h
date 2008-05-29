/* See LICENSE file for copyright and license details. */

/* appearance */
#define BARPOS			/* BarTop, BarBot, BarOff */  BarOff
#define BORDERPX		1
#define FONT			"-*-terminus-medium-*-*-*-*-*-*-*-*-*-*-*"
#define NORMBORDERCOLOR		"#cccccc"
#define NORMBGCOLOR		"#cccccc"
#define NORMFGCOLOR		"#000000"
#define SELBORDERCOLOR		"#000099"
#define SELBGCOLOR		"#0066ff"
#define SELFGCOLOR		"#ffffff"

/* tagging */
const char tags[][MAXTAGLEN] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
Bool seltags[LENGTH(tags)] = {[0] = True};

/* layout(s) */
#define MWFACT			0.9	/* master width factor [0.1 .. 0.9] */
#define RESIZEHINTS		False /* False - respect size hints in tiled resizals */
#define SNAP			32	/* snap pixel */

/* key definitions */
#define MODKEY			Mod1Mask
Key keys[] = {
	/* modifier			key		function	argument */
	{ MODKEY,			XK_F2,		spawn,
		"exe=`dmenu_path | dmenu -fn '"FONT"' -nb '"NORMBGCOLOR"' -nf '"NORMFGCOLOR"'"
		" -sb '"SELBGCOLOR"' -sf '"SELFGCOLOR"'` && exec $exe" },
	{ MODKEY,			XK_F3,		spawn, "exec `dmenu_desktop`" },
/*	{ MODKEY|ShiftMask,		XK_Return,	spawn, "exec aterm -tr -fade 60% -sh 60 -sr" }, */
	{ MODKEY,			XK_F12,		spawn, "exec xterm"  },
	{ MODKEY|ShiftMask,		XK_c,		killclient,	NULL },
	{ MODKEY,			XK_F4,		killclient,	NULL },
	{ MODKEY|ShiftMask,		XK_q,		spawn,		"exec Xshutdown.sh ask" },
};
