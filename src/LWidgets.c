#include "LWidgets.h"
#include "Gui.h"
#include "Drawer2D.h"
#include "Launcher.h"
#include "ExtMath.h"
#include "Window.h"
#include "Funcs.h"
#include "LWeb.h"
#include "Platform.h"
#include "LScreens.h"
#include "Input.h"

#ifndef CC_BUILD_WEB
#define BORDER 1
#define BORDER2 (2 * BORDER)
#define BORDER3 (3 * BORDER)
#define BORDER4 (4 * BORDER)

void LWidget_SetLocation(void* widget, cc_uint8 horAnchor, cc_uint8 verAnchor, int xOffset, int yOffset) {
	struct LWidget* w = (struct LWidget*)widget;
	w->horAnchor = horAnchor; w->verAnchor = verAnchor;
	w->xOffset   = xOffset;   w->yOffset   = yOffset;
	LWidget_CalcPosition(widget);
}

void LWidget_CalcPosition(void* widget) {
	struct LWidget* w = (struct LWidget*)widget;
	w->x = Gui_CalcPos(w->horAnchor, Display_ScaleX(w->xOffset), w->width,  Window_Width);
	w->y = Gui_CalcPos(w->verAnchor, Display_ScaleY(w->yOffset), w->height, Window_Height);
}

void LWidget_Draw(void* widget) {
	struct LWidget* w = (struct LWidget*)widget;
	w->last.X = w->x; w->last.Width  = w->width;
	w->last.Y = w->y; w->last.Height = w->height;

	if (w->hidden) return;
	w->VTABLE->Draw(w);
	Launcher_MarkDirty(w->x, w->y, w->width, w->height);
}

void LWidget_Redraw(void* widget) {
	struct LWidget* w = (struct LWidget*)widget;
	Launcher_ResetArea(w->last.X, w->last.Y, w->last.Width, w->last.Height);
	LWidget_Draw(w);
}


/*########################################################################################################################*
*------------------------------------------------------ButtonWidget-------------------------------------------------------*
*#########################################################################################################################*/
static BitmapCol LButton_Expand(BitmapCol a, int amount) {
	int r, g, b;
	r = BitmapCol_R(a) + amount; Math_Clamp(r, 0, 255);
	g = BitmapCol_G(a) + amount; Math_Clamp(g, 0, 255);
	b = BitmapCol_B(a) + amount; Math_Clamp(b, 0, 255);
	return BitmapCol_Make(r, g, b, 255);
}

static void LButton_DrawBackground(struct LButton* w) {
	BitmapCol activeCol   = BitmapCol_Make(126, 136, 191, 255);
	BitmapCol inactiveCol = BitmapCol_Make(111, 111, 111, 255);
	BitmapCol col;

	if (Launcher_ClassicBackground) {
		col = w->hovered ? activeCol : inactiveCol;
		Gradient_Noise(&Launcher_Framebuffer, col, 8,
						w->x + BORDER,      w->y + BORDER,
						w->width - BORDER2, w->height - BORDER2);
	} else {
		col = w->hovered ? Launcher_ButtonForeActiveCol : Launcher_ButtonForeCol;
		Gradient_Vertical(&Launcher_Framebuffer, LButton_Expand(col, 8), LButton_Expand(col, -8),
						  w->x + BORDER,      w->y + BORDER,
						  w->width - BORDER2, w->height - BORDER2);
	}
}

static void LButton_DrawBorder(struct LButton* w) {
	BitmapCol black   = BitmapCol_Make(0, 0, 0, 255);
	BitmapCol backCol = Launcher_ClassicBackground ? black : Launcher_ButtonBorderCol;

	Drawer2D_Clear(&Launcher_Framebuffer, backCol, 
					w->x + BORDER,            w->y,
					w->width - BORDER2,       BORDER);
	Drawer2D_Clear(&Launcher_Framebuffer, backCol, 
					w->x + BORDER,            w->y + w->height - BORDER,
					w->width - BORDER2,       BORDER);
	Drawer2D_Clear(&Launcher_Framebuffer, backCol, 
					w->x,                     w->y + BORDER,
					BORDER,                   w->height - BORDER2);
	Drawer2D_Clear(&Launcher_Framebuffer, backCol, 
					w->x + w->width - BORDER, w->y + BORDER,
					BORDER,                   w->height - BORDER2);
}

static void LButton_DrawHighlight(struct LButton* w) {
	BitmapCol activeCol   = BitmapCol_Make(189, 198, 255, 255);
	BitmapCol inactiveCol = BitmapCol_Make(168, 168, 168, 255);
	BitmapCol highlightCol;

	if (Launcher_ClassicBackground) {
		highlightCol = w->hovered ? activeCol : inactiveCol;
		Drawer2D_Clear(&Launcher_Framebuffer, highlightCol,
						w->x + BORDER2,     w->y + BORDER,
						w->width - BORDER4, BORDER);
		Drawer2D_Clear(&Launcher_Framebuffer, highlightCol, 
						w->x + BORDER,       w->y + BORDER2,
						BORDER,              w->height - BORDER4);
	} else if (!w->hovered) {
		Drawer2D_Clear(&Launcher_Framebuffer, Launcher_ButtonHighlightCol, 
						w->x + BORDER2,      w->y + BORDER,
						w->width - BORDER4,  BORDER);
	}
}

static void LButton_Draw(void* widget) {
	struct LButton* w = (struct LButton*)widget;
	struct DrawTextArgs args;
	int xOffset, yOffset;
	if (w->hidden) return;

	xOffset = w->width  - w->_textSize.Width;
	yOffset = w->height - w->_textSize.Height;
	DrawTextArgs_Make(&args, &w->text, &Launcher_TitleFont, true);

	LButton_DrawBackground(w);
	LButton_DrawBorder(w);
	LButton_DrawHighlight(w);

	if (!w->hovered) Drawer2D_Cols['f'] = Drawer2D_Cols['7'];
	Drawer2D_DrawText(&Launcher_Framebuffer, &args, 
					  w->x + xOffset / 2, w->y + yOffset / 2);

	if (!w->hovered) Drawer2D_Cols['f'] = Drawer2D_Cols['F'];
	Launcher_MarkDirty(w->x, w->y, w->width, w->height);
}

