#include "cartagenius.h"

/*
** cg_create_document()
** Create the ouput PDF file.
*/
yerr_t cg_create_document(char *output, cg_t *carta)
{
  PDF *p;
  int i;
  yerr_t return_value = YENOERR;

  if (!(carta->p = p = PDF_new()))
    {
      YLOG_ADD(YLOG_ERR, "PDF allocation error.");
      return (YEUNDEF);
    }

  PDF_TRY(p)
    {
      ystr_t ys = ys_new("");
      /* document's initialization */
      ys_printf(&ys, "compatibility %s",
		(carta->pdf_version == PDF_13) ? "1.3" :
		(carta->pdf_version == PDF_15) ? "1.5" : "1.4");
      if (carta->master_password && carta->user_password)
	{
	  ys_cat(&ys, " masterpassword ");
	  ys_cat(&ys, carta->master_password);
	  ys_cat(&ys, " userpassword ");
	  ys_cat(&ys, carta->user_password);
	}
      if (PDF_begin_document(p, output, 0, ys) == -1)
	{
	  YLOG_ADD(YLOG_ERR, "Unable to create file '%s'", output);
	  PDF_delete(p);
	  ys_del(&ys);
	  return (YEUNDEF);
	}
      ys_del(&ys);
      PDF_set_info(p, "Creator", DEFAULT_CREATOR);
      if (carta->author)
	PDF_set_info(p, "Author", carta->author);
      if (carta->title)
	PDF_set_info(p, "Title", carta->title);
      if (carta->subject)
	PDF_set_info(p, "Subject", carta->subject);
      if (carta->keywords)
	PDF_set_info(p, "Keywords", carta->keywords);
      if (carta->copyright)
	PDF_set_info(p, "Copyright", carta->copyright);
      if (carta->version)
	PDF_set_info(p, "Version", carta->version);
      if (carta->language)
	PDF_set_info(p, "Language", carta->language);
      if (carta->note)
	PDF_set_info(p, "Note", carta->note);

      /* load variables, fonts and images */
      cg_load_variables(carta);
      cg_load_fonts(carta);
      cg_load_images(carta);

      /* process every decks */
      for (i = 0; i < yv_len(carta->decks); ++i)
	{
	  cg_deck_t *deck = carta->decks[i];

	  /* update variables */
	  cg_expr_set_scalar(carta, "DECK_INDEX", (double)i + 1);
	  cg_expr_set_element(carta, "PAGE", deck->paper_width,
			      deck->paper_height);
	  cg_expr_set_element(carta, "PAGE_DIM", deck->paper_width,
			      deck->paper_height);
	  cg_expr_set_element(carta, "PAGE_DIMENSION", deck->paper_width,
			      deck->paper_height);
	  cg_expr_set_element(carta, "CARD", deck->card_width,
			      deck->card_height);
	  cg_expr_set_element(carta, "CARD_DIM", deck->card_width,
			      deck->card_height);
	  cg_expr_set_element(carta, "CARD_DIMENSION", deck->card_width,
			      deck->card_height);
	  cg_expr_set_element(carta, "MARGIN", deck->paper_margin_w,
			      deck->paper_margin_h);
	  cg_expr_set_element(carta, "MARGIN_DIM", deck->paper_margin_w,
			      deck->paper_margin_h);
	  cg_expr_set_element(carta, "MARGIN_DIMENSION",
			      deck->paper_margin_w, deck->paper_margin_h);
	  cg_expr_set_element(carta, "SPACE", deck->space_width,
			      deck->space_height);
	  cg_expr_set_element(carta, "SPACE_DIM", deck->space_width,
			      deck->space_height);
	  cg_expr_set_element(carta, "SPACE_DIMENSION", deck->space_width,
			      deck->space_height);
	  cg_expr_set_scalar(carta, "CARD_DECK",
			     (double)cg_deck_get_nb_cards(carta, deck));
	  cg_expr_set_scalar(carta, "CARDS_IN_DECK",
			     (double)cg_deck_get_nb_cards(carta,deck));
	  cg_expr_set_scalar(carta, "NBR_CARDS_IN_DECK",
			     (double)cg_deck_get_nb_cards(carta,deck));
	  /* process deck */
	  cg_process_deck(carta, deck);
	}
      cg_unload_images(carta);
      PDF_end_document(p, "");
    }
  PDF_CATCH(p)
    {
      YLOG_ADD(YLOG_ERR, "PDFlib exception:\n[%d] %s: %s\n",
	       PDF_get_errnum(p), PDF_get_apiname(p), PDF_get_errmsg(p));
      return_value = YEUNDEF;
    }
  PDF_delete(p);
  return (return_value);
}

