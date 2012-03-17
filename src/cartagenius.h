#ifndef __CARTAGENIUS_H__
#define __CARTAGENIUS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pdflib.h"

#include "ydefs.h"
#include "yerror.h"
#include "ylog.h"
#include "ystr.h"
#include "yvect.h"
#include "ydom.h"
#include "yvalue.h"

#ifdef __CYGWIN__
# define CARTA_SEP	'\\'
#else
# define CARTA_SEP	'/'
#endif /* __CYGWIN__ */

/*! @define DEFAULT_MARGIN Default paper margin. */
#define DEFAULT_MARGIN	"15mm"

/*! @define DEFAULT_CREATOR Default document creator. */
#define DEFAULT_CREATOR	"Pandocréon Carta-Genius (http://www.pandocreon.com)"

/*! @define LUMINANCE_RED Luminance constant for red color. */
#define LUMINANCE_RED		0.299
/*! @define LUMINANCE_GREEN Luminance constant for green color. */
#define LUMINANCE_GREEN		0.587
/*! @define LUMINANCE_BLUE Luminance constant for blue color. */
#define LUMINANCE_BLUE		0.114

/*! @define LUMINANCE_THRESHOLD Luminance minimal value. */
#define LUMINANCE_THRESHOLD	0.1

/*! @define CG_MAX_NESTED_LOOPS Maximum number of nested loops. */
#define CG_MAX_NESTED_LOOPS	1000
/*! @define CG_MAX_LOOP_ITERATIONS Maximum number of loop iterations. */
#define CG_MAX_LOOP_ITERATIONS	10000

/*! @define CG_IS_OPERATOR	Check if a character is an operator. */
#define	CG_IS_OPERATOR(c)	(c == '&' || c == '|' || \
				 c == '=' || c == '<' || c == '>' || \
				 c == '+' || c == '-' || \
				 c == '*' || c == '/' || c == '%')
/*! @define CG_LEVEL1_OPERATOR	Check level 1 operators. */
#define CG_LEVEL1_OPERATOR(c)	(c == '&' || c == '|')
/*! @define CG_LEVEL2_OPERATOR	Check level 2 operators. */
#define CG_LEVEL2_OPERATOR(c)	(c == '=' || c == '<' || c == '>')
/*! @define CG_LEVEL3_OPERATOR	Check level 3 operators. */
#define CG_LEVEL3_OPERATOR(c)	(c == '+' || c == '-')
/*! @define CG_LEVEL4_OPERATOR	Check level 4 operators. */
#define CG_LEVEL4_OPERATOR(c)	(c == '*' || c == '/' || c == '%')


/* ************* structures for expression parser ************* */

/*!
 * @typedef	cg_expr_t
 *		Main structure for expressions parser.
 * @field	vars		Vector of variables.
 * @field	funcs		Vector of functions.
 * @field	precision	Number of digits after floating point.
 */
typedef struct cg_expr_s
{
  yvect_t vars;		/* cg_expr_var_t */
  yvect_t funcs;	/* cg_expr_func_t */
  int precision;
} cg_expr_t;

/*!
 * @typedef	cg_expr_type_t
 *		Type of expression variable.
 * @constant	CG_EXPR_SCALAR	Constant value.
 * @constant	CG_EXPR_VALUE	Length value.
 * @constant	CG_EXPR_STRING	Character string.
 * @constant	CG_EXPR_OBJECT	Graphic object.
 * @constant	CG_EXPR_COLOR	Color.
 * @constant	CG_EXPR_BOOL	Boolean.
 */
typedef enum cg_expr_type_e
{
  CG_EXPR_SCALAR = 0,
  CG_EXPR_VALUE,
  CG_EXPR_STRING,
  CG_EXPR_ELEMENT,
  CG_EXPR_COLOR,
  CG_EXPR_BOOL
} cg_expr_type_t;

/*!
 * @typedef	cg_expr_var_t
 *		Structure for expressions variables.
 * @field	name		Variable's name.
 * @field	type		Variable's type.
 * @field	element		Variable's element type.
 * @field	const_value	Variable's constant value.
 * @field	value		Variable's value.
 * @field	s		String value.
 */
typedef struct cg_expr_var_s
{
  char *name;
  cg_expr_type_t type;
  union {
    struct {
      double red;
      double green;
      double blue;
    } color;
    struct {
      yvalue_t width;
      yvalue_t height;
    } element;
    double scalar;
    yvalue_t value;
    ybool_t boolean;
    char *string;
  } value;
} cg_expr_var_t;