static void LButton_Hover(void* w, int x, int y, cc_bool wasOver) {
	/* only need to redraw when changing from unhovered to hovered */	
	if (!wasOver) LWidget_Draw(w); 
}

const static struct LWidgetVTABLE lbutton_VTABLE = {
	LButton_Draw, NULL,
	NULL, NULL,                  /* Key    */
	LButton_Hover, LWidget_Draw, /* Hover  */
	NULL, NULL                   /* Select */
};
void LButton_Init(struct LScreen* s, struct LButton* w, int width, int height, const char* text) {
	w->VTABLE = &lbutton_VTABLE;
	w->tabSelectable = true;
	w->width  = Display_ScaleX(width);
	w->height = Display_ScaleY(height);

	LButton_SetConst(w, text);
	s->widgets[s->numWidgets++] = (struct LWidget*)w;
}

void LButton_SetConst(struct LButton* w, const char* text) {
	struct DrawTextArgs args;
	w->text = String_FromReadonly(text);
	DrawTextArgs_Make(&args, &w->text, &Launcher_TitleFont, true);
	w->_textSize = Drawer2D_MeasureText(&args);
}


/*########################################################################################################################*
*------------------------------------------------------InputWidget--------------------------------------------------------*
*#########################################################################################################################*/
CC_NOINLINE static void LInput_GetText(struct LInput* w, String* text) {
	int i;
	if (!w->password) { *text = w->text; return; }

	for (i = 0; i < w->text.length; i++) {
		String_Append(text, '*');
	}
}

static void LInput_DrawOuterBorder(struct LInput* w) {
	BitmapCol col = BitmapCol_Make(97, 81, 110, 255);

	if (w->selected) {
		Drawer2D_Clear(&Launcher_Framebuffer, col, 
			w->x,                     w->y, 
			w->width,                 BORDER);
		Drawer2D_Clear(&Launcher_Framebuffer, col, 
			w->x,                     w->y + w->height - BORDER, 
			w->width,                 BORDER);
		Drawer2D_Clear(&Launcher_Framebuffer, col, 
			w->x,                     w->y, 
			BORDER,                   w->height);
		Drawer2D_Clear(&Launcher_Framebuffer, col, 
			w->x + w->width - BORDER, w->y, 
			BORDER,                   w->height);
	} else {
		Launcher_ResetArea(w->x,                     w->y, 
						   w->width,                 BORDER);
		Launcher_ResetArea(w->x,                     w->y + w->height - BORDER,
						   w->width,                 BORDER);
		Launcher_ResetArea(w->x,                     w->y, 
						   BORDER,                   w->height);
		Launcher_ResetArea(w->x + w->width - BORDER, w->y, 
						   BORDER,                   w->height);
	}
}

static void LInput_DrawInnerBorder(struct LInput* w) {
	BitmapCol col = BitmapCol_Make(165, 142, 168, 255);

	Drawer2D_Clear(&Launcher_Framebuffer, col,
		w->x + BORDER,             w->y + BORDER, 
		w->width - BORDER2,        BORDER);
	Drawer2D_Clear(&Launcher_Framebuffer, col,
		w->x + BORDER,             w->y + w->height - BORDER2,
		w->width - BORDER2,        BORDER);
	Drawer2D_Clear(&Launcher_Framebuffer, col,
		w->x + BORDER,             w->y + BORDER, 
		BORDER,                    w->height - BORDER2);
	Drawer2D_Clear(&Launcher_Framebuffer, col,
		w->x + w->width - BORDER2, w->y + BORDER,
		BORDER,                    w->height - BORDER2);
}

static void LInput_BlendBoxTop(struct LInput* w) {
	BitmapCol col = BitmapCol_Make(0, 0, 0, 255);

	Gradient_Blend(&Launcher_Framebuffer, col, 75,
		w->x + BORDER,      w->y + BORDER, 
		w->width - BORDER2, BORDER);
	Gradient_Blend(&Launcher_Framebuffer, col, 50,
		w->x + BORDER,      w->y + BORDER2,
		w->width - BORDER2, BORDER);
	Gradient_Blend(&Launcher_Framebuffer, col, 25,
		w->x + BORDER,      w->y + BORDER3, 
		w->width - BORDER2, BORDER);
}

static void LInput_DrawText(struct LInput* w, struct DrawTextArgs* args) {
	int y, hintHeight;

	if (w->text.length || !w->hintText) {
		y = w->y + (w->height - w->_textHeight) / 2;
		Drawer2D_DrawText(&Launcher_Framebuffer, args, w->x + 5, y + 2);
	} else {
		args->text = String_FromReadonly(w->hintText);
		args->font = &Launcher_HintFont;

		hintHeight = Drawer2D_TextHeight(args);
		y = w->y + (w->height - hintHeight) / 2;
		Drawer2D_DrawText(&Launcher_Framebuffer, args, w->x + 5, y);
	}
}

