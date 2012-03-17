#include "cartagenius.h"

/*
** cg_new_fontdef()
** Create a new fontdef object.
*/
cg_fontdef_t *cg_new_fontdef()
{
  cg_fontdef_t *def;

  if (!(def = malloc0(sizeof(cg_fontdef_t))))
    {
      YLOG_ADD(YLOG_WARN, "Memory allocation error.");
      return (NULL);
    }
  def->fill_color.type = def->stroke_color.type = CG_EXPR_COLOR;
  def->line_space = 1.0;
  def->h_scale = 100.0;
  return (def);
}

/*
** cg_del_fontdef()
** Delete a fontdef object.
*/
void cg_del_fontdef(cg_fontdef_t *def)
{
  if (!def)
    return ;
  free0(def->id);
  free0(def->fontname);
  free_var(&def->fill_color, &def->fill_color);
  free_var(&def->stroke_color, &def->stroke_color);
  free0(def);
}

/*
** cg_put_text_define()
** Put a text definition in user space.
*/
yerr_t cg_put_text_define(cg_t *carta, ydom_node_t *node)
{
  char *id = NULL;
  yvect_t array;
  cg_text_define_t *td;

  if (!(id = ydom_node_get_attr_value(node, "id")) || !strlen(id))
    {
      free0(id);
      YLOG_ADD(YLOG_WARN, "Empty text definition's id [line %d].",
	       node->line_nbr);
      return (YEINVAL);
    }
  if (!(td = malloc0(sizeof(cg_text_define_t))) ||
      !(td->blocks = yv_new()))
    {
      free0(id);
      free0(td);
      YLOG_ADD(YLOG_ERR, "Memory allocation error.");
      return (YENOMEM);
    }
  td->id = id;

  /* process all blocks */
  if ((array = ydom_node_xpath(node, "block")) &&
      yv_len(array) && array[0])
    {
      int i;
      for (i = 0; i < yv_len(array); ++i)
	{
	  cg_text_block_t *block;
	  char *fontname = ydom_node_get_attr_value(array[i], "font");
	  char *fontsize = ydom_node_get_attr_value(array[i], "size");
	  char *color = ydom_node_get_attr_value(array[i], "color");
	  char *space = ydom_node_get_attr_value(array[i], "space");
	  char *underline = ydom_node_get_attr_value(array[i], "underline");
	  char *overline = ydom_node_get_attr_value(array[i], "overline");
	  char *strikeout = ydom_node_get_attr_value(array[i], "strikeout");
	  char *char_space = ydom_node_get_attr_value(array[i], "char-space");
	  char *h_scale = ydom_node_get_attr_value(array[i], "h-scale");
	  cg_expr_var_t e_fs = {0}, e_c = {0}, e_s = {0}, e_cs = {0};
	  cg_expr_var_t e_hs = {0};
	  ydom_node_t *subnode;

	  /* get block attributes */
	  if (!(block = malloc0(sizeof(cg_text_block_t))) ||
	      !(block->font = cg_new_fontdef()) ||
	      !(block->parts = yv_new()))
	    {
	      if (block)
		free0(block->font);
	      free0(block);
	      YLOG_ADD(YLOG_ERR, "Memory allocation error.");
	    }
	  else if (!fontsize ||
		   cg_expr(carta, fontsize, &e_fs, carta->default_unit) != YENOERR ||
		   e_fs.type != CG_EXPR_VALUE)
	    YLOG_ADD(YLOG_WARN, "Expression error ('size' parameter) [line %d].",
		     node->line_nbr);
	  else
	    {
	      /* add the block */
	      yv_add(&td->blocks, block);
	      block->font->fontname = strdup(fontname);
	      block->font->fontsize = e_fs.value.value;
	      if (color &&
		  cg_expr(carta, color, &e_c, carta->default_unit) == YENOERR &&
		  e_c.type == CG_EXPR_COLOR)
		block->font->fill_color = block->font->stroke_color = e_c;
	      if (space &&
		  cg_expr(carta, space, &e_s, carta->default_unit) == YENOERR &&
		  e_c.type == CG_EXPR_SCALAR)
		block->font->line_space = e_s.value.scalar;
	      if (underline && (!strcasecmp(underline, "yes") || !strcasecmp(underline, "true")))
		block->font->underline = YTRUE;
	      if (overline && (!strcasecmp(overline, "yes") || !strcasecmp(overline, "true")))
		block->font->overline = YTRUE;
	      if (strikeout && (!strcasecmp(strikeout, "yes") || !strcasecmp(strikeout, "true")))
		block->font->strikeout = YTRUE;
	      if (char_space &&
		  cg_expr(carta, char_space, &e_cs, carta->default_unit) == YENOERR &&
		  e_cs.type == CG_EXPR_VALUE)
		block->font->char_space = e_cs.value.value;
	      if (h_scale)
		{
		  ystr_t hs_ys = ys_new("");
		  ys_printf(&hs_ys, "(%s)", h_scale);
		  if (cg_expr(carta, hs_ys, &e_hs, carta->default_unit) != YENOERR ||
		      e_hs.type != CG_EXPR_SCALAR)
		    YLOG_ADD(YLOG_WARN, "Expression error ('char-space' parameter) [line %d].",
			     node->line_nbr);
		  else
		    block->font->h_scale = e_hs.value.scalar;
		  ys_del(&hs_ys);
		}
	      /* process all parts of the block */
	      for (subnode = ydom_node_get_first_child(array[i]); subnode;
		   subnode = ydom_node_get_next(subnode))
		{
		  cg_text_part_t *part;

		  if (!(part = malloc0(sizeof(cg_text_part_t))))
		    {
		      YLOG_ADD(YLOG_ERR, "Memory allocation error.");
		      continue ;
		    }
		  if (ydom_node_is_text(subnode))
		    {
		      char *string = ydom_node_get_value(subnode);
		      part->string = ys_new(string);
		      part->type = TEXT_PART_STRING;
		      free0(string);
		    }
		  else if (ydom_node_is_element(subnode) && subnode->name &&
			   !strcasecmp(subnode->name, "br"))
		    part->type = TEXT_PART_BR;
		  else if (ydom_node_is_element(subnode) && subnode->name &&
			   !strcasecmp(subnode->name, "p"))
		    part->type = TEXT_PART_PARA;
		  else if (ydom_node_is_element(subnode) && subnode->name &&
			   !strcasecmp(subnode->name, "value"))
		    {
		      ydom_node_t *subnode2;
		      char *precision = ydom_node_get_attr_value(subnode, "precision");
		      /* get expression value */
		      for (subnode2 = ydom_node_get_first_child(subnode);
			   subnode2; subnode2 = ydom_node_get_next(subnode2))
			{
			  cg_expr_var_t var = {0}, var2 = {0};
			  yerr_t res_val;
			  char *str;
			  ystr_t exp_s;
			  int old_precision = carta->expr->precision;
			  
			  if (!ydom_node_is_text(subnode2) ||
			      !(str = ydom_node_get_value(subnode2)))
			    continue ;
			  if (precision)
			    cg_expr_set_precision(carta, precision);
			  exp_s = ys_new("");
			  ys_printf(&exp_s, "(%s)", str);
			  res_val = cg_expr(carta, exp_s, &var, carta->default_unit);
			  ys_del(&exp_s);
			  free0(str);
			  if (res_val == YENOERR)
			    res_val = cg_expr_f_string(carta, &var2, var);
			  free_var(&var, &var);
			  if (res_val == YENOERR && var2.value.string)
			    {
			      free0(part->string);
			      part->string = ys_new(var2.value.string);
			      part->type = TEXT_PART_STRING;
			    }
			  free_var(&var2, &var2);
			  carta->expr->precision = old_precision;
			}
		      free0(precision);
		    }
		  else
		    {
		      free0(part);
		      continue ;
		    }
		  yv_add(&block->parts, part);
		}
	    }
	  free0(fontname);
	  free0(fontsize);
	  free0(color);
	  free0(space);
	  free0(underline);
	  free0(overline);
	  free0(strikeout);
	  free0(char_space);
	  free0(h_scale);
	}
    }
  yv_del(&array, NULL, NULL);
  yv_add(&carta->text_defines, td);
  return (YENOERR);
}

