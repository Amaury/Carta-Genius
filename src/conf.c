#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cartagenius.h"

/*
** cg_init()
** Initialize the Carta object and launch the configuration file parsing.
*/
cg_t *cg_init(char *input, const char *inc_paths,
	      const char *font_paths, const char *img_paths)
{
  cg_t *carta;
  ystr_t ys;

  /* init */
  if (!(carta = malloc0(sizeof(cg_t))))
    {
      YLOG_ADD(YLOG_ERR, "Memory error");
      return (NULL);
    }
  if (!(ys = ys_new("")) ||
      !(carta->xml_doms = yv_new()) ||
      !(carta->fonts = yv_new()) ||
      !(carta->images = yv_new()) ||
      !(carta->masks = yv_new()) ||
      !(carta->templates = yv_new()) ||
      !(carta->variables = yv_new()) ||
      !(carta->decks = yv_new()) ||
      !(carta->inc_paths = yv_new()) ||
      !(carta->img_paths = yv_new()) ||
      !(carta->font_paths = yv_new()) ||
      !(carta->text_defines = yv_new()))
    {
      YLOG_ADD(YLOG_ERR, "Memory error");
      ys_del(&ys);
      yv_del(&carta->xml_doms, NULL, NULL);
      yv_del(&carta->fonts, NULL, NULL);
      yv_del(&carta->images, NULL, NULL);
      yv_del(&carta->masks, NULL, NULL);
      yv_del(&carta->templates, NULL, NULL);
      yv_del(&carta->variables, NULL, NULL);
      yv_del(&carta->decks, NULL, NULL);
      yv_del(&carta->inc_paths, NULL, NULL);
      yv_del(&carta->img_paths, NULL, NULL);
      yv_del(&carta->font_paths, NULL, NULL);
      free0(carta);
      return (NULL);
    }
  /* extract path directories */
  add_inc_paths(carta, inc_paths);
  add_font_paths(carta, font_paths);
  add_img_paths(carta, img_paths);

  /* initialize expression evaluation engine */
  if (cg_expr_init(carta) != YENOERR)
    YLOG_ADD(YLOG_WARN, "Parser init error.");

  /* read input file */
  if (cg_read_conf(carta, input, YTRUE) != YENOERR)
    {
      YLOG_ADD(YLOG_ERR, "Problem during input file processing.");
      yv_del(&carta->xml_doms, NULL, NULL);
      yv_del(&carta->fonts, NULL, NULL);
      yv_del(&carta->images, NULL, NULL);
      yv_del(&carta->masks, NULL, NULL);
      yv_del(&carta->templates, NULL, NULL);
      yv_del(&carta->variables, NULL, NULL);
      yv_del(&carta->decks, NULL, NULL);
      yv_del(&carta->inc_paths, NULL, NULL);
      yv_del(&carta->img_paths, NULL, NULL);
      yv_del(&carta->font_paths, NULL, NULL);
      yv_del(&carta->text_defines, NULL, NULL);
      if (carta->expr)
	{
	  yv_del(&carta->expr->vars, free_var, NULL);
	  yv_del(&carta->expr->funcs, free_func, NULL);
	  free0(carta->expr);
	}
      free0(carta);
    }
  return (carta);
}