static void LInput_Draw(void* widget) {
	struct LInput* w = (struct LInput*)widget;
	String text; char textBuffer[STRING_SIZE];
	struct DrawTextArgs args;
	Size2D size;

	String_InitArray(text, textBuffer);
	LInput_GetText(w, &text);
	DrawTextArgs_Make(&args, &text, &Launcher_TextFont, false);

	size = Drawer2D_MeasureText(&args);
	w->width       = max(w->minWidth, size.Width + 20);
	w->_textHeight = size.Height;

	LInput_DrawOuterBorder(w);
	LInput_DrawInnerBorder(w);
	Drawer2D_Clear(&Launcher_Framebuffer, BITMAPCOL_WHITE,
		w->x + BORDER2,     w->y + BORDER2, 
		w->width - BORDER4, w->height - BORDER4);
	LInput_BlendBoxTop(w);

	Drawer2D_Cols['f'] = Drawer2D_Cols['0'];
	LInput_DrawText(w, &args);
	Drawer2D_Cols['f'] = Drawer2D_Cols['F'];
}

static Rect2D LInput_MeasureCaret(struct LInput* w) {
	String text; char textBuffer[STRING_SIZE];
	struct DrawTextArgs args;
	Rect2D r;

	String_InitArray(text, textBuffer);
	LInput_GetText(w, &text);
	DrawTextArgs_Make(&args, &text, &Launcher_TextFont, true);

	r.X = w->x + 5;
	r.Y = w->y + w->height - 5; r.Height = 2;

	if (w->caretPos == -1) {
		r.X += Drawer2D_TextWidth(&args);
		r.Width = 10;
	} else {
		args.text = String_UNSAFE_Substring(&text, 0, w->caretPos);
		r.X += Drawer2D_TextWidth(&args);

		args.text = String_UNSAFE_Substring(&text, w->caretPos, 1);
		r.Width   = Drawer2D_TextWidth(&args);
	}
	return r;
}

static TimeMS caretStart;
static cc_bool lastCaretShow;
static Rect2D lastCaretRec;
#define Rect2D_Equals(a, b) a.X == b.X && a.Y == b.Y && a.Width == b.Width && a.Height == b.Height

static void LInput_TickCaret(void* widget) {
	struct LInput* w = (struct LInput*)widget;
	int elapsed;
	cc_bool caretShow;
	Rect2D r;

	elapsed = (int)(DateTime_CurrentUTC_MS() - caretStart);
	if (!caretStart) return;

	caretShow = (elapsed % 1000) < 500;
	if (caretShow == lastCaretShow) return;
	lastCaretShow = caretShow;

	LInput_Draw(w);
	r = LInput_MeasureCaret(w);

	if (caretShow) {
		Drawer2D_Clear(&Launcher_Framebuffer, BITMAPCOL_BLACK,
					   r.X, r.Y, r.Width, r.Height);
	}
	
	if (Rect2D_Equals(r, lastCaretRec)) {
		/* Fast path, caret is blinking in same spot */
		Launcher_MarkDirty(r.X, r.Y, r.Width, r.Height);
	} else {
		Launcher_MarkDirty(w->x, w->y, w->width, w->height);
	}
	lastCaretRec = r;
}

static void LInput_AdvanceCaretPos(struct LInput* w, cc_bool forwards) {
	if (forwards && w->caretPos == -1) return;
	if (!forwards && w->caretPos == 0) return;
	if (w->caretPos == -1 && !forwards) /* caret after text */
		w->caretPos = w->text.length;

	w->caretPos += (forwards ? 1 : -1);
	if (w->caretPos < 0 || w->caretPos >= w->text.length) w->caretPos = -1;
	LWidget_Redraw(w);
}

static void LInput_MoveCaretToCursor(struct LInput* w) {
	String text; char textBuffer[STRING_SIZE];
	struct DrawTextArgs args;
	int i, charX, charWidth;
	int x = Mouse_X, y = Mouse_Y;

	/* Input widget may have been selected by pressing tab */
	/* In which case cursor is completely outside, so ignore */
	if (!Gui_Contains(w->x, w->y, w->width, w->height, x, y)) return;
	lastCaretShow = false;

	String_InitArray(text, textBuffer);
	LInput_GetText(w, &text);
	x -= w->x; y -= w->y;

	DrawTextArgs_Make(&args, &text, &Launcher_TextFont, true);
	if (x >= Drawer2D_TextWidth(&args)) {
		w->caretPos = -1; return; 
	}

	for (i = 0; i < text.length; i++) {
		args.text = String_UNSAFE_Substring(&text, 0, i);
		charX     = Drawer2D_TextWidth(&args);

		args.text = String_UNSAFE_Substring(&text, i, 1);
		charWidth = Drawer2D_TextWidth(&args);
		if (x >= charX && x < charX + charWidth) {
			w->caretPos = i; return;
		}
	}
}

static void LInput_Select(void* widget, cc_bool wasSelected) {
	caretStart = DateTime_CurrentUTC_MS();
	LInput_MoveCaretToCursor((struct LInput*)widget);
	/* TODO: Only draw outer border */
	if (wasSelected) return;
	LWidget_Draw(widget);
	Window_OpenKeyboard();
}

static void LInput_Unselect(void* widget) {
	caretStart = 0;
	/* TODO: Only draw outer border */
	LWidget_Draw(widget);
	Window_CloseKeyboard();
}

static void LInput_CopyFromClipboard(String* text, void* widget) {
	struct LInput* w = (struct LInput*)widget;
	String_UNSAFE_TrimStart(text);
	String_UNSAFE_TrimEnd(text);

	if (w->ClipboardFilter) w->ClipboardFilter(text);
	LInput_AppendString(w, text);
}

static void LInput_KeyDown(void* widget, int key, cc_bool was) {
	struct LInput* w = (struct LInput*)widget;
	if (key == KEY_BACKSPACE) {
		LInput_Backspace(w);
	} else if (key == KEY_DELETE) {
		LInput_Delete(w);
	} else if (key == 'C' && Key_IsActionPressed()) {
		if (w->text.length) Clipboard_SetText(&w->text);
	} else if (key == 'V' && Key_IsActionPressed()) {
		Clipboard_RequestText(LInput_CopyFromClipboard, w);
	} else if (key == KEY_ESCAPE) {
		LInput_Clear(w);
	} else if (key == KEY_LEFT) {
		LInput_AdvanceCaretPos(w, false);
	} else if (key == KEY_RIGHT) {
		LInput_AdvanceCaretPos(w, true);
	}
}