/*
** cg_put_text_area()
** Put a text area in card.
*/
yerr_t cg_put_text_area(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
			ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *id = NULL, *x = NULL, *y = NULL, *width = NULL, *height = NULL;
  char *radius = NULL, *rotation = NULL, *opacity = NULL, *blendmode = NULL;
  char *fill_color = NULL, *line_color = NULL, *line_width = NULL;
  char *border = NULL, *dash1 = NULL, *dash2 = NULL;
  yvalue_t v_x, v_y, v_w, v_h, v_r, v_l, v_d1, v_d2;
  yerr_t res = YENOERR;
  int gstate;

  id = ydom_node_get_attr_value(node, "id");
  x = ydom_node_get_attr_value(node, "x");
  y = ydom_node_get_attr_value(node, "y");
  width = ydom_node_get_attr_value(node, "width");
  height = ydom_node_get_attr_value(node, "height");
  radius = ydom_node_get_attr_value(node, "radius");
  rotation = ydom_node_get_attr_value(node, "rotation");
  opacity = ydom_node_get_attr_value(node, "opacity");
  blendmode = ydom_node_get_attr_value(node, "blendmode");
  fill_color = ydom_node_get_attr_value(node, "fill-color");
  line_color = ydom_node_get_attr_value(node, "line-color");
  line_width = ydom_node_get_attr_value(node, "line-width");
  border = ydom_node_get_attr_value(node, "border");
  dash1 = ydom_node_get_attr_value(node, "dash1");
  dash2 = ydom_node_get_attr_value(node, "dash2");

  if (!id || !x || !y || !width || !height)
    {
      YLOG_ADD(YLOG_WARN, "Empty text area declaration [line %d].",
	       node->line_nbr);
      res = YEUNDEF;
    }
  else
    {
      /* search defined text with the same id */
      int i;
      cg_text_define_t *td = NULL;

      for (i = 0; i < yv_len(carta->text_defines); ++i)
	{
	  td = carta->text_defines[i];
	  if (td->id && !strcmp(td->id, id))
	    break ;
	  td = NULL;
	}
      if (!td)
	YLOG_ADD(YLOG_WARN, "Unable to find defined text '%s' [line %d].",
		 id, node->line_nbr);
      else
	{
	  cg_expr_var_t e_x = {0}, e_y = {0}, e_w = {0}, e_h = {0};
	  cg_expr_var_t e_d1 = {0}, e_d2 = {0};

	  if (cg_expr(carta, x, &e_x, carta->default_unit) != YENOERR ||
	      e_x.type != CG_EXPR_VALUE)
	    YLOG_ADD(YLOG_WARN, "Expression error ('x' parameter) [line %d].",
		     node->line_nbr);
	  else if (cg_expr(carta, y, &e_y, carta->default_unit) != YENOERR ||
		   e_y.type != CG_EXPR_VALUE)
	    YLOG_ADD(YLOG_WARN, "Expression error ('y' parameter) [line %d].",
		     node->line_nbr);
	  else if (cg_expr(carta, width, &e_w, carta->default_unit) != YENOERR ||
		   e_w.type != CG_EXPR_VALUE)
	    YLOG_ADD(YLOG_WARN, "Expression error ('width' parameter) [line %d].",
		     node->line_nbr);
	  else if (cg_expr(carta, height, &e_h, carta->default_unit) != YENOERR ||
		   e_h.type != CG_EXPR_VALUE)
	    YLOG_ADD(YLOG_WARN, "Expression error ('height' parameter) [line %d].",
		     node->line_nbr);
	  else
	    {
	      cg_expr_var_t e_r = {0};
	      if (radius &&
		  (cg_expr(carta, radius, &e_r, carta->default_unit) != YENOERR ||
		   e_r.type != CG_EXPR_VALUE))
		{
		  YLOG_ADD(YLOG_WARN, "Expression error ('radius' parameter) [line %d].",
			   node->line_nbr);
		  free0(radius);
		}
	      v_x = e_x.value.value;
	      v_y = e_y.value.value;
	      v_w = e_w.value.value;
	      v_h = e_h.value.value;
	      v_r = e_r.value.value;
	      v_x.value = YVAL2PT(v_x) + YVAL2PT(card_x);
	      v_y.value = YVAL2PT(v_y) + YVAL2PT(card_y);
	      v_w.value = YVAL2PT(v_w);
	      v_h.value = YVAL2PT(v_h);
	      v_x.unit = v_y.unit = v_w.unit = v_h.unit = YUNIT_PT;
	      free_var(&e_r, &e_r);
	      
	      /* put frame as in text box */
	      /* put frame */
	      if (fill_color || line_color)
		{
		  yvalue_t v_x2 = v_x;
		  yvalue_t v_y2 = v_y;

		  PDF_save(carta->p);
		  if (opacity)
		    {
		      ystr_t ys = ys_new("");
		      ys_printf(&ys, "opacityfill %s opacitystroke %s",
				opacity, opacity);
		      gstate = PDF_create_gstate(carta->p, ys);
		      ys_del(&ys);
		      PDF_set_gstate(carta->p, gstate);
		    }
		  if (blendmode)
		    {
		      ystr_t ys = ys_new("");
		      ys_printf(&ys, "blendmode %s", blendmode);
		      gstate = PDF_create_gstate(carta->p, ys);
		      ys_del(&ys);
		      PDF_set_gstate(carta->p, gstate);
		    }
		  if (fill_color)
		    {
		      cg_expr_var_t e_fc;
		      if (cg_expr(carta, fill_color, &e_fc,
				  carta->default_unit) != YENOERR ||
			  e_fc.type != CG_EXPR_COLOR)
			YLOG_ADD(YLOG_WARN, "Expression error ('fill-color' parameter) [line %d].",
				 node->line_nbr);
		      else
			PDF_setcolor(carta->p, "fill", "rgb", e_fc.value.color.red,
				     e_fc.value.color.green, e_fc.value.color.blue, 0.0);
		      free_var(&e_fc, &e_fc);
		    }
		  if (line_color)
		    {
		      cg_expr_var_t e_lc;
		      if (cg_expr(carta, line_color, &e_lc,
				  carta->default_unit) != YENOERR ||
			  e_lc.type != CG_EXPR_COLOR)
			YLOG_ADD(YLOG_WARN, "Expression error ('line-color' parameter) [line %d].",
				 node->line_nbr);
		      else
			PDF_setcolor(carta->p, "stroke", "rgb", e_lc.value.color.red,
				     e_lc.value.color.green, e_lc.value.color.blue, 0.0);
		      free_var(&e_lc, &e_lc);
		    }
		  if (line_width)
		    {
		      cg_expr_var_t e_l;
		      if (cg_expr(carta, line_width, &e_l, carta->default_unit) != YENOERR ||
			  e_l.type != CG_EXPR_VALUE)
			YLOG_ADD(YLOG_WARN, "Expression error ('line-width' parameter) [line %d].",
				 node->line_nbr);
		      else
			{
			  v_l = e_l.value.value;
			  PDF_setlinewidth(carta->p, YVAL2PT(v_l));
			}
		      free_var(&e_l, &e_l);
		    }
		  if (dash1 && dash2)
		    {
		      if (cg_expr(carta, dash1, &e_d1, carta->default_unit) != YENOERR ||
			  e_d1.type != CG_EXPR_VALUE)
			YLOG_ADD(YLOG_WARN, "Expression error ('dash1' parameter) [line %d].",
				 node->line_nbr);
		      else if (cg_expr(carta, dash2, &e_d2, carta->default_unit) != YENOERR ||
			       e_d2.type != CG_EXPR_VALUE)
			YLOG_ADD(YLOG_WARN, "Expression error ('dash2' parameter) [line %d].",
				 node->line_nbr);
		      else
			{
			  v_d1 = e_d1.value.value;
			  v_d2 = e_d2.value.value;
			  PDF_setdash(carta->p, YVAL2PT(v_d1), YVAL2PT(v_d2));
			}
		      free_var(&e_d1, &e_d1);
		      free_var(&e_d2, &e_d2);
		    }
		  if (rotation)
		    {
		      cg_expr_var_t e_d;
		      ystr_t ys_rot = ys_new("");
		      ys_printf(&ys_rot, "(%s)", rotation);
		      if (cg_expr(carta, ys_rot, &e_d, carta->default_unit) != YENOERR ||
			  e_d.type != CG_EXPR_SCALAR)
			YLOG_ADD(YLOG_WARN, "Expression error ('rotation' parameter) [line %d].",
				 node->line_nbr);
		      else
			{
			  double degrees = e_d.value.scalar;
			  cg_rotate(&v_x2, &v_y2, degrees);
			  PDF_rotate(carta->p, degrees);
			}
		      ys_del(&ys_rot);
		      free_var(&e_d, &e_d);
		    }
		  if (radius)
		    {
		      double tmp_x, tmp_y;
		      /* create a round-box */
		      tmp_x = YVAL2PT(v_x2) + YVAL2PT(v_r);
		      tmp_y = YVAL2PT(v_y2) + YVAL2PT(v_r);
		      PDF_arc(carta->p, tmp_x, tmp_y, YVAL2PT(v_r), 180.0, 270.0);
		      tmp_x = YVAL2PT(v_x2) + YVAL2PT(v_w) - YVAL2PT(v_r);
		      tmp_y = YVAL2PT(v_y2) + YVAL2PT(v_r);
		      PDF_arc(carta->p, tmp_x, tmp_y, YVAL2PT(v_r), 270.0, 0.0);
		      tmp_x = YVAL2PT(v_x2) + YVAL2PT(v_w) - YVAL2PT(v_r);
		      tmp_y = YVAL2PT(v_y2) + YVAL2PT(v_h) - YVAL2PT(v_r);
		      PDF_arc(carta->p, tmp_x, tmp_y, YVAL2PT(v_r), 0.0, 90.0);
		      tmp_x = YVAL2PT(v_x2) + YVAL2PT(v_r);
		      tmp_y = YVAL2PT(v_y2) + YVAL2PT(v_h) - YVAL2PT(v_r);
		      PDF_arc(carta->p, tmp_x, tmp_y, YVAL2PT(v_r), 90.0, 180.0);
		      PDF_closepath(carta->p);
		    }
		  else
		    {
		      /* create a straight box */
		      PDF_rect(carta->p, YVAL2PT(v_x2), YVAL2PT(v_y2),
			       YVAL2PT(v_w), YVAL2PT(v_h));
		    }
		  if (fill_color && line_color)
		    PDF_fill_stroke(carta->p);
		  else if (fill_color)
		    PDF_fill(carta->p);
		  else
		    PDF_stroke(carta->p);
		  PDF_restore(carta->p);
		}

	      /* put text */
	      PDF_save(carta->p);
	      if (opacity)
		{
		  ystr_t ys = ys_new("");
		  ys_printf(&ys, "opacityfill %s opacitystroke %s",
			    opacity, opacity);
		  gstate = PDF_create_gstate(carta->p, ys);
		  ys_del(&ys);
		  PDF_set_gstate(carta->p, gstate);
		}
	      if (blendmode)
		{
		  ystr_t ys = ys_new("");
		  ys_printf(&ys, "blendmode %s", blendmode);
		  gstate = PDF_create_gstate(carta->p, ys);
		  ys_del(&ys);
		  PDF_set_gstate(carta->p, gstate);
		}
	      if (rotation)
		{
		  cg_expr_var_t e_d;
		  ystr_t ys_rot = ys_new("");
		  ys_printf(&ys_rot, "(%s)", rotation);
		  if (cg_expr(carta, ys_rot, &e_d, carta->default_unit) != YENOERR ||
		      e_d.type != CG_EXPR_SCALAR)
		    YLOG_ADD(YLOG_WARN, "Expression error ('rotation' parameter) [line %d].",
			     node->line_nbr);
		  else
		    {
		      double degrees = e_d.value.scalar;
		      cg_rotate(&v_x, &v_y, degrees);
		      PDF_rotate(carta->p, degrees);
		    }
		  ys_del(&ys_rot);
		  free_var(&e_d, &e_d);
		}
	      if (border)
		{
		  cg_expr_var_t e_l = {0};
		  if (cg_expr(carta, border, &e_l, carta->default_unit) != YENOERR ||
		      e_l.type != CG_EXPR_VALUE)
		    {
		      YLOG_ADD(YLOG_WARN, "Expression error ('border' parameter) [line %d].",
			       node->line_nbr);
		      memset(&v_l, 0, sizeof(v_l));
		      border = NULL;
		    }
		  else
		    {
		      v_l = e_l.value.value;
		      v_x.value += YVAL2PT(v_l);
		      v_y.value += YVAL2PT(v_l);
		      v_w.value -= 2 * YVAL2PT(v_l);
		      v_h.value -= 2 * YVAL2PT(v_l);
		    }
		  free_var(&e_l, &e_l);
		}
	      /* x => v_x, y => v_y, w => v_w, h => v_h */
	      cg_write_words_in_area(carta, td, v_x, v_y, v_w, v_h);
	      PDF_restore(carta->p);
	    }
	}
    }
  free0(x);
  free0(y);
  free0(width);
  free0(height);
  free0(radius);
  free0(rotation);
  free0(opacity);
  free0(blendmode);
  free0(fill_color);
  free0(line_color);
  free0(line_width);
  free0(border);
  free0(dash1);
  free0(dash2);
  return (res);
}