/*
** cg_read_conf()
** Read the configuration file.
*/
yerr_t cg_read_conf(cg_t *carta, const char *input, ybool_t initial)
{
  ydom_t *dom;
  ydom_node_t *root_node;
  yvect_t array;
  int i;
  char *pt = NULL, *pt2 = NULL;

  if (!(dom = ydom_new()))
    {
      YLOG_ADD(YLOG_ERR, "Memory error");
      return (YENOMEM);
    }
  if (ydom_read_file(dom, input) != YENOERR)
    {
      YLOG_ADD(YLOG_ERR, "Problem with file '%s'", input);
      ydom_del(dom);
      return (YEINVAL);
    }
  if (!(array = ydom_xpath(dom, "/pandocreon:carta-genius")) ||
      !yv_len(array) || !array[0])
    {
      YLOG_ADD(YLOG_ERR, "No configuration");
      yv_del(&array, NULL, NULL);
      ydom_del(dom);
      return (YEINVAL);
    }
  root_node = array[0];
  yv_del(&array, NULL, NULL);
  yv_add(&carta->xml_doms, dom);

  /* get document info */
  if ((array = ydom_node_xpath(root_node, "info")) &&
      yv_len(array) && array[0])
    {
      carta->author = ydom_node_get_attr_value(array[0], "author");
      carta->title = ydom_node_get_attr_value(array[0], "title");
      carta->subject = ydom_node_get_attr_value(array[0], "subject");
      carta->keywords = ydom_node_get_attr_value(array[0], "keywords");
      carta->copyright = ydom_node_get_attr_value(array[0], "copyright");
      carta->version = ydom_node_get_attr_value(array[0], "version");
      carta->language = ydom_node_get_attr_value(array[0], "lang");
      carta->note = ydom_node_get_attr_value(array[0], "note");
    }
  yv_del(&array, NULL, NULL);

  /* get PDF version */
  if ((array = ydom_node_xpath(root_node, "pdf")) &&
      yv_len(array) && array[0])
    {
      char *passwd1 = NULL, *passwd2 = NULL;
      pt = ydom_node_get_attr_value(array[0], "version");
      passwd1 = ydom_node_get_attr_value(array[0], "master-password");
      passwd2 = ydom_node_get_attr_value(array[0], "user-password");
      if (!passwd1 || !passwd2 || !strcmp(passwd1, passwd2))
	{
	  free0(passwd1);
	  free0(passwd2);
	}
      carta->master_password = passwd1;
      carta->user_password = passwd2;
    }
  if (pt && !strcmp(pt, "1.3"))
    carta->pdf_version = PDF_13;
  else if (pt && !strcmp(pt, "1.5"))
    carta->pdf_version = PDF_15;
  else
    carta->pdf_version = PDF_14;
  free0(pt);
  yv_del(&array, NULL, NULL);

  /* get general configuration */
  if (initial)
    {
      /* get default unit */
      if ((array = ydom_node_xpath(root_node, "unit")) &&
	  yv_len(array) && array[0] &&
	  (pt = ydom_node_get_attr_value(array[0], "base")))
	{
	  if (!strcasecmp(pt, "mm") || !strcasecmp(pt, "millimeter"))
	    carta->default_unit = YUNIT_MM;
	  else if (!strcasecmp(pt, "cm") || !strcasecmp(pt, "centimeter"))
	    carta->default_unit = YUNIT_CM;
	  else if (!strcasecmp(pt, "in") || !strcasecmp(pt, "inch"))
	    carta->default_unit = YUNIT_IN;
	  else if (!strcasecmp(pt, "pt") || !strcasecmp(pt, "point"))
	    carta->default_unit = YUNIT_PT;
	  else
	    {
	      YLOG_ADD(YLOG_ERR, "Bad default unit [line %d]",
		       ((ydom_node_t*)array[0])->line_nbr);
	      free0(pt);
	      yv_del(&array, NULL, NULL);
	      ydom_del(dom);
	      return (YEINVAL);
	    }
	  free0(pt);
	}
      else
	{
	  YLOG_ADD(YLOG_INFO, "No default unit. Set to mm.");
	  carta->default_unit = YUNIT_MM;
	}
      yv_del(&array, NULL, NULL);
      
      /* get cards' back configuration */
      if ((array = ydom_node_xpath(root_node, "back")) &&
	  yv_len(array) && array[0])
	{
	  if ((pt = ydom_node_get_attr_value(array[0], "side")))
	    {
	      if (!strcasecmp(pt, "width"))
		carta->do_back = BACK_WIDTH;
	      else if (!strcasecmp(pt, "height"))
		carta->do_back = BACK_HEIGHT;
	      free0(pt);
	    }
	  else
	    carta->do_back = BACK_NO;
	  if ((pt = ydom_node_get_attr_value(array[0], "reverse")) &&
	      (!strcasecmp(pt, "yes") || !strcasecmp(pt, "true")))
	    carta->reverse = YTRUE;
	  free0(pt);
	}
      else
	carta->do_back = BACK_NO;
      yv_del(&array, NULL, NULL);
    }

  /* get templates */
  if ((array = ydom_node_xpath(root_node, "templates/template")) &&
      yv_len(array))
    {
      int i;

      for (i = 0; i < yv_len(array); ++i)
	yv_add(&carta->templates, array[i]);
    }
  yv_del(&array, NULL, NULL);

  /* get init variables */
  if ((array = ydom_node_xpath(root_node, "variables/var")) &&
      yv_len(array))
    {
      int i;

      for (i = 0; i < yv_len(array); ++i)
	yv_add(&carta->variables, array[i]);
    }
  yv_del(&array, NULL, NULL);

  /* get default font */
  if ((array = ydom_node_xpath(root_node, "fonts")) &&
      yv_len(array) && array[0])
    {
      char *default_font;
      
      if ((default_font = ydom_node_get_attr_value(array[0], "default")))
	{
	  free0(carta->default_font);
	  carta->default_font = default_font;
	}
    }
    yv_del(&array, NULL, NULL);

  /* get fonts */
  if ((array = ydom_node_xpath(root_node, "fonts/*")) &&
      yv_len(array) && array[0])
    {
      int i;

      for (i = 0; i < yv_len(array); ++i)
	{
	  char *tag_name = ydom_node_get_name(array[i]);
	  if (tag_name && !strcmp(tag_name, "font"))
	    import_font(carta, array[i]);
	  else if (tag_name && !strcmp(tag_name, "path"))
	    {
	      char *dir = ydom_node_get_attr_value(array[i], "dir");
	      if (dir && strlen(dir))
		add_font_paths(carta, dir);
	      else
		YLOG_ADD(YLOG_WARN, "Path without 'dir' argument [line %d].",
			 ((ydom_node_t*)array[i])->line_nbr);
	      free0(dir);
	    }
	  free0(tag_name);
	}
    }
  yv_del(&array, NULL, NULL);

  /* get images */
  if ((array = ydom_node_xpath(root_node, "images/*")) &&
      yv_len(array) && array[0])
    {
      int i;

      for (i = 0; i < yv_len(array); ++i)
	{
	  char *tag_name = ydom_node_get_name(array[i]);
	  if (tag_name && !strcmp(tag_name, "image"))
	    import_image(carta, array[i], YFALSE);
	  else if (tag_name && !strcmp(tag_name, "mask"))
	    import_image(carta, array[i], YTRUE);
	  else if (tag_name && !strcmp(tag_name, "path"))
	    {
	      /* add a directory for path searching */
	      char *dir = ydom_node_get_attr_value(array[i], "dir");
	      if (dir && strlen(dir))
		add_img_paths(carta, dir);
	      else
		YLOG_ADD(YLOG_WARN, "Path without 'dir' argument [line %d].",
			 ((ydom_node_t*)array[i])->line_nbr);
	      free0(dir);
	    }
	  free0(tag_name);
	}
    }
  yv_del(&array, NULL, NULL);

  /* get all decks */
  array = ydom_node_xpath(root_node, "deck");
  for (i = 0; i < yv_len(array); ++i)
    {
      cg_deck_t *deck;
      ydom_node_t *deck_node = array[i];
      yvect_t array2;

      deck = malloc0(sizeof(cg_deck_t));
      deck->line_nbr = deck_node->line_nbr;
      /* get paper size */
      if ((array2 = ydom_node_xpath(deck_node, "paper")) &&
	  yv_len(array2) && array2[0])
	{
	  cg_expr_var_t e_w = {0}, e_h = {0};
	  yvalue_t width, height, margin_w, margin_h;
	  
	  /* get width and height */
	  if ((pt = ydom_node_get_attr_value(array2[0], "type")))
	    {
	      cg_paper_t paper_type = cg_get_paper(pt);
	      width = cg_get_paper_width(paper_type);
	      height = cg_get_paper_height(paper_type);
	      if ((pt2 = ydom_node_get_attr_value(array2[0], "landscape")) &&
		  (!strcasecmp(pt2, "yes") || !strcasecmp(pt2, "true")))
		{
		  yvalue_t tmp_val = width;
		  width = height;
		  height = tmp_val;
		}
	    }
	  else if ((pt = ydom_node_get_attr_value(array2[0], "width")) &&
		   cg_expr(carta, pt, &e_w, carta->default_unit) == YENOERR &&
		   e_w.type == CG_EXPR_VALUE &&
		   (pt2 = ydom_node_get_attr_value(array2[0], "height")) &&
		   cg_expr(carta, pt2, &e_h, carta->default_unit) == YENOERR &&
		   e_h.type == CG_EXPR_VALUE)
	    {
	      width = e_w.value.value;
	      height = e_h.value.value;
	    }
	  else
	    {
	      YLOG_ADD(YLOG_WARN, "No valid paper size. Set to a4.");
	      width = cg_get_paper_width(A4);
	      height = cg_get_paper_height(A4);
	    }
	  free0(pt);
	  free0(pt2);
	  free_var(&e_w, &e_w);
	  free_var(&e_h, &e_h);
	  /* get margin */
	  if ((pt = ydom_node_get_attr_value(array2[0], "margin")))
	    margin_w = margin_h = yvalue_read(pt, carta->default_unit);
	  else
	    margin_w = margin_h = yvalue_read(DEFAULT_MARGIN, carta->default_unit);
	  free0(pt);
	  if ((pt = ydom_node_get_attr_value(array2[0], "margin-width")) &&
	      cg_expr(carta, pt, &e_w, carta->default_unit) == YENOERR &&
	      e_w.type == CG_EXPR_VALUE)
	    margin_w = e_w.value.value;
	  free0(pt);
	  if ((pt = ydom_node_get_attr_value(array2[0], "margin-height")) &&
	      cg_expr(carta, pt, &e_h, carta->default_unit) == YENOERR &&
	      e_h.type == CG_EXPR_VALUE)
	    margin_h = e_h.value.value;
	  free0(pt);
	  free_var(&e_w, &e_w);
	  free_var(&e_h, &e_h);
	  deck->paper_width = width;
	  deck->paper_height = height;
	  deck->paper_margin_w = margin_w;
	  deck->paper_margin_h = margin_h;
	}
      else
	{
	  YLOG_ADD(YLOG_INFO, "No paper size, set to default (A4).");
	  deck->paper_width = cg_get_paper_width(A4);
	  deck->paper_height = cg_get_paper_height(A4);
	  deck->paper_margin_w = deck->paper_margin_h =
	    yvalue_read(DEFAULT_MARGIN, carta->default_unit);
	}
      yv_del(&array2, NULL, NULL);

      /* get hidden ditch config */
      if ((array2 = ydom_node_xpath(deck_node, "hidden-ditch")) &&
	  yv_len(array2) && array2[0])
	{
	  /* get margin */
	  deck->ditch_even = ydom_node_get_attr_value(array2[0], "even");
	  deck->ditch_odd = ydom_node_get_attr_value(array2[0], "odd");
	}
      yv_del(&array2, NULL, NULL);

      /* get card size */
      if ((array2 = ydom_node_xpath(deck_node, "cardsize")) &&
	  yv_len(array2) && array2[0])
	{
	  yvalue_t width, height, space_w, space_h;

	  if ((pt = ydom_node_get_attr_value(array2[0], "type")))
	    {
	      cg_paper_t paper_type = cg_get_paper(pt);
	      width = cg_get_paper_width(paper_type);
	      height = cg_get_paper_height(paper_type);
	      if ((pt2 = ydom_node_get_attr_value(array2[0], "landscape")) &&
		  (!strcasecmp(pt2, "yes") || !strcasecmp(pt2, "true")))
		{
		  yvalue_t tmp_val = width;
		  width = height;
		  height = tmp_val;
		}
	    }
	  else if ((pt = ydom_node_get_attr_value(array2[0], "width")) &&
	      (pt2 = ydom_node_get_attr_value(array2[0], "height")))
	    {
	      width = yvalue_read(pt, carta->default_unit);
	      height = yvalue_read(pt2, carta->default_unit);
	    }
	  else
	    {
	      YLOG_ADD(YLOG_WARN, "No valid card size. Set to a6.");
	      width = cg_get_paper_width(A6);
	      height = cg_get_paper_height(A6);
	    }
	  free0(pt);
	  free0(pt2);
	  deck->card_width = width;
	  deck->card_height = height;
	  if ((pt = ydom_node_get_attr_value(array2[0], "space")))
	    space_w = space_h = yvalue_read(pt, carta->default_unit);
	  else if ((pt = ydom_node_get_attr_value(array2[0], "space-width")) &&
		   (pt2 = ydom_node_get_attr_value(array2[0], "space-height")))
	    {
	      space_w = yvalue_read(pt, carta->default_unit);
	      space_h = yvalue_read(pt2, carta->default_unit);
	    }
	  else
	    {
	      space_w.unit = space_h.unit = YUNIT_PT;
	      space_w.value = space_h.value = 0.0;
	    }
	  deck->space_width = space_w;
	  deck->space_height = space_h;
	  free0(pt);
	  free0(pt2);
	}
      else
	{
	  YLOG_ADD(YLOG_INFO, "No card size. Set to a6.");
	  deck->card_width = cg_get_paper_width(A6);
	  deck->card_height = cg_get_paper_height(A6);
	}
      yv_del(&array2, NULL, NULL);

      /* get cards */
      deck->cards = ydom_node_xpath(deck_node, "card");

      yv_add(&carta->decks, deck);
    }

  /* get included files */
  if ((array = ydom_node_xpath(root_node, "includes/*")) &&
      yv_len(array) && array[0])
    {
      int i;
      for (i = 0; i < yv_len(array); ++i)
	{
	  char *tag_name = ydom_node_get_name(array[i]);
	  if (tag_name && !strcmp(tag_name, "include"))
	    {
	      /* include a file */
	      char *file = ydom_node_get_attr_value(array[i], "file");
	      if (file && strlen(file))
		include_file(carta, file);
	      else
		YLOG_ADD(YLOG_WARN, "Include without 'file' argument [line %d].",
			 ((ydom_node_t*)array[i])->line_nbr);
	      free0(file);
	    }
	  else if (tag_name && !strcmp(tag_name, "path"))
	    {
	      /* add an include dir */
	      char *dir = ydom_node_get_attr_value(array[i], "dir");
	      if (dir && strlen(dir))
		add_inc_paths(carta, dir);
	      else
		YLOG_ADD(YLOG_WARN, "Path without 'dir' argument [line %d].",
			 ((ydom_node_t*)array[i])->line_nbr);
	      free0(dir);
	    }
	  free0(tag_name);
	}
    }
  yv_del(&array, NULL, NULL);

  return (YENOERR);
}