static void LInput_KeyChar(void* widget, char c) {
	struct LInput* w = (struct LInput*)widget;
	LInput_Append(w, c);
}

static cc_bool LInput_DefaultInputFilter(char c) {
	return c >= ' ' && c <= '~' && c != '&';
}

const static struct LWidgetVTABLE linput_VTABLE = {
	LInput_Draw, LInput_TickCaret,
	LInput_KeyDown, LInput_KeyChar, /* Key    */
	NULL, NULL,                     /* Hover  */
	/* TODO: Don't redraw whole thing, just the outer border */
	LInput_Select, LInput_Unselect  /* Select */
};
void LInput_Init(struct LScreen* s, struct LInput* w, int width, const char* hintText) {
	w->VTABLE = &linput_VTABLE;
	w->tabSelectable = true;
	w->TextFilter    = LInput_DefaultInputFilter;
	String_InitArray(w->text, w->_textBuffer);
	
	w->width    = Display_ScaleX(width);
	w->height   = Display_ScaleY(30);
	w->minWidth = w->width;
	LWidget_CalcPosition(w);

	w->hintText = hintText;
	w->caretPos = -1;
	s->widgets[s->numWidgets++] = (struct LWidget*)w;
}

void LInput_SetText(struct LInput* w, const String* text_) {
	String text; char textBuffer[STRING_SIZE];
	struct DrawTextArgs args;
	Size2D size;

	String_Copy(&w->text, text_);
	String_InitArray(text, textBuffer);
	LInput_GetText(w, &text);
	DrawTextArgs_Make(&args, &text, &Launcher_TextFont, true);

	size = Drawer2D_MeasureText(&args);
	w->width       = max(w->minWidth, size.Width + 20);
	w->_textHeight = size.Height;
}

static CC_NOINLINE cc_bool LInput_AppendRaw(struct LInput* w, char c) {
	if (w->TextFilter(c) && w->text.length < w->text.capacity) {
		if (w->caretPos == -1) {
			String_Append(&w->text, c);
		} else {
			String_InsertAt(&w->text, w->caretPos, c);
			w->caretPos++;
		}
		return true;
	}
	return false;
}

void LInput_Append(struct LInput* w, char c) {
	cc_bool appended = LInput_AppendRaw(w, c);
	if (appended && w->TextChanged) w->TextChanged(w);
	if (appended) LWidget_Redraw(w);
}

void LInput_AppendString(struct LInput* w, const String* str) {
	int i, appended = 0;
	for (i = 0; i < str->length; i++) {
		if (LInput_AppendRaw(w, str->buffer[i])) appended++;
	}

	if (appended && w->TextChanged) w->TextChanged(w);
	if (appended) LWidget_Redraw(w);
}

void LInput_Backspace(struct LInput* w) {
	if (!w->text.length || w->caretPos == 0) return;

	if (w->caretPos == -1) {
		String_DeleteAt(&w->text, w->text.length - 1);
	} else {	
		String_DeleteAt(&w->text, w->caretPos - 1);
		w->caretPos--;
		if (w->caretPos == -1) w->caretPos = 0;
	}

	if (w->TextChanged) w->TextChanged(w);
	if (w->caretPos >= w->text.length) w->caretPos = -1;
	LWidget_Redraw(w);
}

void LInput_Delete(struct LInput* w) {
	if (!w->text.length || w->caretPos == -1) return;

	String_DeleteAt(&w->text, w->caretPos);
	if (w->caretPos == -1) w->caretPos = 0;

	if (w->TextChanged) w->TextChanged(w);
	if (w->caretPos >= w->text.length) w->caretPos = -1;
	LWidget_Redraw(w);
}

void LInput_Clear(struct LInput* w) {
	if (!w->text.length) return;
	w->text.length = 0;

	if (w->TextChanged) w->TextChanged(w);
	w->caretPos = -1;
	LWidget_Redraw(w);
}


/*########################################################################################################################*
*------------------------------------------------------LabelWidget--------------------------------------------------------*
*#########################################################################################################################*/
static void LLabel_Draw(void* widget) {
	struct LLabel* w = (struct LLabel*)widget;
	struct DrawTextArgs args;

	DrawTextArgs_Make(&args, &w->text, w->font, true);
	Drawer2D_DrawText(&Launcher_Framebuffer, &args, w->x, w->y);
}

const static struct LWidgetVTABLE llabel_VTABLE = {
	LLabel_Draw, NULL,
	NULL, NULL, /* Key    */
	NULL, NULL, /* Hover  */
	NULL, NULL  /* Select */
};
void LLabel_Init(struct LScreen* s, struct LLabel* w, const char* text) {
	w->VTABLE = &llabel_VTABLE;
	w->font   = &Launcher_TextFont;

	String_InitArray(w->text, w->_textBuffer);
	LLabel_SetConst(w, text);
	s->widgets[s->numWidgets++] = (struct LWidget*)w;
}

void LLabel_SetText(struct LLabel* w, const String* text) {
	struct DrawTextArgs args;
	Size2D size;
	String_Copy(&w->text, text);

	DrawTextArgs_Make(&args, &w->text, w->font, true);
	size = Drawer2D_MeasureText(&args);
	w->width = size.Width; w->height = size.Height;
	LWidget_CalcPosition(w);
}

void LLabel_SetConst(struct LLabel* w, const char* text) {
	String str = String_FromReadonly(text);
	LLabel_SetText(w, &str);
}