/* **************************** MAIN STRUCTURES ******************* */

/*!
 * @typedef	cg_pdf_version_t
 *		PDF norm version.
 * @constant	PDF_14	PDF version 1.4 (Acrobat 5).
 * @constant	PDF_13	PDF version 1.3 (Acrobat 4).
 * @constant	PDF_15	PDF version 1.5 (Acrobat 6).
 */
typedef enum cg_pdf_version_e
{
  PDF_14 = 0,
  PDF_13,
  PDF_15
} cg_pdf_version_t;

/*!
 * @typedef	cg_paper_t
 *		Paper sizes.
 * @constant	PRINTER		320x450mm.
 */
typedef enum cg_paper_e
{
  A4 = 0, FOUR_A0, TWO_A0, A0, A1, A2, A3, A5, A6, A7, A8, A9, A10,
  RA0, RA1, RA2, RA3, RA4,
  SRA0, SRA1, SRA2, SRA3, SRA4,
  B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10,
  C0, C1, C2, C3, C4, C5, C6, C7, C8, C9, C10,
  D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10,
  LETTER, LEGAL, LEDGER, SEMI_LETTER, EXECUTIVE, TABLOID,
  DL, COM10, MONARCH,
  E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10,
  E_A3, E_A3_2, E_A4, E_A4_2, E_A5, E_A5_2, E_A6, E_A7,
  E_B4, E_B4_2, E_B5, E_B6, E_B7, E_B8,
  ID1, ID2, ID3,
  BUSINESS_CARD
} cg_paper_t;

/*!
 * @typedef	cg_back_t
 *		Setup the cards' back.
 * @constant	BACK_NO		No back.
 * @constant	BACK_WIDTH	Turn side on the width side.
 * @constant	BACK_HEIGHT	Turn side on the height side.
 */
typedef enum cg_back_e
{
  BACK_NO = 0,
  BACK_WIDTH,
  BACK_HEIGHT
} cg_back_t;

/*!
 * @typedef	cg_font_t
 *		Structure for font handling.
 * @field	id	Font's name.
 * @field	f	Handler to the opened font.
 * @field	metrics	Path to the font metric's file.
 * @field	outline	Path to the font outline's file.
 */
typedef struct cg_font_s
{
  char *id;
  int f;
  char *metrics;
  char *outline;
} cg_font_t;

/*!
 * @typedef	cg_text_alignment_t
 *		Enumeration of text alignment types.
 * @constant	ALIGN_LEFT	Left.
 * @constant	ALIGN_RIGHT	Right.
 * @constant	ALIGN_CENTER	Center.
 * @constant	ALIGN_JUSTIFY	Justify.
 */
typedef enum cg_text_alignement_e
{
  ALIGN_LEFT = 0,
  ALIGN_RIGHT,
  ALIGN_CENTER,
  ALIGN_JUSTIFY
} cg_text_alignement_t;

/*!
 * @typedef	cg_fontdef_t
 *		Structure for font definition.
 * @field	id		Font's identifier.
 * @field	fontname	Font to use.
 * @field	fontsize	Font's size.
 * @field	fill_color	Text's fill color.
 * @field	stroke_color	Text's stroke color.
 * @field	line_space	Space between lines.
 * @field	underline	Boolean.
 * @field	overline	Boolean.
 * @field	strikeout	Boolean.
 * @field	char_space	Space between characters.
 * @field	word_space	Space between words.
 * @field	h_scale		Horizontal scale.
 * @field	rise		Distance between text and baseline.
 * @field	align		Text's alignment.
 * @field	para_indent	Paragraph indentation.
 * @field	adapt		Boolean: adapt text to box.
 */
typedef struct cg_fontdef_s
{
  char *id;
  char *fontname;		/* MANDATORY */
  yvalue_t fontsize;		/* MANDATORY */
  cg_expr_var_t fill_color;	/* black */
  cg_expr_var_t stroke_color;	/* black */
  double line_space;		/* 1.0 */
  ybool_t underline;		/* false */
  ybool_t overline;		/* false */
  ybool_t strikeout;		/* false */
  yvalue_t char_space;		/* 0.0 */
  yvalue_t word_space;		/* 0.0 */
  double h_scale;		/* 100.0 */
  yvalue_t rise;		/* 0.0 */
  cg_text_alignement_t align;	/* left */
  yvalue_t para_indent;		/* 0.0 */
  ybool_t adapt;		/* false */
} cg_fontdef_t;

