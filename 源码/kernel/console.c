
#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"
#include "font.h"

/* #define __TTY_DEBUG__ */

/* local routines */
PUBLIC void	set_cursor(unsigned int position);
PUBLIC void	set_video_start_addr(u32 addr);
PUBLIC void	flush(CONSOLE* con);
PRIVATE	void	w_copy(unsigned int dst, const unsigned int src, int size);
PUBLIC void	clear_screen(int pos, int len);
PUBLIC void out_char(CONSOLE* con, char ch);
/*****************************************************************************
 *                                init_screen
 *****************************************************************************/
/**
 * Initialize the console of a certain tty.
 * 
 * @param tty  Whose console is to be initialized.
 *****************************************************************************/
PUBLIC void init_screen(TTY* tty)
{
	int nr_tty = tty - tty_table;
	//tty->console = console_table + nr_tty;
	tty->console = console_table;

	/* 
	 * NOTE:
	 *   variables related to `position' and `size' below are
	 *   in WORDs, but not in BYTEs.
	 */
	// int v_mem_size = V_MEM_SIZE >> 1; /* size of Video Memory */
	// int size_per_con = v_mem_size / NR_CONSOLES;
	// tty->console->orig = nr_tty * size_per_con;
	// tty->console->con_size = size_per_con / SCR_WIDTH * SCR_WIDTH;
	// tty->console->cursor = tty->console->crtc_start = tty->console->orig;
	// tty->console->is_full = 0;

	int v_mem_size = V_MEM_SIZE >> 1; /* size of Video Memory */
	int size_per_con = v_mem_size / NR_CONSOLES;
	tty->console->orig = 0;
	tty->console->con_size = 480;
	tty->console->cursor = tty->console->crtc_start = tty->console->orig;
	tty->console->is_full = 0;
	
	if (nr_tty == 0) {
		//tty->console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		/* 
		 * `?' in this string will be replaced with 0, 1, 2, ...
		 */
		const char prompt[] = "[TTY #?]\n";

		const char * p = prompt;
		for (; *p; p++)
			out_char(tty->console, *p == '?' ? nr_tty + '0' : *p);
	}

	set_cursor(tty->console->cursor);
}

void print_char(int x, int y, char c, char ch)
{
	x=x*8;
	y=y*16;
	char* font=hankaku + ch * 16;
	int i;
	int xsize=320;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = VGA_MEM_BASE + (y + i) * xsize + x;
		for(int j=0; j<8; j++)
			p[j]=0;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

/*****************************************************************************
 *                                out_char
 *****************************************************************************/
/**
 * Print a char in a certain console.
 * 
 * @param con  The console to which the char is printed.
 * @param ch   The char to print.
 *****************************************************************************/
PUBLIC void out_char(CONSOLE* con, char ch)
{
	u8* pch = (u8*)(V_MEM_BASE + con->cursor * 2);

	assert(con->cursor - con->orig < con->con_size);

	/*
	 * calculate the coordinate of cursor in current console (not in
	 * current screen)
	 */
	int cursor_x = (con->cursor - con->orig) % SCR_WIDTH;
	int cursor_y = (con->cursor - con->orig) / SCR_WIDTH;

	switch(ch) {
	case '\n':
		con->cursor = con->orig + SCR_WIDTH * (cursor_y + 1);
		break;
	case '\b':
		if (con->cursor > con->orig) {
			con->cursor--;
			int cursor_x = (con->cursor - con->orig) % SCR_WIDTH;
			int cursor_y = (con->cursor - con->orig) / SCR_WIDTH;
			print_char(cursor_x,cursor_y,7,' ');
		}
		break;
	default:
		print_char(cursor_x,cursor_y,7,ch);
		con->cursor++;
		break;
	}

	if (con->cursor - con->orig >= con->con_size) {
		cursor_x = (con->cursor - con->orig) % SCR_WIDTH;
		cursor_y = (con->cursor - con->orig) / SCR_WIDTH;
		int cp_orig = con->orig + (cursor_y + 1) * SCR_WIDTH - SCR_SIZE;
		w_copy(con->orig, cp_orig, SCR_SIZE - SCR_WIDTH);
		con->crtc_start = con->orig;
		con->cursor = con->orig + (SCR_SIZE - SCR_WIDTH) + cursor_x;
		clear_screen(con->cursor, SCR_WIDTH);
		if (!con->is_full)
			con->is_full = 1;
	}

	assert(con->cursor - con->orig < con->con_size);

	while (con->cursor >= con->crtc_start + SCR_SIZE ||
	       con->cursor < con->crtc_start) {
		scroll_screen(con, SCR_UP);

		clear_screen(con->cursor, SCR_WIDTH);
	}

	flush(con);
}

/*****************************************************************************
 *                                clear_screen
 *****************************************************************************/
/**
 * Write whitespaces to the screen.
 * 
 * @param pos  Write from here.
 * @param len  How many whitespaces will be written.
 *****************************************************************************/
PUBLIC void clear_screen(int pos, int len)
{
	// u8 * pch = (u8*)(V_MEM_BASE + pos * 2);
	
	while (--len >= 0) {
		//*pch++ = ' ';
		//*pch++ = DEFAULT_CHAR_COLOR;
		int cursor_x = (pos) % SCR_WIDTH;
		int cursor_y = (pos) / SCR_WIDTH;
		print_char(cursor_x,cursor_y,7,' ');
		pos++;
	}

}


/*****************************************************************************
 *                            is_current_console
 *****************************************************************************/
/**
 * Uses `nr_current_console' to determine if a console is the current one.
 * 
 * @param con   Ptr to console.
 * 
 * @return   TRUE if con is the current console.
 *****************************************************************************/
PUBLIC int is_current_console(CONSOLE* con)
{
	return (con == &console_table[current_console]);
}


/*****************************************************************************
 *                                set_cursor
 *****************************************************************************/
/**
 * Display the cursor by setting CRTC (6845 compatible) registers.
 * 
 * @param position  Position of the cursor based on the beginning of the video
 *                  memory. Note that it counts in WORDs, not in BYTEs.
 *****************************************************************************/
PUBLIC void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}


/*****************************************************************************
 *                                set_video_start_addr
 *****************************************************************************/
/**
 * Routine for hardware screen scrolling.
 * 
 * @param addr  Offset in the video memory.
 *****************************************************************************/
PUBLIC void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}