/*########################################################################################################################*
*-------------------------------------------------------BoxWidget---------------------------------------------------------*
*#########################################################################################################################*/
static void LLine_Draw(void* widget) {
	struct LLine* w = (struct LLine*)widget;
	Gradient_Blend(&Launcher_Framebuffer, w->col, 128, w->x, w->y, w->width, w->height);
}

const static struct LWidgetVTABLE lline_VTABLE = {
	LLine_Draw, NULL,
	NULL, NULL, /* Key    */
	NULL, NULL, /* Hover  */
	NULL, NULL  /* Select */
};
void LLine_Init(struct LScreen* s, struct LLine* w, int width) {
	w->VTABLE = &lline_VTABLE;
	w->width  = Display_ScaleX(width);
	w->height = Display_ScaleY(2);
	s->widgets[s->numWidgets++] = (struct LWidget*)w;
}


/*########################################################################################################################*
*------------------------------------------------------SliderWidget-------------------------------------------------------*
*#########################################################################################################################*/
static void LSlider_DrawBoxBounds(struct LSlider* w) {
	BitmapCol boundsTop    = BitmapCol_Make(119, 100, 132, 255);
	BitmapCol boundsBottom = BitmapCol_Make(150, 130, 165, 255);

	/* TODO: Check these are actually right */
	Drawer2D_Clear(&Launcher_Framebuffer, boundsTop,
				  w->x,     w->y,
				  w->width, BORDER);
	Drawer2D_Clear(&Launcher_Framebuffer, boundsBottom,
				  w->x,	    w->y + w->height - BORDER,
				  w->width, BORDER);

	Gradient_Vertical(&Launcher_Framebuffer, boundsTop, boundsBottom,
					 w->x,					   w->y,
					 BORDER,                   w->height);
	Gradient_Vertical(&Launcher_Framebuffer, boundsTop, boundsBottom,
					 w->x + w->width - BORDER, w->y,
					 BORDER,				   w->height);
}

static void LSlider_DrawBox(struct LSlider* w) {
	BitmapCol progTop    = BitmapCol_Make(220, 204, 233, 255);
	BitmapCol progBottom = BitmapCol_Make(207, 181, 216, 255);
	int halfHeight = (w->height - BORDER2) / 2;

	Gradient_Vertical(&Launcher_Framebuffer, progTop, progBottom,
					  w->x + BORDER,	  w->y + BORDER, 
					  w->width - BORDER2, halfHeight);
	Gradient_Vertical(&Launcher_Framebuffer, progBottom, progTop,
					  w->x + BORDER,	  w->y + BORDER + halfHeight, 
		              w->width - BORDER2, halfHeight);
}

static void LSlider_Draw(void* widget) {
	struct LSlider* w = (struct LSlider*)widget;
	int curWidth;

	LSlider_DrawBoxBounds(w);
	LSlider_DrawBox(w);

	curWidth = (int)((w->width - BORDER2) * w->value / w->maxValue);
	Drawer2D_Clear(&Launcher_Framebuffer, w->col,
				   w->x + BORDER, w->y + BORDER, 
				   curWidth,      w->height - BORDER2);
}

const static struct LWidgetVTABLE lslider_VTABLE = {
	LSlider_Draw, NULL,
	NULL, NULL, /* Key    */
	NULL, NULL, /* Hover  */
	NULL, NULL  /* Select */
};
void LSlider_Init(struct LScreen* s, struct LSlider* w, int width, int height, BitmapCol col) {
	w->VTABLE   = &lslider_VTABLE;
	w->width    = Display_ScaleX(width); 
	w->height   = Display_ScaleY(height);
	w->maxValue = 100;
	w->col      = col;
	s->widgets[s->numWidgets++] = (struct LWidget*)w;
}


/*########################################################################################################################*
*------------------------------------------------------TableWidget--------------------------------------------------------*
*#########################################################################################################################*/
static void FlagColumn_Draw(struct ServerInfo* row, struct DrawTextArgs* args, int x, int y) {
	Bitmap* bmp = Flags_Get(row);
	if (bmp) Drawer2D_BmpCopy(&Launcher_Framebuffer, x + 2, y + 6, bmp);
}

static void NameColumn_Draw(struct ServerInfo* row, struct DrawTextArgs* args, int x, int y) {
	args->text = row->name;
}
static int NameColumn_Sort(const struct ServerInfo* a, const struct ServerInfo* b) {
	return String_Compare(&b->name, &a->name);
}

static void PlayersColumn_Draw(struct ServerInfo* row, struct DrawTextArgs* args, int x, int y) {
	String_Format2(&args->text, "%i/%i", &row->players, &row->maxPlayers);
}
static int PlayersColumn_Sort(const struct ServerInfo* a, const struct ServerInfo* b) {
	return b->players - a->players;
}

static void UptimeColumn_Draw(struct ServerInfo* row, struct DrawTextArgs* args, int x, int y) {
	int uptime = row->uptime;
	char unit  = 's';

	if (uptime >= SECS_PER_DAY * 7) {
		uptime /= SECS_PER_DAY;  unit = 'd';
	} else if (uptime >= SECS_PER_HOUR) {
		uptime /= SECS_PER_HOUR; unit = 'h';
	} else if (uptime >= SECS_PER_MIN) {
		uptime /= SECS_PER_MIN;  unit = 'm';
	}
	String_Format2(&args->text, "%i%r", &uptime, &unit);
}
static int UptimeColumn_Sort(const struct ServerInfo* a, const struct ServerInfo* b) {
	return b->uptime - a->uptime;
}

static void SoftwareColumn_Draw(struct ServerInfo* row, struct DrawTextArgs* args, int x, int y) {
	args->text = row->software;
}
static int SoftwareColumn_Sort(const struct ServerInfo* a, const struct ServerInfo* b) {
	return String_Compare(&b->software, &a->software);
}