/*
** cg_deck_get_nb_cards()
** Returns the number of cards in a deck.
*/
int cg_deck_get_nb_cards(cg_t *carta, cg_deck_t *deck)
{
  int count = 0;
  int j;

  for (j = 0; j < yv_len(deck->cards); ++j)
    count += cg_card_count(carta, deck->cards[j]);
  return (count);
}

/*
** cg_deck_get_card()
** Returns a card from its index in the deck.
*/
ydom_node_t* cg_deck_get_card(cg_t *carta, cg_deck_t *deck, int num)
{
  int count = 0;
  int j;

  for (j = 0; j < yv_len(deck->cards); ++j)
    {
      count += cg_card_count(carta, deck->cards[j]);
      if (num < count)
	return (deck->cards[j]);
    }
  return (NULL);
}

/*
** cg_process_deck()
** Process a deck of cards and write PDF data.
*/
yerr_t cg_process_deck(cg_t *carta, cg_deck_t *deck)
{
  int nbr_cards, cards_per_page, nbr_pages, i;

  /* check paper size */
  if (!deck->cols || !deck->rows)
    {
      YLOG_ADD(YLOG_WARN, "Cards bigger than paper size [line %d]. Deck skipped.",
	       deck->line_nbr);
      return (YE2BIG);
    }
  
  /* compute the number of pages and cards per page */
  cards_per_page = deck->cols * deck->rows;
  nbr_cards = cg_deck_get_nb_cards(carta,deck);
  nbr_pages =  nbr_cards / cards_per_page;
  nbr_pages = (nbr_cards % cards_per_page) ? (nbr_pages + 1) :
    nbr_pages;
  /* process every pages */
  for (i = 0; i < nbr_pages; ++i)
    {
      if (!carta->reverse)
	{
	  cg_process_deck_fronts(carta, deck, i, cards_per_page);
	  cg_process_deck_backs(carta, deck, i, cards_per_page);
	}
      else
	{
	  cg_process_deck_backs(carta, deck, i, cards_per_page);
	  cg_process_deck_fronts(carta, deck, i, cards_per_page);
	}
    }
  return (YENOERR);
}

/*
** cg_process_deck_fronts()
** Process cards' front, for one page.
*/
yerr_t cg_process_deck_fronts(cg_t *carta, cg_deck_t *deck, int page,
			      int cards_per_page)
{
  yvalue_t val_x, val_y;
  int j, o_x, o_y;
  ydom_node_t *card;

  cg_new_page(carta, deck);
  cg_cut_lines(carta, deck, BACK_NO);
  for (j = 0; j < cards_per_page &&
	 ((page * cards_per_page) + j) < cg_deck_get_nb_cards(carta, deck); ++j)
    {
      o_x = j % deck->cols;
      o_y = (deck->rows - 1) - (j / deck->cols);
      val_x.value = (double)(o_x * YVAL2PT(deck->card_width)) +
	(double)(o_x * YVAL2PT(deck->space_width)) +
	YVAL2PT(deck->paper_margin_w);
      val_y.value = (double)(o_y * YVAL2PT(deck->card_height)) +
	(double)(o_y * YVAL2PT(deck->space_height)) +
	YVAL2PT(deck->paper_margin_h);
      val_x.unit = val_y.unit = YUNIT_PT;
      card = cg_deck_get_card(carta, deck, (page * cards_per_page) + j);
      /* update variable */
      cg_expr_set_scalar(carta, "CARD_INDEX",
			 (double)((page * cards_per_page) + j + 1));
      /* process card */
      cg_process_card(carta, deck, card, val_x, val_y);
    }
  PDF_end_page_ext(carta->p, "");
  return (YENOERR);
}