/*****************************************************************************
 *                                select_console
 *****************************************************************************/
/**
 * Select a console as the current.
 * 
 * @param nr_console   Console nr, range in [0, NR_CONSOLES-1].
 *****************************************************************************/
PUBLIC void select_console(int nr_console)
{
	
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) return;
	current_console = 0;
	//flush(&console_table[current_console = nr_console]);
}


/*****************************************************************************
 *                                scroll_screen
 *****************************************************************************/
/**
 * Scroll the screen.
 *
 * Note that scrolling UP means the content of the screen will go upwards, so
 * that the user can see lines below the bottom. Similarly scrolling DOWN means
 * the content of the screen will go downwards so that the user can see lines
 * above the top.
 *
 * When there is no line below the bottom of the screen, scrolling UP takes no
 * effects; when there is no line above the top of the screen, scrolling DOWN
 * takes no effects.
 * 
 * @param con   The console whose screen is to be scrolled.
 * @param dir   SCR_UP : scroll the screen upwards;
 *              SCR_DN : scroll the screen downwards
 *****************************************************************************/
PUBLIC void scroll_screen(CONSOLE* con, int dir)
{
	/*
	 * variables below are all in-console-offsets (based on con->orig)
	 */
	int oldest; /* addr of the oldest available line in the console */
	int newest; /* .... .. ... latest ......... .... .. ... ....... */
	int scr_top;/* position of the top of current screen */

	newest = (con->cursor - con->orig) / SCR_WIDTH * SCR_WIDTH;
	oldest = con->is_full ? (newest + SCR_WIDTH) % con->con_size : 0;
	scr_top = con->crtc_start - con->orig;

	if (dir == SCR_DN) {
		if (!con->is_full && scr_top > 0) {
			con->crtc_start -= SCR_WIDTH;
		}
		else if (con->is_full && scr_top != oldest) {
			if (con->cursor - con->orig >= con->con_size - SCR_SIZE) {
				if (con->crtc_start != con->orig)
					con->crtc_start -= SCR_WIDTH;
			}
			else if (con->crtc_start == con->orig) {
				scr_top = con->con_size - SCR_SIZE;
				con->crtc_start = con->orig + scr_top;
			}
			else {
				con->crtc_start -= SCR_WIDTH;
			}
		}
	}
	else if (dir == SCR_UP) {
		if (!con->is_full && newest >= scr_top + SCR_SIZE) {
			con->crtc_start += SCR_WIDTH;
		}
		else if (con->is_full && scr_top + SCR_SIZE - SCR_WIDTH != newest) {
			if (scr_top + SCR_SIZE == con->con_size)
				con->crtc_start = con->orig;
			else
				con->crtc_start += SCR_WIDTH;
		}
	}
	else {
		assert(dir == SCR_DN || dir == SCR_UP);
	}

	flush(con);
}


/*****************************************************************************
 *                                flush
 *****************************************************************************/
/**
 * Set the cursor and starting address of a console by writing the
 * CRT Controller Registers.
 * 
 * @param con  The console to be set.
 *****************************************************************************/
PUBLIC void flush(CONSOLE* con)
{
	// if (is_current_console(con)) {
	// 	set_cursor(con->cursor);
	// 	set_video_start_addr(con->crtc_start);
	// }

#ifdef __TTY_DEBUG__
	int lineno = 0;
	for (lineno = 0; lineno < con->con_size / SCR_WIDTH; lineno++) {
		u8 * pch = (u8*)(V_MEM_BASE +
				   (con->orig + (lineno + 1) * SCR_WIDTH) * 2
				   - 4);
		*pch++ = lineno / 10 + '0';
		*pch++ = RED_CHAR;
		*pch++ = lineno % 10 + '0';
		*pch++ = RED_CHAR;
	}
#endif
}

/*****************************************************************************
 *                                w_copy
 *****************************************************************************/
/**
 * Copy data in WORDS.
 *
 * Note that the addresses of dst and src are not pointers, but integers, 'coz
 * in most cases we pass integers into it as parameters.
 * 
 * @param dst   Addr of destination.
 * @param src   Addr of source.
 * @param size  How many words will be copied.
 *****************************************************************************/
PRIVATE	void w_copy(unsigned int dst, const unsigned int src, int size)
{
	int xsize=320;
	int src_copy=src;
	int src_x,src_y,dst_x,dst_y;
	for(int k=0; k<size; k++)
	{
		src_x=(src_copy % SCR_WIDTH)*8;
		src_y=(src_copy / SCR_WIDTH)*16;
		dst_x=(dst % SCR_WIDTH)*8;
		dst_y=(dst / SCR_WIDTH)*16;
		char *p, *d /* data */;
		for (int i = 0; i < 16; i++) {
			p = VGA_MEM_BASE + (src_y + i) * xsize + src_x;
			d = VGA_MEM_BASE + (dst_y + i) * xsize + dst_x;
			for(int j=0; j<8; j++)
				d[j]=p[j];
		}
		src_copy++;
		dst++;
	}
}