static struct LTableColumn tableColumns[5] = {
	{ "",          15, FlagColumn_Draw,     NULL,                false, false },
	{ "Name",     328, NameColumn_Draw,     NameColumn_Sort,     true,  true  },
	{ "Players",   73, PlayersColumn_Draw,  PlayersColumn_Sort,  true,  true  },
	{ "Uptime",    73, UptimeColumn_Draw,   UptimeColumn_Sort,   true,  true  },
	{ "Software", 143, SoftwareColumn_Draw, SoftwareColumn_Sort, false, true  }
};


static int sortingCol = -1;
/* Works out top and height of the scrollbar */
static void LTable_GetScrollbarCoords(struct LTable* w, int* y, int* height) {
	float scale;
	if (!w->rowsCount) { *y = 0; *height = 0; return; }

	scale   = w->height / (float)w->rowsCount;
	*y      = Math_Ceil(w->topRow * scale);
	*height = Math_Ceil(w->visibleRows * scale);
	*height = min(*y + *height, w->height) - *y;
}

/* Ensures top/first visible row index lies within table */
static void LTable_ClampTopRow(struct LTable* w) { 
	if (w->topRow > w->rowsCount - w->visibleRows) {
		w->topRow = w->rowsCount - w->visibleRows;
	}
	if (w->topRow < 0) w->topRow = 0;
}

/* Returns index of selected row in currently visible rows */
static int LTable_GetSelectedIndex(struct LTable* w) {
	struct ServerInfo* entry;
	int row;

	for (row = 0; row < w->rowsCount; row++) {
		entry = LTable_Get(row);
		if (String_CaselessEquals(w->selectedHash, &entry->hash)) return row;
	}
	return -1;
}

/* Sets selected row to given row, scrolling table if needed */
static void LTable_SetSelectedTo(struct LTable* w, int index) {
	if (!w->rowsCount) return;
	if (index >= w->rowsCount) index = w->rowsCount - 1;
	if (index < 0) index = 0;

	String_Copy(w->selectedHash, &LTable_Get(index)->hash);
	LTable_ShowSelected(w);
	w->OnSelectedChanged();
}

#define HDR_YOFFSET 3
#define HDR_YPADDING 5
#define ROW_YOFFSET 3
#define ROW_YPADDING 1
#define CELL_XOFFSET 6
#define CELL_XPADDING 5

/* Draws background behind column headers */
static void LTable_DrawHeaderBackground(struct LTable* w) {
	BitmapCol gridCol = BitmapCol_Make(20, 20, 10, 255);

	if (!Launcher_ClassicBackground) {
		Drawer2D_Clear(&Launcher_Framebuffer, gridCol,
						w->x, w->y, w->width, w->hdrHeight);
	} else {
		Launcher_ResetArea(w->x, w->y, w->width, w->hdrHeight);
	}
}

/* Works out the background colour of the given row */
static BitmapCol LTable_RowCol(struct LTable* w, struct ServerInfo* row) {
	BitmapCol gridCol     = BitmapCol_Make( 20,  20, 10, 255);
	BitmapCol featSelCol  = BitmapCol_Make( 50,  53,  0, 255);
	BitmapCol featuredCol = BitmapCol_Make(101, 107,  0, 255);
	BitmapCol selectedCol = BitmapCol_Make( 40,  40, 40, 255);
	cc_bool selected;

	if (row) {
		selected = String_Equals(&row->hash, w->selectedHash);
		if (row->featured) {
			return selected ? featSelCol : featuredCol;
		} else if (selected) {
			return selectedCol;
		}
	}
	return Launcher_ClassicBackground ? 0 : gridCol;
}

/* Draws background behind each row in the table */
static void LTable_DrawRowsBackground(struct LTable* w) {
	struct ServerInfo* entry;
	BitmapCol col;
	int y, height, row;

	y = w->rowsBegY;
	for (row = w->topRow; ; row++, y += w->rowHeight) {
		entry = row < w->rowsCount ? LTable_Get(row) : NULL;
		col   = LTable_RowCol(w, entry);	

		/* last row may get chopped off */
		height = min(y + w->rowHeight, w->rowsEndY) - y;
		/* hit the end of the table */
		if (height < 0) break;

		if (col) {
			Drawer2D_Clear(&Launcher_Framebuffer, col,
				w->x, y, w->width, height);
		} else {
			Launcher_ResetArea(w->x, y, w->width, height);
		}
	}
}

/* Draws a gridline below column headers and gridlines after each column */
static void LTable_DrawGridlines(struct LTable* w) {
	int i, x;
	if (Launcher_ClassicBackground) return;

	x = w->x;
	Drawer2D_Clear(&Launcher_Framebuffer, Launcher_BackgroundCol,
				   x, w->y + w->hdrHeight, w->width, w->gridlineHeight);

	for (i = 0; i < w->numColumns; i++) {
		x += w->columns[i].width;
		if (!w->columns[i].columnGridline) continue;
			
		Drawer2D_Clear(&Launcher_Framebuffer, Launcher_BackgroundCol,
					   x, w->y, w->gridlineWidth, w->height);
		x += w->gridlineWidth;
	}
}

/* Draws the entire background of the table */
static void LTable_DrawBackground(struct LTable* w) {
	LTable_DrawHeaderBackground(w);
	LTable_DrawRowsBackground(w);
	LTable_DrawGridlines(w);
}

/* Draws title of each column at top of the table */
static void LTable_DrawHeaders(struct LTable* w) {
	struct DrawTextArgs args;
	int i, x, y;

	DrawTextArgs_MakeEmpty(&args, &Launcher_TextFont, true);
	x = w->x; y = w->y;

	for (i = 0; i < w->numColumns; i++) {
		args.text = String_FromReadonly(w->columns[i].name);
		Drawer2D_DrawClippedText(&Launcher_Framebuffer, &args, 
								x + CELL_XOFFSET, y + HDR_YOFFSET, 
								w->columns[i].width - CELL_XPADDING);

		x += w->columns[i].width;
		if (w->columns[i].columnGridline) x += w->gridlineWidth;
	}
}