/*
** cg_compute_cols_rows()
** Compute number of colons and rows of every decks' pages.
*/
yerr_t cg_compute_cols_rows(cg_t *carta)
{
  double useable_w, useable_h;
  cg_deck_t *deck;
  int i;

  for (i = 0; i < yv_len(carta->decks); ++i)
    {
      deck = carta->decks[i];
      useable_w = yvalue_get(deck->paper_width, carta->default_unit) -
	(2 * yvalue_get(deck->paper_margin_w, carta->default_unit));
      useable_h = yvalue_get(deck->paper_height, carta->default_unit) -
	(2 * yvalue_get(deck->paper_margin_h, carta->default_unit));
      deck->cols = (int)(useable_w / yvalue_get(deck->card_width,
						carta->default_unit));
      deck->rows = (int)(useable_h / yvalue_get(deck->card_height,
						carta->default_unit));
      /* adjust depending from space between cards */
      while (deck->cols > 0)
	{
	  double total_width;
	  total_width = deck->cols *
	    yvalue_get(deck->card_width, carta->default_unit);
	  total_width += (deck->cols - 1) *
	    yvalue_get(deck->space_width, carta->default_unit);
	  if (total_width <= useable_w)
	    break ;
	  deck->cols--;
	}
      while (deck->rows > 0)
	{
	  double total_height;
	  total_height = deck->rows *
	    yvalue_get(deck->card_height, carta->default_unit);
	  total_height += (deck->rows - 1) *
	    yvalue_get(deck->space_height, carta->default_unit);
	  if (total_height <= useable_h)
	    break ;
	  deck->rows--;
	}
      /* check cards */
      if (!yv_len(deck->cards))
	{
	  YLOG_ADD(YLOG_WARN, "No card in deck [line %d]. Deck skipped.",
		   deck->line_nbr);
	  deck = yv_ext(carta->decks, i);
	  free_deck(deck, NULL);
	  i--;
	}
      /* check paper size */
      if (!deck->cols || !deck->rows)
	{
	  YLOG_ADD(YLOG_WARN, "Cards bigger than paper size [line %d]. Deck skipped.",
		   deck->line_nbr);
	  deck = yv_ext(carta->decks, i);
	  free_deck(deck, NULL);
	  i--;
	}
    }
  /* check decks */
  if (!yv_len(carta->decks))
    {
      YLOG_ADD(YLOG_ERR, "No usable deck. Abort.");
      return (YENOENT);
    }
  return (YENOERR);
}