/*!
 * @typedef	cg_image_t
 *		Structure for image handling.
 * @field	id	Image's name.
 * @field	i	Handler to the opened image.
 * @field	width	Default width of the image.
 * @field	height	Default height of the image.
 * @field	file	Path to the image's file.
 * @field	mask	Path to the image mask's file.
 * @field	m	Handler to the opened image mask.
 * @field	mask_id	Image mask identifier.
 */
typedef struct cg_image_s
{
  char *id;
  int i;
  yvalue_t width;
  yvalue_t height;
  char *file;
  char *mask;
  int m;
  char *mask_id;
} cg_image_t;

/*!
 * @typedef	cg_deck_t
 *		Structure of one deck.
 * @field	paper_width	Paper's width.
 * @field	paper_height	Paper's height.
 * @field	paper_margin_w	Paper's right and left margins.
 * @field	paper_margin	Paper's top and bottom margins.
 * @field	card_width	Card's width.
 * @field	card_height	Card's height.
 * @field	space_width	Horizontal space between cards.
 * @field	space_height	Vertical space between cards.
 * @field	cols		Number of colons on each sheet.
 * @field	rows		Number of rows on each sheet.
 * @field	ditch_odd	Odd hiiden-ditch color.
 * @field	ditch_even	Even hidden-ditch color.
 * @field	cards		Vector of deck's cards.
 * @field	line_nbr	Line number in XML file.
 */
typedef struct cg_deck_s
{
  yvalue_t paper_width;
  yvalue_t paper_height;
  yvalue_t paper_margin_w;
  yvalue_t paper_margin_h;
  yvalue_t card_width;
  yvalue_t card_height;
  yvalue_t space_width;
  yvalue_t space_height;
  int cols;
  int rows;
  char *ditch_odd;
  char *ditch_even;
  yvect_t cards;	/* ydom_node_t* */
  int line_nbr;
} cg_deck_t;

/*!
 * @typedef	cg_t
 *		Main structure of Carta-Genius program.
 * @field	p		PDFLib object.
 * @field	xml_doms	Input files' XML DOM object.
 * @field	pdf_version	Used PDF version.
 * @field	author		Document's creator.
 * @field	title		Document's title.
 * @field	subject		Document's subject.
 * @field	keywords	Document's keywords.
 * @field	copyright	Document's copyright.
 * @field	version		Document's version.
 * @field	language	Document's language.
 * @field	note		Document's note.
 * @field	master_password	Encryption's master password.
 * @field	user_password	Encryption's user password.
 * @field	default_unit	Default unit.
 * @field	default_font	Default font's name.
 * @field	templates	Vector of templates.
 * @field	fonts		Vector of fonts.
 * @field	images		Vector of images.
 * @field	masks		Vector if image masks.
 * @field	decks		Vector of decks.
 * @field	do_back		Cards' back configuration.
 * @field	reverse		Page order.
 * @field	expr		Expression evaluation engine.
 * @field	inc_paths	Paths for file inclusion.
 * @field	img_paths	Paths for images.
 * @field	font_paths	Paths for fonts.
 * @field	text_defines	Defined texts.
 * @field	font_defines	Defined fonts.
 */
typedef struct cg_s
{
  PDF *p;
  yvect_t xml_doms;	/* ydom_t* */
  cg_pdf_version_t pdf_version;
  char *author;
  char *title;
  char *subject;
  char *keywords;
  char *copyright;
  char *version;
  char *language;
  char *note;
  char *master_password;
  char *user_password;
  yunit_t default_unit;
  char *default_font;
  yvect_t templates;	/* ydom_node_t* */
  yvect_t variables;	/* ydom_node_t* */
  yvect_t fonts;	/* cg_font_t* */
  yvect_t images;	/* cg_image_t* */
  yvect_t masks;	/* cg_image_t* */
  yvect_t decks;	/* cg_deck_t* */
  cg_back_t do_back;
  ybool_t reverse;
  struct cg_expr_s *expr;
  yvect_t inc_paths;	/* char* */
  yvect_t img_paths;	/* char* */
  yvect_t font_paths;	/* char* */
  yvect_t text_defines;	/* cg_text_define_t* */
  yvect_t font_defines; /* cg_fontdef_t* */
} cg_t;

/* ************* expression parser (again ) ********* */

/*!
 * @typedef	cg_expr_func_ptr_t
 *		Function pointer.
 * @param	carta	Pointer to the main program's structure.
 * @param	result	Pointer to the resulting variable.
 * @param	...	Other parameters.
 * @return	YENOERR if OK.
 */