/* Draws contents of the currently visible rows in the table */
static void LTable_DrawRows(struct LTable* w) {
	String str; char strBuffer[STRING_SIZE];
	struct ServerInfo* entry;
	struct DrawTextArgs args;
	int i, x, y, row, end;

	String_InitArray(str, strBuffer);
	DrawTextArgs_Make(&args, &str, w->rowFont, true);
	y   = w->rowsBegY;
	end = w->topRow + w->visibleRows;

	for (row = w->topRow; row < end; row++, y += w->rowHeight) {
		x = w->x;

		if (row >= w->rowsCount)            break;
		if (y + w->rowHeight > w->rowsEndY) break;
		entry = LTable_Get(row);

		for (i = 0; i < w->numColumns; i++) {
			args.text = str;
			w->columns[i].DrawRow(entry, &args, x, y);

			if (args.text.length) {
				Drawer2D_DrawClippedText(&Launcher_Framebuffer, &args, 
										x + CELL_XOFFSET, y + ROW_YOFFSET, 
										w->columns[i].width - CELL_XPADDING);
			}

			x += w->columns[i].width;
			if (w->columns[i].columnGridline) x += w->gridlineWidth;
		}
	}
}

/* Draws scrollbar on the right edge of the table */
static void LTable_DrawScrollbar(struct LTable* w) {
	BitmapCol classicBack   = BitmapCol_Make( 80,  80,  80, 255);
	BitmapCol classicScroll = BitmapCol_Make(160, 160, 160, 255);
	BitmapCol backCol   = Launcher_ClassicBackground ? classicBack   : Launcher_ButtonBorderCol;
	BitmapCol scrollCol = Launcher_ClassicBackground ? classicScroll : Launcher_ButtonForeActiveCol;

	int x, y, height;
	x = w->x + w->width - w->scrollbarWidth;
	LTable_GetScrollbarCoords(w, &y, &height);

	Drawer2D_Clear(&Launcher_Framebuffer, backCol,
					x, w->y,     w->scrollbarWidth, w->height);		
	Drawer2D_Clear(&Launcher_Framebuffer, scrollCol, 
					x, w->y + y, w->scrollbarWidth, height);
}

cc_bool LTable_HandlesKey(int key) {
	return key == KEY_UP || key == KEY_DOWN || key == KEY_PAGEUP || key == KEY_PAGEDOWN;
}

static void LTable_KeyDown(void* widget, int key, cc_bool was) {
	struct LTable* w = (struct LTable*)widget;
	int index = LTable_GetSelectedIndex(w);

	if (key == KEY_UP) {
		index--;
	} else if (key == KEY_DOWN) {
		index++;
	} else if (key == KEY_PAGEUP) {
		index -= w->visibleRows;
	} else if (key == KEY_PAGEDOWN) {
		index += w->visibleRows;
	} else { return; }

	w->_lastRow = -1;
	LTable_SetSelectedTo(w, index);
}

static void LTable_MouseMove(void* widget, int deltaX, int deltaY, cc_bool wasOver) {
	struct LTable* w = (struct LTable*)widget;
	int x = Mouse_X - w->x, y = Mouse_Y - w->y, col;

	if (w->draggingScrollbar) {
		float scale = w->height / (float)w->rowsCount;
		int row     = (int)((y - w->mouseOffset) / scale);
		/* avoid expensive redraw when possible */
		if (w->topRow == row) return;

		w->topRow = row;
		LTable_ClampTopRow(w);
		LWidget_Draw(w);
	} else if (w->draggingColumn >= 0) {
		if (!deltaX || x >= w->x + w->width - 20) return;
		col = w->draggingColumn;

		w->columns[col].width += deltaX;
		Math_Clamp(w->columns[col].width, 20, w->width - 20);
		LWidget_Draw(w);
	}
}

static void LTable_RowsClick(struct LTable* w) {
	int mouseY = Mouse_Y - w->rowsBegY;
	int row    = w->topRow + mouseY / w->rowHeight;
	TimeMS now;

	LTable_SetSelectedTo(w, row);
	now = DateTime_CurrentUTC_MS();

	/* double click on row to join */
	if (w->_lastClick + 1000 >= now && row == w->_lastRow) {
		Launcher_ConnectToServer(&LTable_Get(row)->hash);
	}

	w->_lastRow   = LTable_GetSelectedIndex(w);
	w->_lastClick = now;
}

/* Handles clicking on column headers (either resizes a column or sort rows) */
static void LTable_HeadersClick(struct LTable* w) {
	int x, i, mouseX = Mouse_X;

	for (i = 0, x = w->x; i < w->numColumns; i++) {
		/* clicked on gridline, begin dragging */
		if (mouseX >= (x - 8) && mouseX < (x + 8) && w->columns[i].interactable) {
			w->draggingColumn = i - 1;
			return;
		}

		x += w->columns[i].width;
		if (w->columns[i].columnGridline) x += w->gridlineWidth;
	}

	for (i = 0, x = w->x; i < w->numColumns; i++) {
		if (mouseX >= x && mouseX < (x + w->columns[i].width) && w->columns[i].interactable) {
			sortingCol = i;
			w->columns[i].invertSort = !w->columns[i].invertSort;
			LTable_Sort(w);
			return;
		}

		x += w->columns[i].width;
		if (w->columns[i].columnGridline) x += w->gridlineWidth;
	}
}

/* Handles clicking on the scrollbar on right edge of table */
static void LTable_ScrollbarClick(struct LTable* w) {
	int y, height, mouseY = Mouse_Y - w->y;
	LTable_GetScrollbarCoords(w, &y, &height);

	if (mouseY < y) {
		w->topRow -= w->visibleRows;
	} else if (mouseY >= y + height) {
		w->topRow += w->visibleRows;
	} else {
		w->mouseOffset = mouseY - y;
	}

	w->draggingScrollbar = true;
	LTable_ClampTopRow(w);
}