/*
** import_font()
** Try to import a given font.
*/
void import_font(cg_t *carta, ydom_node_t *node)
{
  struct stat st;
  cg_font_t *font;

  if (!(font = malloc0(sizeof(cg_font_t))))
    return ;
  font->id = ydom_node_get_attr_value(node, "id");
  font->outline = ydom_node_get_attr_value(node, "outline");
  font->metrics = ydom_node_get_attr_value(node, "metrics");
  if (!font->id || !font->outline || !font->metrics)
    {
      YLOG_ADD(YLOG_WARN, "Empty font declaration [line %d].",
	       node->line_nbr);
      free0(font->outline);
      free0(font->id);
      free0(font);
      return ;
    }
  font->f = -1;
  if (!stat(font->outline, &st) && !stat(font->metrics, &st))
    {
      yv_add(&carta->fonts, font);
      return ;
    }
  else
    {
      ystr_t ys1 = ys_new(""), ys2 = ys_new("");
      int i;
      for (i = 0; i < yv_len(carta->font_paths); ++i)
	{
	  ys_printf(&ys1, "%s%c%s", carta->font_paths[i],
		    CARTA_SEP, font->outline);
	  ys_printf(&ys2, "%s%c%s", carta->font_paths[i],
		    CARTA_SEP, font->metrics);
	  if (!stat(ys1, &st) && !stat(ys2, &st))
	    {
	      free0(font->outline);
	      free0(font->metrics);
	      font->outline = ys_string(ys1);
	      font->metrics = ys_string(ys2);
	      yv_add(&carta->fonts, font);
	      ys_del(&ys1);
	      ys_del(&ys2);
	      return ;
	    }
	}
      ys_del(&ys1);
      ys_del(&ys2);
    }
  YLOG_ADD(YLOG_WARN, "Unable to find files '%s' and '%s' in font paths.",
	   font->outline, font->metrics);
  free_font(font, NULL);
}