/*
** cg_process_deck_backs()
** Process cards' back, for one page.
*/
yerr_t cg_process_deck_backs(cg_t *carta, cg_deck_t *deck, int page,
			     int cards_per_page)
{
  yvalue_t val_x, val_y;
  int j, o_x, o_y;
  ydom_node_t *card;

  if (carta->do_back != BACK_NO)
    {
      cg_new_page(carta, deck);
      if (carta->do_back == BACK_WIDTH)
	cg_swip_page(carta, deck);
      cg_cut_lines(carta, deck, carta->do_back);
      for (j = 0; j < cards_per_page &&
	     ((page * cards_per_page) + j) < cg_deck_get_nb_cards(carta, deck); ++j)
	{
	  if (carta->do_back != BACK_HEIGHT && carta->do_back != BACK_WIDTH)
	    continue ;
	  /* compute card's coordinates */
	  o_x = j % deck->cols;
	  o_y = (deck->rows - 1) - (j / deck->cols);
	  val_x.value = YVAL2PT(deck->paper_width) -
	    YVAL2PT(deck->paper_margin_w) -
	    ((double)(o_x + 1) * YVAL2PT(deck->card_width)) -
	    ((double)o_x * YVAL2PT(deck->space_width));
	  val_y.value = ((double)o_y * YVAL2PT(deck->card_height)) +
	    ((double)o_y * YVAL2PT(deck->space_height)) +
	    YVAL2PT(deck->paper_margin_h);
	  val_x.unit = val_y.unit = YUNIT_PT;
	  card = cg_deck_get_card(carta, deck, (page * cards_per_page) + j);
	  /* update variable */
	  cg_expr_set_scalar(carta, "CARD_INDEX",
			     (double)((page * cards_per_page) + j));
	  /* process card's back */
	  cg_process_back(carta, deck, card, val_x, val_y);
	}
      if (carta->do_back == BACK_WIDTH)
	cg_swip_page(carta, deck);
      PDF_end_page_ext(carta->p, "");
    }
  return (YENOERR);
}

/*
** cg_new_page()
** Create a new page and set all orientation information.
*/
yerr_t cg_new_page(cg_t *carta, cg_deck_t *deck)
{
  ystr_t ys;
  double w, h;

  w = YVAL2PT(deck->paper_width);
  h = YVAL2PT(deck->paper_height);
  ys = ys_new("");
  ys_printf(&ys, "artbox {0.0 0.0 %f %f} bleedbox {0.0 0.0 %f %f} "
	    "cropbox {0.0 0.0 %f %f} trimbox {0.0 0.0 %f %f}", w, h,
	    w, h, w, h, w, h);
  PDF_begin_page_ext(carta->p, w, h, ys);
  ys_del(&ys);
  return (YENOERR);
}