static void LTable_MouseDown(void* widget, cc_bool wasSelected) {
	struct LTable* w = (struct LTable*)widget;

	if (Mouse_X >= Window_Width - w->scrollbarWidth) {
		LTable_ScrollbarClick(w);
		w->_lastRow = -1;
	} else if (Mouse_Y < w->rowsBegY) {
		LTable_HeadersClick(w);
		w->_lastRow = -1;
	} else {
		LTable_RowsClick(w);
	}
	LWidget_Draw(w);
}

static void LTable_MouseWheel(void* widget, float delta) {
	struct LTable* w = (struct LTable*)widget;
	w->topRow -= Utils_AccumulateWheelDelta(&w->_wheelAcc, delta);
	LTable_ClampTopRow(w);
	LWidget_Draw(w);
	w->_lastRow = -1;
}

/* Stops an in-progress dragging of resizing column. */
static void LTable_StopDragging(void* widget) {
	struct LTable* w = (struct LTable*)widget;
	w->draggingColumn    = -1;
	w->draggingScrollbar = false;
	w->mouseOffset       = 0;
}

void LTable_Reposition(struct LTable* w) {
	int rowsHeight;
	w->hdrHeight = Drawer2D_FontHeight(&Launcher_TextFont, true) + HDR_YPADDING;
	w->rowHeight = Drawer2D_FontHeight(w->rowFont,         true) + ROW_YPADDING;

	w->rowsBegY = w->y + w->hdrHeight + w->gridlineHeight;
	w->rowsEndY = w->y + w->height;
	rowsHeight  = w->height - (w->rowsBegY - w->y);

	w->visibleRows = rowsHeight / w->rowHeight;
	LTable_ClampTopRow(w);
}

static void LTable_Draw(void* widget) {
	struct LTable* w = (struct LTable*)widget;
	LTable_DrawBackground(w);
	LTable_DrawHeaders(w);
	LTable_DrawRows(w);
	LTable_DrawScrollbar(w);
	Launcher_MarkAllDirty();
}

const static struct LWidgetVTABLE ltable_VTABLE = {
	LTable_Draw,      NULL,
	LTable_KeyDown,   NULL, /* Key    */
	LTable_MouseMove, NULL, /* Hover  */
	LTable_MouseDown, LTable_StopDragging, /* Select */
	LTable_MouseWheel,      /* Wheel */
};
void LTable_Init(struct LTable* w, struct FontDesc* rowFont) {
	int i;
	w->VTABLE     = &ltable_VTABLE;
	w->columns    = tableColumns;
	w->numColumns = Array_Elems(tableColumns);
	w->rowFont    = rowFont;

	w->scrollbarWidth = Display_ScaleX(10);
	w->gridlineWidth  = Display_ScaleX(2);
	w->gridlineHeight = Display_ScaleY(2);
	
	for (i = 0; i < w->numColumns; i++) {
		w->columns[i].width = Display_ScaleX(w->columns[i].width);
	}
}

void LTable_Reset(struct LTable* w) {
	LTable_StopDragging(w);
	LTable_Reposition(w);

	w->topRow    = 0;
	w->rowsCount = 0;
	sortingCol   = -1;
	w->_wheelAcc = 0.0f;

	w->selectedHash->length = 0;
	w->filter->length       = 0;
}

void LTable_ApplyFilter(struct LTable* w) {
	int i, j, count;

	count = FetchServersTask.numServers;
	for (i = 0, j = 0; i < count; i++) {
		if (String_CaselessContains(&Servers_Get(i)->name, w->filter)) {
			FetchServersTask.servers[j++]._order = FetchServersTask.orders[i];
		}
	}

	w->rowsCount = j;
	for (; j < count; j++) {
		FetchServersTask.servers[j]._order = -100000;
	}

	w->_lastRow = -1;
	LTable_ClampTopRow(w);
}

static int LTable_SortOrder(const struct ServerInfo* a, const struct ServerInfo* b) {
	int order;
	if (sortingCol >= 0) {
		order = tableColumns[sortingCol].SortOrder(a, b);
		return tableColumns[sortingCol].invertSort ? -order : order;
	}

	/* Default sort order. (most active server, then by highest uptime) */
	if (a->players != b->players) return a->players - b->players;
	return a->uptime - b->uptime;
}

static void LTable_QuickSort(int left, int right) {
	cc_uint16* keys = FetchServersTask.orders; cc_uint16 key;

	while (left < right) {
		int i = left, j = right;
		struct ServerInfo* mid = Servers_Get((i + j) >> 1);

		/* partition the list */
		while (i <= j) {
			while (LTable_SortOrder(mid, Servers_Get(i)) < 0) i++;
			while (LTable_SortOrder(mid, Servers_Get(j)) > 0) j--;
			QuickSort_Swap_Maybe();
		}
		/* recurse into the smaller subset */
		QuickSort_Recurse(LTable_QuickSort)
	}
}

void LTable_Sort(struct LTable* w) {
	if (!FetchServersTask.numServers) return;
	FetchServersTask_ResetOrder();
	LTable_QuickSort(0, FetchServersTask.numServers - 1);

	LTable_ApplyFilter(w);
	LTable_ShowSelected(w);
}

void LTable_ShowSelected(struct LTable* w) {
	int i = LTable_GetSelectedIndex(w);
	if (i == -1) return;

	if (i >= w->topRow + w->visibleRows) {
		w->topRow = i - (w->visibleRows - 1);
	}
	if (i < w->topRow) w->topRow = i;
	LTable_ClampTopRow(w);
}
#endif