typedef yerr_t (*cg_expr_func_ptr_t)(cg_t *carta, cg_expr_var_t *result, ...);

/*!
 * @typedef	cg_expr_func_t
 *		Structure for expressions functions.
 * @field	name		Function's name.
 * @field	nbr_params	Number of function's parameters.
 * @field	f		Pointer to the function.
 */
typedef struct cg_expr_func_s
{
  char *name;
  int nbr_params;
  cg_expr_func_ptr_t f;
} cg_expr_func_t;

/* ************* structures for wording ************* */

/*!
 * @typedef	cg_text_define_t
 *		A rich-text definition.
 * @field	id	Text's name.
 * @field	blocks	Array of text blocks.
 */
typedef struct cg_text_define_s
{
  char *id;
  yvect_t blocks;	/* cg_text_block_t* */
} cg_text_define_t;

/*!
 * @typedef	cg_text_block_t
 *		A text block.
 * @field	fontname	Font to use.
 * @field	fontsize	Font's size.
 * @field	color		Text's color.
 * @field	space		Lines space.
 * @field	underline	Boolean.
 * @field	overline	Boolean.
 * @field	strikeout	Boolean.
 * @field	char_space	Characters space.
 * @field	h_scale		Horizontal scale.
 * @field	parts		Block's text parts.
 */
typedef struct cg_text_block_s
{
  cg_fontdef_t *font;
  /*
  char *fontname;
  yvalue_t fontsize;
  cg_expr_var_t color;
  double space;
  ybool_t underline;
  ybool_t overline;
  ybool_t strikeout;
  yvalue_t char_space;
  double h_scale;
  */
  yvect_t parts;	/* cg_text_part_t */
} cg_text_block_t;

/*!
 * @typedef	cg_text_part_type_t
 *		Enumeration of text part types.
 * @constant	TEXT_PART_STRING	Character string.
 * @constant	TEXT_PART_PARA		New paragraph.
 * @constant	TEXT_PART_BR		New line.
 */
typedef enum cg_text_part_type_e
{
  TEXT_PART_STRING = 0,
  TEXT_PART_PARA,
  TEXT_PART_BR
} cg_text_part_type_t;

/*!
 * @typedef	cg_text_part_t
 *		A text part.
 * @field	type	The part's type.
 * @field	string	The content if it is a string.
 */
typedef struct cg_text_part_s
{
  cg_text_part_type_t type;
  ystr_t string;
} cg_text_part_t;

/* ************************ functions ************************** */