/*
** cg_write_words_in_area()
*/
yerr_t cg_write_words_in_area(cg_t *carta, cg_text_define_t *td,
			      yvalue_t x, yvalue_t y, yvalue_t w, yvalue_t h)
{
  cg_text_block_t *block;
  ybool_t do_loop = YTRUE;
  int font_handle = -1;
  double current_offset_pt = 0.0, strw = 0.0, basefontsize = -1.0;
  ystr_t word = NULL, word2 = NULL;

  while (do_loop && (block = yv_pop(td->blocks)))
    {
      cg_fontdef_t *font = block->font;
      if (!yv_len(block->parts))
	{
	  free_text_block(&block, NULL);
	  continue ;
	}
      if ((font_handle = cg_get_font(carta, font->fontname)) == -1)
	{
	  YLOG_ADD(YLOG_WARN, "Unable to find font '%s'.", font->fontname);
	  free_text_block(&block, NULL);
	  continue ;
	}
      if (YVAL2PT(font->fontsize) > YVAL2PT(h))
	{
	  /* no enough vertical space - abort */
	  yv_put(&td->blocks, block);
	  break ;
	}
      /* set text attributes */
      PDF_set_value(carta->p, "charspacing", YVAL2PT(font->char_space));
      PDF_set_value(carta->p, "horizscaling", font->h_scale);
      if (font->underline)
	PDF_set_parameter(carta->p, "underline", "true");
      if (font->overline)
	PDF_set_parameter(carta->p, "overline", "true");
      if (font->strikeout)
	PDF_set_parameter(carta->p, "strikeout", "true");
      PDF_setcolor(carta->p, "fillstroke", "rgb", font->fill_color.value.color.red,
		   font->fill_color.value.color.green,
		   font->fill_color.value.color.blue, 0.0);
      PDF_setfont(carta->p, font_handle, YVAL2PT(font->fontsize));
      PDF_set_value(carta->p, "leading", YVAL2PT(font->fontsize) * font->line_space);

      while (YVAL2PT(font->fontsize) <= YVAL2PT(h) &&
	     yv_len(block->parts))
	{
	  cg_text_part_t *part = yv_pop(block->parts);
	  if (part->type == TEXT_PART_BR || part->type == TEXT_PART_PARA)
	    {
	      h.value = YVAL2PT(h) - YVAL2PT(font->fontsize);
	      h.unit = YUNIT_PT;
	      current_offset_pt = 0.0;
	      if (part->type == TEXT_PART_BR)
		{
		  basefontsize = -1.0;
		  free_text_part(part, NULL);
		}
	      else if (part->type == TEXT_PART_PARA)
		{
		  part->type = TEXT_PART_BR;
		  yv_put(&block->parts, part);
		}
	      continue ;
	    }
	  /* TEXT_PART_STRING */
	  if (basefontsize < 0)
	    basefontsize = YVAL2PT(font->fontsize);
	  if (YVAL2PT(font->fontsize) > basefontsize)
	    {
	      if (current_offset_pt == 0.0)
		h.value = YVAL2PT(h) + basefontsize -
		  YVAL2PT(font->fontsize);
	      else
		h.value = YVAL2PT(h) -
		  YVAL2PT(font->fontsize);
	      h.unit = YUNIT_PT;
	      current_offset_pt = 0.0;
	      basefontsize = -1.0;
	      yv_put(&block->parts, part);
	      continue ;
	    }
	  ys_trim(part->string);
	  if (!ys_len(part->string))
	    {
	      free_text_part(&part, NULL);
	      continue ;
	    }
	  word = ys_popword(part->string);
	  word2 = ys_subs(word, "&nbsp;", " ");
	  ys_del(&word);
	  word = word2;
	  word2 = NULL;
	  if ((strw = PDF_stringwidth(carta->p, word, font_handle,
				      YVAL2PT(font->fontsize))) >
	      (YVAL2PT(w) - current_offset_pt))
	    {
	      /* put the word back in string */
	      ys_putc(&part->string, ' ');
	      ys_tac(&part->string, word);
	      ys_del(&word);
	      /* CRLF */
	      h.value = YVAL2PT(h) - YVAL2PT(font->fontsize);
	      h.unit = YUNIT_PT;
	      current_offset_pt = 0.0;
	    }
	  else
	    {
	      if (ys_len(part->string))
		ys_addc(&word, ' ');
	      strw = PDF_stringwidth(carta->p, word, font_handle,
				     YVAL2PT(font->fontsize));
	      PDF_fit_textline(carta->p, word, 0,
			       YVAL2PT(x) + current_offset_pt,
			       YVAL2PT(y) + YVAL2PT(h) - basefontsize,
			       "");
	      current_offset_pt += strw;
	    }
	  if (ys_len(part->string))
	    yv_put(&block->parts, part);
	}
      /* check if there is vertical space left */
      if (YVAL2PT(font->fontsize) > YVAL2PT(h))
	do_loop = YFALSE;

      if (yv_len(block->parts))
	yv_put(&td->blocks, block);

      if (font->underline)
	PDF_set_parameter(carta->p, "underline", "false");
      if (font->overline)
	PDF_set_parameter(carta->p, "overline", "false");
      if (font->strikeout)
	PDF_set_parameter(carta->p, "strikeout", "false");
    }
  PDF_set_value(carta->p, "charspacing", 0.0);
  PDF_set_value(carta->p, "horizscaling", 100);
  return (YENOERR);
}

/*
** free_text_define()
*/
void free_text_define(void *td, void *data)
{
  cg_text_define_t *def = td;

  if (!td)
    return ;
  free0(def->id);
  if (!data)
    free0(def);
}

/*
** free_text_block()
*/
void free_text_block(void *tb, void *data)
{
  cg_text_block_t *block = tb;

  if (!tb)
    return ;
  cg_del_fontdef(block->font);
  yv_del(&block->parts, free_text_part, NULL);
  if (!data)
    free0(block);
}

/*
** free_text_part()
*/
void free_text_part(void *tp, void *data)
{
  cg_text_part_t *part = tp;

  if (!tp)
    return ;
  if (part->type == TEXT_PART_STRING)
    ys_del(&part->string);
  if (!data)
    free0(part);
}