/*
** import_image()
** Try to import a given image.
*/
void import_image(cg_t *carta, ydom_node_t *node, ybool_t is_mask)
{
  char *width = NULL, *height = NULL;
  struct stat st;
  cg_image_t *image;

  if (!(image = malloc0(sizeof(cg_image_t))))
    return ;
  image->id = ydom_node_get_attr_value(node, "id");
  image->file = ydom_node_get_attr_value(node, "file");
  if (!image->id || !image->file)
    {
      if (is_mask)
	YLOG_ADD(YLOG_WARN, "Empty image mask declaration [line %d].",
		 node->line_nbr);
      else
	YLOG_ADD(YLOG_WARN, "Empty image declaration [line %d].",
		 node->line_nbr);
      free0(image->id);
      free0(image);
      return ;
    }
  if (!is_mask)
    {
      image->mask = ydom_node_get_attr_value(node, "mask");
      image->mask_id = ydom_node_get_attr_value(node, "mask-id");
      width = ydom_node_get_attr_value(node, "width");
      height = ydom_node_get_attr_value(node, "height");
      image->width = yvalue_read(width, carta->default_unit);
      image->height = yvalue_read(height, carta->default_unit);
      free0(width);
      free0(height);
    }
  image->i = image->m = -1;
  if (!stat(image->file, &st))
    {
      if (is_mask)
	yv_add(&carta->masks, image);
      else
	yv_add(&carta->images, image);
      return ;
    }
  else
    {
      ystr_t ys = ys_new("");
      int i;
      for (i = 0; i < yv_len(carta->img_paths); ++i)
	{
	  ys_printf(&ys, "%s%c%s", carta->img_paths[i],
		    CARTA_SEP, image->file);
	  if (!stat(ys, &st))
	    {
	      free0(image->file);
	      image->file = ys_string(ys);
	      if (is_mask)
		yv_add(&carta->masks, image);
	      else
		yv_add(&carta->images, image);
	      ys_del(&ys);
	      return ;
	    }
	}
      ys_del(&ys);
    }
  YLOG_ADD(YLOG_WARN, "Unable to find file '%s' in image paths.", image->file);
  free_image(image, NULL);
}