/* text_define.c */
cg_fontdef_t *cg_new_fontdef(void);
void cg_del_fontdef(cg_fontdef_t *def);
yerr_t cg_put_text_define(cg_t *carta, ydom_node_t *node);
yerr_t cg_put_text_area(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
			ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_write_words_in_area(cg_t *carta, cg_text_define_t *td,
			      yvalue_t x, yvalue_t y, yvalue_t w, yvalue_t h);
void free_text_define(void *td, void *data);
void free_text_block(void *tb, void *data);
void free_text_part(void *tp, void *data);

/* expr.c */
yerr_t cg_expr(cg_t *carta, char *expr, cg_expr_var_t *result,
	       yunit_t default_unit);
yerr_t cg_expr_get_operand(cg_t *carta, char **expr, cg_expr_var_t *result);
yerr_t cg_expr_eval(cg_t *carta, char **expr, cg_expr_var_t *result);
yerr_t cg_expr_do_func(cg_t *carta, char *name, char **expr,
		       cg_expr_var_t *result);
yerr_t cg_expr_init(cg_t *carta);
void free_var(void *var, void *data);
void free_func(void *func, void *data);
yerr_t cg_expr_set_precision(cg_t *carta, char *str);
yerr_t cg_expr_set_bool(cg_t *carta, char *name, ybool_t boolean);
yerr_t cg_expr_set_scalar(cg_t *carta, char *name, double scalar);
yerr_t cg_expr_set_value(cg_t *carta, char *name, yvalue_t value);
yerr_t cg_expr_set_string(cg_t *carta, char *name, char *string);
yerr_t cg_expr_set_element(cg_t *carta, char *name, yvalue_t width,
			   yvalue_t height);
yerr_t cg_expr_set_color(cg_t *carta, char *name, double red,
			 double green, double blue);
yerr_t cg_expr_set_func(cg_t *carta, char *name, int nbr_params,
			cg_expr_func_ptr_t f);
yerr_t cg_expr_get_var(cg_t *carta, char *name, cg_expr_var_t *result);
yerr_t cg_expr_get_func(cg_t *carta, char *name, int nbr_params,
			cg_expr_func_t *result);
yerr_t cg_expr_f_var(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_width(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_height(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_landscape(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_plus(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_minus(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_mult(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_div(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_mod(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_sqrt(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_pow(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_strlen(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_strwidth(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_sin(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_cos(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_tan(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_asin(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_acos(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_atan(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_string(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_bool(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_scalar(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_value(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_element(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_color(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_min(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_max(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_strftime(cg_t*carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_time(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_srand(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_rand(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_not(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_and(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_or(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_equal(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_greater_than(cg_t *carta, cg_expr_var_t *result, ...);
yerr_t cg_expr_f_less_than(cg_t *carta, cg_expr_var_t *result, ...);


/* conf.c */
cg_t *cg_init(char *input, const char *inc_paths,
	      const char *font_paths, const char *img_paths);
yerr_t cg_read_conf(cg_t *carta, const char *input, ybool_t initial);
yerr_t cg_compute_cols_rows(cg_t *carta);
void import_font(cg_t *carta, ydom_node_t *node);
void import_image(cg_t *carta, ydom_node_t *node, ybool_t is_mask);
void include_file(cg_t *carta, const char *file);
void add_font_paths(cg_t *carta, const char *paths);
void add_img_paths(cg_t *carta, const char *paths);
void add_inc_paths(cg_t *carta, const char *paths);
void free_dom(void *dom, void *data);
void free_deck(void *deck, void *data);
void free_image(void *image, void *data);
void free_font(void *font, void *data);
void free_path(void *path, void *data);

/* paper.c */
cg_paper_t cg_get_paper(const char *paper_type);
yvalue_t cg_get_paper_width(cg_paper_t paper);
yvalue_t cg_get_paper_height(cg_paper_t paper);

/* document.c */
yerr_t cg_create_document(char *output, cg_t *carta);
int cg_deck_get_nb_cards(cg_t *carta, cg_deck_t *deck);
ydom_node_t* cg_deck_get_card(cg_t *carta, cg_deck_t *deck, int num);
yerr_t cg_process_deck(cg_t *carta, cg_deck_t *deck);
yerr_t cg_check_paper_size(cg_t *carta, cg_deck_t *deck);
yerr_t cg_process_deck_fronts(cg_t *carta, cg_deck_t *deck, int page,
			      int cards_per_page);
yerr_t cg_process_deck_backs(cg_t *carta, cg_deck_t *deck, int page,
			     int cards_per_page);
yerr_t cg_new_page(cg_t *carta, cg_deck_t *deck);
yerr_t cg_cut_lines(cg_t *carta, cg_deck_t *deck, cg_back_t back);
yerr_t cg_swip_page(cg_t *carta, cg_deck_t *deck);
yerr_t cg_load_variables(cg_t *carta);
yerr_t cg_load_fonts(cg_t *carta);
yerr_t cg_load_images(cg_t *carta);
yerr_t cg_load_image(cg_t *carta, cg_image_t *image, ybool_t is_mask);
yerr_t cg_unload_images(cg_t *carta);
yerr_t cg_unload_image(cg_t *carta, cg_image_t *image);

/* card.c */
yerr_t cg_process_card(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		       yvalue_t card_x, yvalue_t card_y);
yerr_t cg_process_back(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		       yvalue_t card_x, yvalue_t card_y);
yerr_t cg_process_template(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
			   char *templates, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_process_if(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		     ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_process_while(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
			ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
int cg_card_count(cg_t *carta, ydom_node_t *card);
yerr_t cg_put_var(cg_t *carta, ydom_node_t *node);
yerr_t cg_put_log(cg_t *carta, ydom_node_t *node);
yerr_t cg_put_round_box(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
			ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_put_line(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		   ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_put_bezier(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		     ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_put_polygon(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		      ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_put_grid(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		   ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_put_hexagon(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		      ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_put_circle(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		     ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_put_box(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		  ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_put_text(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		   ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);
yerr_t cg_rotate(yvalue_t *x, yvalue_t *y, double angle);
int cg_get_font(cg_t *carta, const char *fontname);
cg_image_t *cg_get_image(cg_t *carta, const char *id);
yerr_t cg_put_image(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		    ydom_node_t *node, yvalue_t card_x, yvalue_t card_y);

#endif /* __CARTAGENIUS_H__ */