/*
** cg_cut_lines()
** Write the cut lines in a page.
*/
yerr_t cg_cut_lines(cg_t *carta, cg_deck_t *deck, cg_back_t back)
{
  int i;
  double color_luminance = 0;

  /* hidden-ditch */
  PDF_save(carta->p);
  if ((back == BACK_NO && deck->ditch_odd) ||
      (back != BACK_NO && deck->ditch_even))
    {
      unsigned int hex1, hex2, hex3;
      double col1, col2, col3;
      char *used_color = (back == BACK_NO) ? deck->ditch_odd : deck->ditch_even;

      sscanf(used_color, "#%02x%02x%02x", &hex1, &hex2, &hex3);
      col1 = (double)hex1 / 255;
      col2 = (double)hex2 / 255;
      col3 = (double)hex3 / 255;
      color_luminance = (col1 * LUMINANCE_RED) + (col2 * LUMINANCE_GREEN) +
	(col3 * LUMINANCE_BLUE);
      PDF_setcolor(carta->p, "fill", "rgb", col1, col2, col3, 0.0);
      PDF_rect(carta->p, YVAL2PT(deck->paper_margin_w) / 2.0,
	       YVAL2PT(deck->paper_margin_h) / 2.0,
	       YVAL2PT(deck->paper_width) - YVAL2PT(deck->paper_margin_w),
	       YVAL2PT(deck->paper_height) - YVAL2PT(deck->paper_margin_h));
      PDF_fill(carta->p);
    }
  PDF_restore(carta->p);

  /* external cut lines */
  PDF_save(carta->p);
  PDF_setcolor(carta->p, "fillstroke", "rgb", 0.0, 0.0, 0.0, 0.0);
  for (i = 0; i <= deck->cols; ++i)
    {
      double x, y;

      if (i == deck->cols && YVAL2PT(deck->space_width))
	break ;
      x = YVAL2PT(deck->paper_margin_w) + (i * YVAL2PT(deck->card_width)) +
	(i * YVAL2PT(deck->space_width));
      if (back == BACK_HEIGHT || back == BACK_WIDTH)
	x = YVAL2PT(deck->paper_width) - YVAL2PT(deck->paper_margin_w) -
	  (i * YVAL2PT(deck->card_width)) - (i * YVAL2PT(deck->space_width));
      y = 0.0;
      PDF_moveto(carta->p, x, y);
      y = YVAL2PT(deck->paper_margin_h) / 2.0;
      PDF_lineto(carta->p, x, y);
      y = YVAL2PT(deck->paper_height);
      PDF_moveto(carta->p, x, y);
      y = YVAL2PT(deck->paper_height) -
	(YVAL2PT(deck->paper_margin_h) / 2.0);
      PDF_lineto(carta->p, x, y);
      /* second cut line - space between cards */
      if (i < deck->cols && YVAL2PT(deck->space_width))
	{
	  if (back == BACK_HEIGHT || back == BACK_WIDTH)
	    x -= YVAL2PT(deck->card_width);
	  else
	    x += YVAL2PT(deck->card_width);
	  y = 0.0;
	  PDF_moveto(carta->p, x, y);
	  y = YVAL2PT(deck->paper_margin_h) / 2.0;
	  PDF_lineto(carta->p, x, y);
	  y = YVAL2PT(deck->paper_height);
	  PDF_moveto(carta->p, x, y);
	  y = YVAL2PT(deck->paper_height) -
	    (YVAL2PT(deck->paper_margin_h) / 2.0);
	  PDF_lineto(carta->p, x, y);
	}
    }
  for (i = 0; i <= deck->rows; ++i)
    {
      double x, y;

      if (i == deck->rows && YVAL2PT(deck->space_height))
	break ;
      x = 0.0;
      y = YVAL2PT(deck->paper_margin_h) + (i * YVAL2PT(deck->card_height)) +
	(i * YVAL2PT(deck->space_height));
      PDF_moveto(carta->p, x, y);
      x = YVAL2PT(deck->paper_margin_w) / 2.0;
      PDF_lineto(carta->p, x, y);
      x = YVAL2PT(deck->paper_width);
      PDF_moveto(carta->p, x, y);
      x = YVAL2PT(deck->paper_width) -
	(YVAL2PT(deck->paper_margin_w) / 2.0);
      PDF_lineto(carta->p, x, y);
      /* second cut line - space between cards */
      if (i < deck->rows && YVAL2PT(deck->space_height))
	{
	  x = 0.0;
	  y += YVAL2PT(deck->card_height);
	  PDF_moveto(carta->p, x, y);
	  x = YVAL2PT(deck->paper_margin_w) / 2.0;
	  PDF_lineto(carta->p, x, y);
	  x = YVAL2PT(deck->paper_width);
	  PDF_moveto(carta->p, x, y);
	  x = YVAL2PT(deck->paper_width) -
	    (YVAL2PT(deck->paper_margin_w) / 2.0);
	  PDF_lineto(carta->p, x, y);
	}
    }
  PDF_stroke(carta->p);
  PDF_restore(carta->p);

  /* internal cut lines */
  PDF_save(carta->p);
  if (color_luminance >= LUMINANCE_THRESHOLD)
    PDF_setcolor(carta->p, "fillstroke", "rgb", 0.0, 0.0, 0.0, 0.0);
  else
    PDF_setcolor(carta->p, "fillstroke", "rgb", 1.0, 1.0, 1.0, 0.0);
  for (i = 0; i <= deck->cols; ++i)
    {
      double x, y;

      if (i == deck->cols && YVAL2PT(deck->space_width))
	break ;
      x = YVAL2PT(deck->paper_margin_w) + (i * YVAL2PT(deck->card_width)) +
	(i * YVAL2PT(deck->space_width));
      if (back == BACK_HEIGHT || back == BACK_WIDTH)
	x = YVAL2PT(deck->paper_width) - YVAL2PT(deck->paper_margin_w) -
	  (i * YVAL2PT(deck->card_width)) - (i * YVAL2PT(deck->space_width));
      y = YVAL2PT(deck->paper_margin_h) / 2.0;
      PDF_moveto(carta->p, x, y);
      y = YVAL2PT(deck->paper_margin_h) * 2.0 / 3.0;
      PDF_lineto(carta->p, x, y);
      y = YVAL2PT(deck->paper_height) -
	(YVAL2PT(deck->paper_margin_h) / 2.0);
      PDF_moveto(carta->p, x, y);
      y = YVAL2PT(deck->paper_height) -
	(YVAL2PT(deck->paper_margin_h) * 2.0 / 3.0);
      PDF_lineto(carta->p, x, y);
      /* second cut line - space between cards */
      if (i < deck->cols && YVAL2PT(deck->space_width))
	{
	  if (back == BACK_HEIGHT || back == BACK_WIDTH)
	    x -= YVAL2PT(deck->card_width);
	  else
	    x += YVAL2PT(deck->card_width);
	  y = YVAL2PT(deck->paper_margin_h) / 2.0;
	  PDF_moveto(carta->p, x, y);
	  y = YVAL2PT(deck->paper_margin_h) * 2.0 / 3.0;
	  PDF_lineto(carta->p, x, y);
	  y = YVAL2PT(deck->paper_height) -
	    (YVAL2PT(deck->paper_margin_h) / 2.0);
	  PDF_moveto(carta->p, x, y);
	  y = YVAL2PT(deck->paper_height) -
	    (YVAL2PT(deck->paper_margin_h) * 2.0 / 3.0);
	  PDF_lineto(carta->p, x, y);
	}
    }
  for (i = 0; i <= deck->rows; ++i)
    {
      double x, y;

      if (i == deck->rows && YVAL2PT(deck->space_height))
	break ;
      x = YVAL2PT(deck->paper_margin_w) / 2.0;
      y = YVAL2PT(deck->paper_margin_h) + (i * YVAL2PT(deck->card_height)) +
	(i * YVAL2PT(deck->space_height));
      PDF_moveto(carta->p, x, y);
      x = YVAL2PT(deck->paper_margin_w) * 2.0 / 3.0;
      PDF_lineto(carta->p, x, y);
      x = YVAL2PT(deck->paper_width) - (YVAL2PT(deck->paper_margin_w) / 2.0);
      PDF_moveto(carta->p, x, y);
      x = YVAL2PT(deck->paper_width) -
	(YVAL2PT(deck->paper_margin_w) * 2.0 / 3.0);
      PDF_lineto(carta->p, x, y);
      /* second cut line - space between cards */
      if (i < deck->rows && YVAL2PT(deck->space_height))
	{
	  x = YVAL2PT(deck->paper_margin_w) / 2.0;
	  y += YVAL2PT(deck->card_height);
	  PDF_moveto(carta->p, x, y);
	  x = YVAL2PT(deck->paper_margin_w) * 2.0 / 3.0;
	  PDF_lineto(carta->p, x, y);
	  x = YVAL2PT(deck->paper_width) -
	    (YVAL2PT(deck->paper_margin_w) / 2.0);
	  PDF_moveto(carta->p, x, y);
	  x = YVAL2PT(deck->paper_width) -
	    (YVAL2PT(deck->paper_margin_w) * 2.0 / 3.0);
	  PDF_lineto(carta->p, x, y);
	}
    }
  PDF_stroke(carta->p);
  PDF_restore(carta->p);
  return (YENOERR);
}