/*
** include_file()
** Try to include a given file.
*/
void include_file(cg_t *carta, const char *file)
{
  struct stat st;

  if (!stat(file, &st))
    {
      cg_read_conf(carta, file, YFALSE);
      return ;
    }
  else
    {
      ystr_t ys = ys_new("");
      int i;
      for (i = 0; i < yv_len(carta->inc_paths); ++i)
	{
	  ys_printf(&ys, "%s%c%s", carta->inc_paths[i],
		    CARTA_SEP, file);
	  if (!stat(ys, &st))
	    {
	      cg_read_conf(carta, ys, YFALSE);
	      ys_del(&ys);
	      return ;
	    }
	}
      ys_del(&ys);
    }
  YLOG_ADD(YLOG_WARN, "Unable to find file '%s' in include paths.", file);
}

/*
** add_font_paths()
** Add path directories from string.
*/
void add_font_paths(cg_t *carta, const char *paths)
{
  const char *pt;
  ystr_t ys;

  if (!carta)
    {
      YLOG_ADD(YLOG_WARN, "No main structure.");
      return ;
    }
  if (!(ys = ys_new("")))
    {
      YLOG_ADD(YLOG_WARN, "Memory error");
      return ;
    }
  for (pt = paths; pt && *pt; ++pt)
    {
      ys_trunc(ys);
      while (*pt == SEMICOLON || *pt == COMMA)
	pt++;
      for (; *pt && *pt != SEMICOLON && *pt != COMMA; ++pt)
	ys_addc(&ys, *pt);
      yv_add(&carta->font_paths, ys_string(ys));
    }
  ys_del(&ys);
}