/*
** cg_swip_page()
** Do a rotation of the page.
*/
yerr_t cg_swip_page(cg_t *carta, cg_deck_t *deck)
{
  PDF_rotate(carta->p, 180.0);
  PDF_translate(carta->p, -(YVAL2PT(deck->paper_width)),
		-(YVAL2PT(deck->paper_height)));
  return (YENOERR);
}

/*
** cg_load_variables()
** Load all specified variables.
*/
yerr_t cg_load_variables(cg_t *carta)
{
  int i;

  for (i = 0; i < yv_len(carta->variables); ++i)
    cg_put_var(carta, carta->variables[i]);
  return (YENOERR);
}

/*
** cg_load_fonts()
** Load all specified fonts.
*/
yerr_t cg_load_fonts(cg_t *carta)
{
  int i;
  cg_font_t *font;
  ystr_t s, param;
  char *pt;

  param = ys_new("");
  for (i = 0; i < yv_len(carta->fonts); ++i)
    {
      font = carta->fonts[i];
      s = ys_new(font->metrics);
      if ((pt = strrchr(s, '/')))
	{
	  *pt = '\0';
	  PDF_set_parameter(carta->p, "SearchPath", s);
	  ys_printf(&param, "%s=%s", font->id, pt + 1);
	  PDF_set_parameter(carta->p, "FontAFM", param);
	}
      else
	{
	  ys_printf(&param, "%s=%s", font->id, font->metrics);
	  PDF_set_parameter(carta->p, "FontAFM", param);
	}
      ys_printf(&s, font->outline);
      if ((pt = strrchr(s, '/')))
	{
	  *pt = '\0';
	  PDF_set_parameter(carta->p, "SearchPath", s);
	  ys_printf(&param, "%s=%s", font->id, pt + 1);
	  PDF_set_parameter(carta->p, "FontOutline", param);
	}
      else
	{
	  ys_printf(&param, "%s=%s", font->id, font->outline);
	  PDF_set_parameter(carta->p, "FontOutline", param);
	}
      ys_del(&s);
      /* font->f = PDF_load_font(carta->p, font->id, 0, "host", "embedding=true"); */
      font->f = PDF_load_font(carta->p, font->id, 0, "iso8859-15", "embedding=true");
    }
  ys_del(&param);
  return (YENOERR);
}

/*
** cg_load_images()
** Load all specified images.
*/
yerr_t cg_load_images(cg_t *carta)
{
  int i;
  cg_image_t *image;
  yerr_t load_err;

  for (i = 0; i < yv_len(carta->masks); ++i)
    {
      image = carta->masks[i];
      if ((load_err = cg_load_image(carta, image, YTRUE)) != YENOERR)
	return (load_err);
    }
  for (i = 0; i < yv_len(carta->images); ++i)
    {
      image = carta->images[i];
      if ((load_err = cg_load_image(carta, image, YFALSE)) != YENOERR)
	return (load_err);
    }
  return (YENOERR);
}

/*
** cg_load_image()
** Load one specified image.
*/
yerr_t cg_load_image(cg_t *carta, cg_image_t *image, ybool_t is_mask)
{
  char *type;
  ystr_t optlist = NULL;
  yerr_t res = YENOERR;

  if (!image)
    return (YENOERR);
  /* check image type */
  if (!strcasecmp(image->file + strlen(image->file) - strlen(".jpg"), ".jpg") ||
      !strcasecmp(image->file + strlen(image->file) - strlen(".jpeg"), ".jpeg"))
    type = "jpeg";
  else if (!strcasecmp(image->file + strlen(image->file) - strlen(".gif"), ".gif"))
    type = "gif";
  else if (!strcasecmp(image->file + strlen(image->file) - strlen(".png"), ".png"))
    type = "png";
  else
    {
      YLOG_ADD(YLOG_WARN, "Unknown image type (%s).", image->file);
      return (YEUNDEF);
    }
  if (is_mask)
    {
      if (strcmp(type, "png"))
	{
	  YLOG_ADD(YLOG_WARN, "Image masks must be PNG (%s).", image->file);
	  return (YEINVAL);
	}
      if ((image->i = PDF_load_image(carta->p, type, image->file, 0, "mask")) == -1)
	{
	  YLOG_ADD(YLOG_WARN, "Unable to open image mask '%s'.", image->file);
	  return (YEUNDEF);
	}
    }
  else
    {
      optlist = ys_new("");
      if (image->mask &&
	  (image->m = PDF_load_image(carta->p, "png",
				     image->mask, 0, "mask")) != -1)
	ys_printf(&optlist, "masked %d", image->m);
      else if (image->mask_id)
	{
	  int i;
	  cg_image_t *img_mask = NULL;
	  for (i = 0; i < yv_len(carta->masks); ++i)
	    {
	      img_mask = carta->masks[i];
	      if (!strcasecmp(image->mask_id, img_mask->id) &&
		  img_mask->i != -1)
		break ;
	      img_mask = NULL;
	    }
	  if (!img_mask)
	    YLOG_ADD(YLOG_WARN, "Unable to find image mask '%s'.", image->mask_id);
	  else
	    ys_printf(&optlist, "masked %d", img_mask->i);
	}
      else
	image->m = -1;
      if ((image->i = PDF_load_image(carta->p, type, image->file, 0, optlist)) == -1)
	{
	  YLOG_ADD(YLOG_WARN, "Unable to open image '%s'.", image->file);
	  res = YEUNDEF;
	}
      ys_del(&optlist);
    }
  return (res);
}

/*
** cg_unload_images()
** Unload the previously loaded images.
*/
yerr_t cg_unload_images(cg_t *carta)
{
  int i;
  cg_image_t *image;

  for (i = 0; i < yv_len(carta->images); ++i)
    {
      image = carta->images[i];
      cg_unload_image(carta, image);
    }
  return (YENOERR);
}

/*
** cg_unload_image()
** Unload one previously loaded image.
*/
yerr_t cg_unload_image(cg_t *carta, cg_image_t *image)
{
  if (!image)
    return (YENOERR);
  if (image->i != -1)
    PDF_close_image(carta->p, image->i);
  if (image->mask && image->m != -1)
    PDF_close_image(carta->p, image->m);
  image->i = image->m = -1;
  return (YENOERR);
}