/*
** add_img_paths()
** Add path directories from string.
*/
void add_img_paths(cg_t *carta, const char *paths)
{
  const char *pt;
  ystr_t ys;

  if (!carta)
    {
      YLOG_ADD(YLOG_WARN, "No main structure.");
      return ;
    }
  if (!(ys = ys_new("")))
    {
      YLOG_ADD(YLOG_WARN, "Memory error");
      return ;
    }
  for (pt = paths; pt && *pt; ++pt)
    {
      ys_trunc(ys);
      while (*pt == SEMICOLON || *pt == COMMA)
	pt++;
      for (; *pt && *pt != SEMICOLON && *pt != COMMA; ++pt)
	ys_addc(&ys, *pt);
      yv_add(&carta->img_paths, ys_string(ys));
    }
  ys_del(&ys);
}

/*
** add_inc_paths()
** Add path directories from string.
*/
void add_inc_paths(cg_t *carta, const char *paths)
{
  const char *pt;
  ystr_t ys;

  if (!carta)
    {
      YLOG_ADD(YLOG_WARN, "No main structure.");
      return ;
    }
  if (!(ys = ys_new("")))
    {
      YLOG_ADD(YLOG_WARN, "Memory error");
      return ;
    }
  for (pt = paths; pt && *pt; ++pt)
    {
      ys_trunc(ys);
      while (*pt == SEMICOLON || *pt == COMMA)
	pt++;
      for (; *pt && *pt != SEMICOLON && *pt != COMMA; ++pt)
	ys_addc(&ys, *pt);
      yv_add(&carta->inc_paths, ys_string(ys));
    }
  ys_del(&ys);
}

/*
** free_dom()
** Free the memory allocated for an XML DOM object.
*/
void free_dom(void *dom, void *data)
{
  ydom_t *d = dom;

  if (!dom)
    return ;
  ydom_del(d);
}

/*
** free_deck()
** Free the memory allocated for a deck.
*/
void free_deck(void *deck, void *data)
{
  cg_deck_t *d = deck;

  if (!deck)
    return ;
  free0(d->ditch_odd);
  free0(d->ditch_even);
  free0(d);
}

/*
** free_image()
** Free the memory allocated for an image.
*/
void free_image(void *image, void *data)
{
  cg_image_t *i = image;

  if (!image)
    return ;
  free0(i->id);
  free0(i->file);
  free0(i->mask);
  free0(i->mask_id);
  free0(i);
}

/*
** free_font()
** Free the memory allocated for a font.
*/
void free_font(void *font, void *data)
{
  cg_font_t *f = font;

  if (!font)
    return ;
  free0(f->id);
  free0(f->metrics);
  free0(f->outline);
  free0(f);
}

/*
** free_path()
** Free the memory allocated for an include path.
*/
void free_path(void *path, void *data)
{
  char *p = path;

  if (!path)
    return ;
  free0(p);
}
