#include <math.h>
#include "cartagenius.h"

/*
** cg_process_card()
** Process the front of a card.
*/
yerr_t cg_process_card(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		       yvalue_t card_x, yvalue_t card_y)
{
  ydom_node_t *node;
  char *template = NULL;

  /* process card's initial templates */
  template = ydom_node_get_attr_value(card, "template");
  cg_process_template(carta, deck, card, template, card_x, card_y);
  free0(template);
  /* process card's data */
  for (node = ydom_node_get_first_child(card); node;
       node = ydom_node_get_next(node))
    {
      if (!ydom_node_is_element(node))
	continue ;
      if (!strcasecmp(node->name, "text"))
	cg_put_text(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "image"))
	cg_put_image(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "circle"))
	cg_put_circle(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "line"))
	cg_put_line(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "box") ||
	       !strcasecmp(node->name, "round-box"))
	cg_put_box(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "bezier"))
	cg_put_bezier(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "polygon"))
	cg_put_polygon(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "grid"))
	cg_put_grid(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "hexagon"))
	cg_put_hexagon(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "var"))
	cg_put_var(carta, node);
      else if (!strcasecmp(node->name, "text-define") ||
	       !strcasecmp(node->name, "textdef"))
	cg_put_text_define(carta, node);
      else if (!strcasecmp(node->name, "text-area") ||
	       !strcasecmp(node->name, "textbox"))
	cg_put_text_area(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "template"))
	{
	  template = ydom_node_get_attr_value(node, "id");
	  cg_process_template(carta, deck, card, template, card_x, card_y);
	  free0(template);
	}
      else if (!strcasecmp(node->name, "if"))
	cg_process_if(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "while"))
	cg_process_while(carta, deck, card, node, card_x, card_y);
      else if (!strcasecmp(node->name, "log"))
	cg_put_log(carta, node);
    }
  return (YENOERR);
}

/*
** cg_process_back()
** Process the back of a card.
*/
yerr_t cg_process_back(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		       yvalue_t card_x, yvalue_t card_y)
{
  ydom_node_t *node;
  int i;
  char *template = NULL, *id = NULL;
  char *pt = NULL, *pt2 = NULL, *pt_max = NULL;

  pt = template = ydom_node_get_attr_value(card, "back");
  pt_max = pt ? (pt + strlen(pt)) : NULL;
  for (; pt < pt_max; pt = pt2)
    {
      ybool_t processed = YFALSE;

      for (pt2 = pt; *pt2 && *pt2 != COMMA && *pt2 != SEMICOLON &&
	     !IS_SPACE(*pt2); ++pt2)
	;
      while (*pt2 == COMMA || *pt2 == SEMICOLON || IS_SPACE(*pt2))
	*pt2++ = '\0';
      for (i = 0; pt && i < yv_len(carta->templates); ++i)
	{
	  node = carta->templates[i];
	  id = ydom_node_get_attr_value(node, "id");
	  if (id && !strcmp(id, pt))
	    {
	      processed = YTRUE;
	      cg_process_card(carta, deck, node, card_x, card_y);
	    }
	  free0(id);
	}
      if (!processed)
	YLOG_ADD(YLOG_WARN, "No template '%s' found [line %d].",
		 pt, card->line_nbr);
    }
  free0(template);
  return (YENOERR);
}

/*
** cg_process_template()
** Process a template line (comma separated template names).
*/
yerr_t cg_process_template(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
			   char *templates, yvalue_t card_x, yvalue_t card_y)
{
  int i;
  ydom_node_t *node;
  char *id = NULL, *pt = NULL, *pt2 = NULL, *pt_max = NULL;

  if (!templates)
    return (YENOERR);
  if (!strlen(templates))
    {
      YLOG_ADD(YLOG_WARN, "Empty template name [line %d].",
	       card->line_nbr);
      return (YEUNDEF);
    }
  pt = templates;
  pt_max = pt + strlen(pt);
  for (; pt < pt_max; pt = pt2)
    {
      ybool_t processed = YFALSE;

      for (pt2 = pt; *pt2 && *pt2 != COMMA && *pt2 != SEMICOLON &&
	     !IS_SPACE(*pt2); ++pt2)
	;
      while (*pt2 == COMMA || *pt2 == SEMICOLON || IS_SPACE(*pt2))
	*pt2++ = '\0';
      for (i = 0; pt && i < yv_len(carta->templates) && !processed; i++)
	{
	  node = carta->templates[i];
	  id = ydom_node_get_attr_value(node, "id");
	  if (id && !strcmp(id, pt))
	    {
	      processed = YTRUE;
	      cg_process_card(carta, deck, node, card_x, card_y);
	    }
	  free0(id);
	}
      if (!processed)
	YLOG_ADD(YLOG_WARN, "No template '%s' found [line %d].",
		 pt, card->line_nbr);
    }
  return (YENOERR);
}

/*
** cg_process_if()
** Process if instruction.
*/
yerr_t cg_process_if(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		     ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *test = NULL;
  yerr_t res = YENOERR;
  ystr_t ys_test;
  cg_expr_var_t e_test = {0}, e_bool = {0};

  if (!(test = ydom_node_get_attr_value(node, "test")))
    {
      YLOG_ADD(YLOG_ERR, "Empty if declaration [line %d].",
	       node->line_nbr);
      return (YEINVAL);
    }
  ys_test = ys_new("");
  ys_printf(&ys_test, "(%s)", test);
  free0(test);
  if (cg_expr(carta, ys_test, &e_test, carta->default_unit) != YENOERR ||
      cg_expr_f_bool(carta, &e_bool, e_test) != YENOERR)
    {
      YLOG_ADD(YLOG_WARN, "Expression error ('test' parameter) [line %d].",
	       node->line_nbr);
      ys_del(&ys_test);
      free_var(&e_test, &e_test);
      free_var(&e_bool, &e_bool);
      return (YEINVAL);
    }
  if (e_bool.value.boolean)
    res = cg_process_card(carta, deck, node, card_x, card_y);
  ys_del(&ys_test);
  free_var(&e_test, &e_test);
  free_var(&e_bool, &e_bool);
  return (res);
}

/*
** cg_process_while()
** Process while instruction.
*/
yerr_t cg_process_while(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
			ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *test = NULL;
  yerr_t res = YENOERR;
  ystr_t ys_test;
  cg_expr_var_t e_test = {0}, e_bool = {0};
  static int nested_loops = 0;
  int inner_loops = 0;

  if (nested_loops > CG_MAX_NESTED_LOOPS)
    {
      YLOG_ADD(YLOG_WARN, "Too many nested loops [line %d].",
	       node->line_nbr);
      return (YELOOP);
    }
  nested_loops++;
  while (res == YENOERR)
    {
      if (res != YENOERR)
	break ;
      if (inner_loops > CG_MAX_LOOP_ITERATIONS)
	{
	  YLOG_ADD(YLOG_WARN, "Too many loop iterations [line %d].",
		   node->line_nbr);
	  nested_loops--;
	  return (YELOOP);
	}
      inner_loops++;
      if (!(test = ydom_node_get_attr_value(node, "test")))
	{
	  YLOG_ADD(YLOG_ERR, "Empty while declaration [line %d].",
		   node->line_nbr);
	  nested_loops--;
	  return (YEINVAL);
	}
      ys_test = ys_new("");
      ys_printf(&ys_test, "(%s)", test);
      free0(test);
      if (cg_expr(carta, ys_test, &e_test, carta->default_unit) != YENOERR ||
	  cg_expr_f_bool(carta, &e_bool, e_test) != YENOERR)
	{
	  YLOG_ADD(YLOG_WARN, "Expression error ('test' parameter) [line %d].",
		   node->line_nbr);
	  ys_del(&ys_test);
	  free_var(&e_test, &e_test);
	  free_var(&e_bool, &e_bool);
	  nested_loops--;
	  return (YEINVAL);
	}
      if (e_bool.value.boolean)
	res = cg_process_card(carta, deck, node, card_x, card_y);
      else
	res = YEPERM;
      ys_del(&ys_test);
      free_var(&e_test, &e_test);
      free_var(&e_bool, &e_bool);
    }
  nested_loops--;
  return (res);
}

/*
** cg_card_count()
** Returns the number of needed processing of the same card.
*/
int cg_card_count(cg_t *carta, ydom_node_t *card)
{
  char* count = NULL;
  ystr_t ys_count;
  int v_count = 1;
  
  count = ydom_node_get_attr_value(card, "count");
  if (!count)
    return (1);
  else
    {
      cg_expr_var_t e_count = {0};
      ys_count = ys_new("");
      ys_printf(&ys_count, "(%s)", count);
      if (cg_expr(carta, ys_count, &e_count, carta->default_unit) != YENOERR ||
	  e_count.type != CG_EXPR_SCALAR)
	YLOG_ADD(YLOG_WARN, "Expression error ('count' parameter) [line %d].",
		 card->line_nbr);
      else
	v_count = e_count.value.scalar;
      ys_del(&ys_count);
      free_var(&e_count, &e_count);
    }
  free0(count);
  return (v_count);
}

/*
** cg_put_var()
** Put a variable in user space.
*/
yerr_t cg_put_var(cg_t *carta, ydom_node_t *node)
{
  ydom_node_t *subnode;
  char *name = NULL, *precision = NULL;
  ystr_t text, text2;
  cg_expr_var_t var = {0};
  yerr_t res_val;
  int old_precision = carta->expr->precision;

  if (!(name = ydom_node_get_attr_value(node, "name")) ||
      !strlen(name))
    {
      free0(name);
      YLOG_ADD(YLOG_WARN, "Empty variable name [line %d].", node->line_nbr);
      return (YEINVAL);
    }
  text = ys_new("");
  for (subnode = ydom_node_get_first_child(node); subnode;
       subnode = ydom_node_get_next(subnode))
    {
      if (ydom_node_is_text(subnode))
	{
	  char *string = ydom_node_get_value(subnode);
	  ys_cat(&text, string);
	  free0(string);
	}
      else if (ydom_node_is_element(subnode) && subnode->name)
	{
	  if (!strcasecmp(subnode->name, "br"))
	    ys_cat(&text, "\n");
	  else if (!strcasecmp(subnode->name, "p"))
	    ys_cat(&text, "\n\n");
	}
    }
  text2 = ys_subs(text, "&nbsp;", " ");

  /* processing */
  if ((precision = ydom_node_get_attr_value(node, "precision")))
    cg_expr_set_precision(carta, precision);
  ys_printf(&text, "(%s)", text2);
  ys_del(&text2);
  res_val = cg_expr(carta, text, &var, carta->default_unit);
  ys_del(&text);
  carta->expr->precision = old_precision;
  free0(precision);
  if (res_val != YENOERR)
    {
      free0(name);
      return (res_val);
    }
  if (var.type == CG_EXPR_SCALAR)
    res_val = cg_expr_set_scalar(carta, name, var.value.scalar);
  else if (var.type == CG_EXPR_VALUE)
    res_val = cg_expr_set_value(carta, name, var.value.value);
  else if (var.type == CG_EXPR_STRING)
    res_val = cg_expr_set_string(carta, name, var.value.string);
  else if (var.type == CG_EXPR_ELEMENT)
    res_val = cg_expr_set_element(carta, name, var.value.element.width,
				  var.value.element.height);
  else if (var.type == CG_EXPR_COLOR)
    res_val = cg_expr_set_color(carta, name, var.value.color.red,
				var.value.color.green, var.value.color.blue);
  else if (var.type == CG_EXPR_BOOL)
    res_val = cg_expr_set_bool(carta, name, var.value.boolean);
  free_var(&var, &var);
  free0(name);
  return (YENOERR);
}

/*
** cg_put_log()
** Write a log message.
*/
yerr_t cg_put_log(cg_t *carta, ydom_node_t *node)
{
  ydom_node_t *subnode;
  char *precision = NULL;
  ystr_t text, text2;
  cg_expr_var_t var = {0}, str_var = {0};
  yerr_t res_val = YENOERR;
  int old_precision = carta->expr->precision;

  text = ys_new("");
  for (subnode = ydom_node_get_first_child(node); subnode;
       subnode = ydom_node_get_next(subnode))
    {
      if (ydom_node_is_text(subnode))
	{
	  char *string = ydom_node_get_value(subnode);
	  ys_cat(&text, string);
	  free0(string);
	}
      else if (ydom_node_is_element(subnode) && subnode->name)
	{
	  if (!strcasecmp(subnode->name, "br"))
	    ys_cat(&text, "\n");
	  else if (!strcasecmp(subnode->name, "p"))
	    ys_cat(&text, "\n\n");
	}
    }
  text2 = ys_subs(text, "&nbsp;", " ");

  /* processing */
  if ((precision = ydom_node_get_attr_value(node, "precision")))
    cg_expr_set_precision(carta, precision);
  ys_printf(&text, "(%s)", text2);
  ys_del(&text2);
  if ((res_val = cg_expr(carta, text, &var, carta->default_unit)) != YENOERR ||
      (res_val = cg_expr_f_string(carta, &str_var, var)) != YENOERR ||
      !str_var.value.string)
    YLOG_ADD(YLOG_WARN, "Problem to write log [line %d].", node->line_nbr);
  else
    YLOG(str_var.value.string);
  ys_del(&text);
  carta->expr->precision = old_precision;
  free_var(&var, &var);
  free_var(&str_var, &str_var);
  return (res_val);
}

/*
** cg_put_line()
** Put a line in PDF.
*/
yerr_t cg_put_line(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		   ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *x1 = NULL, *y1 = NULL, *x2 = NULL, *y2 = NULL;
  char *color = NULL, *width = NULL, *opacity = NULL;
  char *dash1 = NULL, *dash2 = NULL, *blendmode = NULL;
  yvalue_t v_x1, v_y1, v_x2, v_y2, v_l, v_d1, v_d2;
  yerr_t res = YENOERR;
  int gstate = -1;

  x1 = ydom_node_get_attr_value(node, "x1");
  y1 = ydom_node_get_attr_value(node, "y1");
  x2 = ydom_node_get_attr_value(node, "x2");
  y2 = ydom_node_get_attr_value(node, "y2");
  width = ydom_node_get_attr_value(node, "width");
  color = ydom_node_get_attr_value(node, "color");
  dash1 = ydom_node_get_attr_value(node, "dash1");
  dash2 = ydom_node_get_attr_value(node, "dash2");
  opacity = ydom_node_get_attr_value(node, "opacity");
  blendmode = ydom_node_get_attr_value(node, "blendmode");

  if (!x1 || !y1 || !x2 || !y2 || !color)
    {
      YLOG_ADD(YLOG_ERR, "Empty line declaration [line %d].", node->line_nbr);
      res = YEUNDEF;
    }
  else
    {
      cg_expr_var_t e_x1 = {0}, e_y1 = {0}, e_x2 = {0}, e_y2 = {0};
      cg_expr_var_t e_l = {0}, e_c = {0}, e_d1 = {0}, e_d2 = {0};

      if (cg_expr(carta, x1, &e_x1, carta->default_unit) != YENOERR ||
	  e_x1.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('x1' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, y1, &e_y1, carta->default_unit) != YENOERR ||
	       e_y1.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('y1' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, x2, &e_x2, carta->default_unit) != YENOERR ||
	  e_x2.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('x2' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, y2, &e_y2, carta->default_unit) != YENOERR ||
	       e_y2.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('y2' parameter) [line %d].",
		 node->line_nbr);
      else
	{
	  v_x1 = e_x1.value.value;
	  v_y1 = e_y1.value.value;
	  v_x2 = e_x2.value.value;
	  v_y2 = e_y2.value.value;
	  v_x1.value = YVAL2PT(v_x1) + YVAL2PT(card_x);
	  v_y1.value = YVAL2PT(v_y1) + YVAL2PT(card_y);
	  v_x2.value = YVAL2PT(v_x2) + YVAL2PT(card_x);
	  v_y2.value = YVAL2PT(v_y2) + YVAL2PT(card_y);
	  v_x1.unit = v_y1.unit = v_x2.unit = v_y2.unit = YUNIT_PT;
	  PDF_save(carta->p);
	  if (opacity)
	    {
	      ystr_t ys = ys_new("");
	      ys_printf(&ys, "opacityfill %s opacitystroke %s", opacity, opacity);
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
	  if (color)
	    {
	      if (cg_expr(carta, color, &e_c, carta->default_unit) != YENOERR ||
		  e_c.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "stroke", "rgb", e_c.value.color.red,
			     e_c.value.color.green, e_c.value.color.blue, 0.0);
	    }
	  if (width)
	    {
	      if (cg_expr(carta, width, &e_l, carta->default_unit) != YENOERR ||
		  e_l.type != CG_EXPR_VALUE)
		YLOG_ADD(YLOG_WARN, "Expression error ('width' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  v_l = e_l.value.value;
		  PDF_setlinewidth(carta->p, YVAL2PT(v_l));
		}
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
	    }
	  PDF_moveto(carta->p, YVAL2PT(v_x1), YVAL2PT(v_y1));
	  PDF_lineto(carta->p, YVAL2PT(v_x2), YVAL2PT(v_y2));
	  PDF_stroke(carta->p);
	  PDF_restore(carta->p);
	}
      free_var(&e_x1, &e_x1);
      free_var(&e_y1, &e_y1);
      free_var(&e_x2, &e_x2);
      free_var(&e_y2, &e_y2);
      free_var(&e_l, &e_l);
      free_var(&e_c, &e_c);
      free_var(&e_d1, &e_d1);
      free_var(&e_d2, &e_d2);
    }
  free0(x1);
  free0(y1);
  free0(x2);
  free0(y2);
  free0(width);
  free0(color);
  free0(dash1);
  free0(dash2);
  free0(opacity);
  free0(blendmode);
  return (res);
}

/*
** cg_put_bezier()
** Put a Bezier line in PDF.
*/
yerr_t cg_put_bezier(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		     ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *x1 = NULL, *y1 = NULL, *x2 = NULL, *y2 = NULL;
  char *cp_x1 = NULL, *cp_y1 = NULL, *cp_x2 = NULL, *cp_y2 = NULL;
  char *color = NULL, *width = NULL, *opacity = NULL;
  char *dash1 = NULL, *dash2 = NULL, *blendmode = NULL;
  yvalue_t v_x1, v_y1, v_x2, v_y2, v_cp_x1, v_cp_y1, v_cp_x2, v_cp_y2, v_l;
  yvalue_t v_d1, v_d2;
  yerr_t res = YENOERR;
  int gstate = -1;

  x1 = ydom_node_get_attr_value(node, "x1");
  y1 = ydom_node_get_attr_value(node, "y1");
  x2 = ydom_node_get_attr_value(node, "x2");
  y2 = ydom_node_get_attr_value(node, "y2");
  cp_x1 = ydom_node_get_attr_value(node, "cp-x1");
  cp_y1 = ydom_node_get_attr_value(node, "cp-y1");
  cp_x2 = ydom_node_get_attr_value(node, "cp-x2");
  cp_y2 = ydom_node_get_attr_value(node, "cp-y2");
  width = ydom_node_get_attr_value(node, "width");
  color = ydom_node_get_attr_value(node, "color");
  dash1 = ydom_node_get_attr_value(node, "dash1");
  dash2 = ydom_node_get_attr_value(node, "dash2");
  opacity = ydom_node_get_attr_value(node, "opacity");
  blendmode = ydom_node_get_attr_value(node, "blendmode");

  if (!x1 || !y1 || !x2 || !y2 || !cp_x1 || !cp_y1 ||
      !cp_x2 || !cp_y2 || !color)
    {
      YLOG_ADD(YLOG_ERR, "Empty line declaration [line %d].", node->line_nbr);
      res = YEUNDEF;
    }
  else
    {
      cg_expr_var_t e_x1 = {0}, e_y1 = {0}, e_x2 = {0}, e_y2 = {0};
      cg_expr_var_t e_cp_x1 = {0}, e_cp_y1 = {0}, e_cp_x2 = {0};
      cg_expr_var_t e_cp_y2 = {0}, e_l = {0}, e_c = {0}, e_d1 = {0}, e_d2 = {0};

      if (cg_expr(carta, x1, &e_x1, carta->default_unit) != YENOERR ||
	  e_x1.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('x1' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, y1, &e_y1, carta->default_unit) != YENOERR ||
	       e_y1.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('y1' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, x2, &e_x2, carta->default_unit) != YENOERR ||
	  e_x2.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('x2' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, y2, &e_y2, carta->default_unit) != YENOERR ||
	       e_y2.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('y2' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, cp_x1, &e_cp_x1, carta->default_unit) != YENOERR ||
	       e_cp_x1.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('cp-x1' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, cp_y1, &e_cp_y1, carta->default_unit) != YENOERR ||
	       e_cp_y1.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('cp-y1' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, cp_x2, &e_cp_x2, carta->default_unit) != YENOERR ||
	       e_cp_x2.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('cp-x2' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, cp_y2, &e_cp_y2, carta->default_unit) != YENOERR ||
	       e_cp_y2.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('cp-y2' parameter) [line %d].",
		 node->line_nbr);
      else
	{
	  v_x1 = e_x1.value.value;
	  v_y1 = e_y1.value.value;
	  v_x2 = e_x2.value.value;
	  v_y2 = e_y2.value.value;
	  v_cp_x1 = e_cp_x1.value.value;
	  v_cp_y1 = e_cp_y1.value.value;
	  v_cp_x2 = e_cp_x2.value.value;
	  v_cp_y2 = e_cp_y2.value.value;
	  v_x1.value = YVAL2PT(v_x1) + YVAL2PT(card_x);
	  v_y1.value = YVAL2PT(v_y1) + YVAL2PT(card_y);
	  v_x2.value = YVAL2PT(v_x2) + YVAL2PT(card_x);
	  v_y2.value = YVAL2PT(v_y2) + YVAL2PT(card_y);
	  v_cp_x1.value = YVAL2PT(v_cp_x1) + YVAL2PT(card_x);
	  v_cp_y1.value = YVAL2PT(v_cp_y1) + YVAL2PT(card_y);
	  v_cp_x2.value = YVAL2PT(v_cp_x2) + YVAL2PT(card_x);
	  v_cp_y2.value = YVAL2PT(v_cp_y2) + YVAL2PT(card_y);
	  v_x1.unit = v_y1.unit = v_x2.unit = v_y2.unit = v_cp_x1.unit =
	    v_cp_y1.unit = v_cp_x2.unit = v_cp_y2.unit = YUNIT_PT;
	  PDF_save(carta->p);
	  if (opacity)
	    {
	      ystr_t ys = ys_new("");
	      ys_printf(&ys, "opacityfill %s opacitystroke %s", opacity, opacity);
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
	  if (color)
	    {
	      if (cg_expr(carta, color, &e_c, carta->default_unit) != YENOERR ||
		  e_c.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "stroke", "rgb", e_c.value.color.red,
			     e_c.value.color.green, e_c.value.color.blue, 0.0);
	    }
	  if (width)
	    {
	      if (cg_expr(carta, width, &e_l, carta->default_unit) != YENOERR ||
		  e_l.type != CG_EXPR_VALUE)
		YLOG_ADD(YLOG_WARN, "Expression error ('width' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  v_l = e_l.value.value;
		  PDF_setlinewidth(carta->p, YVAL2PT(v_l));
		}
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
	    }
	  PDF_moveto(carta->p, YVAL2PT(v_x1), YVAL2PT(v_y1));
	  PDF_curveto(carta->p, YVAL2PT(v_cp_x1), YVAL2PT(v_cp_y1),
		      YVAL2PT(v_cp_x2), YVAL2PT(v_cp_y2),
		      YVAL2PT(v_x2), YVAL2PT(v_y2));
	  PDF_stroke(carta->p);
	  PDF_restore(carta->p);
	}
      free_var(&e_x1, &e_x1);
      free_var(&e_y1, &e_y1);
      free_var(&e_x2, &e_x2);
      free_var(&e_y2, &e_y2);
      free_var(&e_cp_x1, &e_cp_x1);
      free_var(&e_cp_y1, &e_cp_y1);
      free_var(&e_cp_x2, &e_cp_x2);
      free_var(&e_cp_y2, &e_cp_y2);
      free_var(&e_l, &e_l);
      free_var(&e_c, &e_c);
      free_var(&e_d1, &e_d1);
      free_var(&e_d2, &e_d2);
    }
  free0(x1);
  free0(y1);
  free0(x2);
  free0(y2);
  free0(cp_x1);
  free0(cp_y1);
  free0(cp_x2);
  free0(cp_y2);
  free0(width);
  free0(color);
  free0(dash1);
  free0(dash2);
  free0(opacity);
  free0(blendmode);
  return (res);
}

/*
** cg_put_polygon()
** Put a polygon in PDF.
*/
yerr_t cg_put_polygon(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		      ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *x = NULL, *y = NULL;
  char *line_width = NULL, *line_color = NULL, *fill_color = NULL;
  char *rotation = NULL, *opacity = NULL, *blendmode = NULL;
  char *dash1 = NULL, *dash2 = NULL;
  yvalue_t v_x, v_y, v_l, v_d1, v_d2;
  yerr_t res = YENOERR;
  int gstate = -1;
  double degrees;
  ystr_t ys_rot;
  ydom_node_t *subnode;

  x = ydom_node_get_attr_value(node, "x");
  y = ydom_node_get_attr_value(node, "y");
  line_width = ydom_node_get_attr_value(node, "line-width");
  line_color = ydom_node_get_attr_value(node, "line-color");
  fill_color = ydom_node_get_attr_value(node, "fill-color");
  rotation = ydom_node_get_attr_value(node, "rotation");
  dash1 = ydom_node_get_attr_value(node, "dash1");
  dash2 = ydom_node_get_attr_value(node, "dash2");
  opacity = ydom_node_get_attr_value(node, "opacity");
  blendmode = ydom_node_get_attr_value(node, "blendmode");

  if (!x || !y || (!fill_color && !line_color))
    {
      YLOG_ADD(YLOG_WARN, "Empty polygon declaration [line %d].", node->line_nbr);
      res = YEUNDEF;
    }
  else
    {
      cg_expr_var_t e_x = {0}, e_y = {0}, e_l = {0}, e_d = {0};
      cg_expr_var_t e_lc = {0}, e_fc = {0}, e_d1 = {0}, e_d2 = {0};

      if (cg_expr(carta, x, &e_x, carta->default_unit) != YENOERR ||
	  e_x.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('x' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, y, &e_y, carta->default_unit) != YENOERR ||
	       e_y.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('y' parameter) [line %d].",
		 node->line_nbr);
      else
	{
	  v_x = e_x.value.value;
	  v_y = e_y.value.value;
	  v_x.value = YVAL2PT(v_x) + YVAL2PT(card_x);
	  v_y.value = YVAL2PT(v_y) + YVAL2PT(card_y);
	  v_x.unit = v_y.unit = YUNIT_PT;
	  PDF_save(carta->p);
	  if (opacity)
	    {
	      ystr_t ys = ys_new("");
	      ys_printf(&ys, "opacityfill %s opacitystroke %s", opacity, opacity);
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
	      if (cg_expr(carta, fill_color, &e_fc, carta->default_unit) != YENOERR ||
		  e_fc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('fill-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "fill", "rgb", e_fc.value.color.red,
			     e_fc.value.color.green, e_fc.value.color.blue, 0.0);
	    }
	  if (line_color)
	    {
	      if (cg_expr(carta, line_color, &e_lc, carta->default_unit) != YENOERR ||
		  e_lc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "stroke", "rgb", e_lc.value.color.red,
			     e_lc.value.color.green, e_lc.value.color.blue, 0.0);
	    }
	  if (line_width)
	    {
	      if (cg_expr(carta, line_width, &e_l, carta->default_unit) != YENOERR ||
		  e_l.type != CG_EXPR_VALUE)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-width' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  v_l = e_l.value.value;
		  PDF_setlinewidth(carta->p, YVAL2PT(v_l));
		}
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
	    }
	  if (rotation)
	    {
	      ys_rot = ys_new("");
	      ys_printf(&ys_rot, "(%s)", rotation);
	      if (cg_expr(carta, ys_rot, &e_d, carta->default_unit) != YENOERR ||
		  e_d.type != CG_EXPR_SCALAR)
		YLOG_ADD(YLOG_WARN, "Expression error ('rotation' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  degrees = e_d.value.scalar;
		  cg_rotate(&v_x, &v_y, degrees);
		  PDF_rotate(carta->p, degrees);
		}
	      ys_del(&ys_rot);
	    }
	  PDF_moveto(carta->p, YVAL2PT(v_x), YVAL2PT(v_y));
	  for (subnode = ydom_node_get_first_child(node); subnode;
	       subnode = ydom_node_get_next(subnode))
	    {
	      char *line_x = NULL, *line_y = NULL, *line_cp_x1 = NULL;
	      char *line_cp_y1 = NULL, *line_cp_x2 = NULL, *line_cp_y2 = NULL;
	      cg_expr_var_t e_line_x = {0}, e_line_y = {0};
	      cg_expr_var_t e_line_cp_x1 = {0}, e_line_cp_y1 = {0};
	      cg_expr_var_t e_line_cp_x2 = {0}, e_line_cp_y2 = {0};
	      yvalue_t v_line_x, v_line_y, v_line_cp_x1, v_line_cp_y1;
	      yvalue_t v_line_cp_x2, v_line_cp_y2;
	      ybool_t line_ok = YFALSE, bezier_ok = YFALSE;

	      if (!ydom_node_is_element(subnode))
		continue ;
	      if (!strcasecmp(subnode->name, "line") ||
		  !strcasecmp(subnode->name, "bezier"))
		{
		  line_x = ydom_node_get_attr_value(subnode, "x");
		  line_y = ydom_node_get_attr_value(subnode, "y");
		  if (cg_expr(carta, line_x, &e_line_x,
			      carta->default_unit) != YENOERR ||
		      e_line_x.type != CG_EXPR_VALUE)
		    YLOG_ADD(YLOG_WARN, "Bad line parameter ('x') [line %d].",
			     node->line_nbr);
		  else if (cg_expr(carta, line_y, &e_line_y,
				   carta->default_unit) != YENOERR ||
			   e_line_y.type != CG_EXPR_VALUE)
		    YLOG_ADD(YLOG_WARN, "Bad line parameter ('y') [line %d].",
			     node->line_nbr);
		  else
		    {
		      v_line_x = e_line_x.value.value;
		      v_line_y = e_line_y.value.value;
		      v_line_x.value = YVAL2PT(v_line_x) + YVAL2PT(card_x);
		      v_line_y.value = YVAL2PT(v_line_y) + YVAL2PT(card_y);
		      v_line_x.unit = v_line_y.unit = YUNIT_PT;
		      line_ok = YTRUE;
		    }
		}
	      if (!strcasecmp(subnode->name, "bezier"))
		{
		  line_cp_x1 = ydom_node_get_attr_value(subnode, "cp-x1");
		  line_cp_y1 = ydom_node_get_attr_value(subnode, "cp-y1");
		  line_cp_x2 = ydom_node_get_attr_value(subnode, "cp-x2");
		  line_cp_y2 = ydom_node_get_attr_value(subnode, "cp-y2");
		  if (cg_expr(carta, line_cp_x1, &e_line_cp_x1,
			      carta->default_unit) != YENOERR ||
		      e_line_cp_x1.type != CG_EXPR_VALUE)
		    YLOG_ADD(YLOG_WARN, "Bad line parameter ('cp-x1') [line %d].",
			     node->line_nbr);
		  else if (cg_expr(carta, line_cp_y1, &e_line_cp_y1,
				   carta->default_unit) != YENOERR ||
			   e_line_cp_y1.type != CG_EXPR_VALUE)
		    YLOG_ADD(YLOG_WARN, "Bad line parameter ('cp-y1') [line %d].",
			     node->line_nbr);
		  else if (cg_expr(carta, line_cp_x2, &e_line_cp_x2,
				   carta->default_unit) != YENOERR ||
			   e_line_cp_x2.type != CG_EXPR_VALUE)
		    YLOG_ADD(YLOG_WARN, "Bad line parameter ('cp-x2') [line %d].",
			     node->line_nbr);
		  else if (cg_expr(carta, line_cp_y2, &e_line_cp_y2,
				   carta->default_unit) != YENOERR ||
			   e_line_cp_y2.type != CG_EXPR_VALUE)
		    YLOG_ADD(YLOG_WARN, "Bad line parameter ('cp-y2') [line %d].",
			     node->line_nbr);
		  else
		    {
		      v_line_cp_x1 = e_line_cp_x1.value.value;
		      v_line_cp_y1 = e_line_cp_y1.value.value;
		      v_line_cp_x2 = e_line_cp_x2.value.value;
		      v_line_cp_y2 = e_line_cp_y2.value.value;
		      v_line_cp_x1.value = YVAL2PT(v_line_cp_x1) + YVAL2PT(card_x);
		      v_line_cp_y1.value = YVAL2PT(v_line_cp_y1) + YVAL2PT(card_y);
		      v_line_cp_x2.value = YVAL2PT(v_line_cp_x2) + YVAL2PT(card_x);
		      v_line_cp_y2.value = YVAL2PT(v_line_cp_y2) + YVAL2PT(card_y);
		      v_line_cp_x1.unit = v_line_cp_y1.unit =
			v_line_cp_x2.unit = v_line_cp_y2.unit = YUNIT_PT;
		      bezier_ok = YTRUE;
		    }
		}
	      if (line_ok && !strcasecmp(subnode->name, "line"))
		PDF_lineto(carta->p, YVAL2PT(v_line_x), YVAL2PT(v_line_y));
	      else if (line_ok && bezier_ok && !strcasecmp(subnode->name, "bezier"))
		PDF_curveto(carta->p, YVAL2PT(v_line_cp_x1), YVAL2PT(v_line_cp_y1),
			    YVAL2PT(v_line_cp_x2), YVAL2PT(v_line_cp_y2),
			    YVAL2PT(v_line_x), YVAL2PT(v_line_y));
	      free0(line_x);
	      free0(line_y);
	      free0(line_cp_x1);
	      free0(line_cp_y1);
	      free0(line_cp_x2);
	      free0(line_cp_y2);
	      free_var(&e_line_x, &e_line_x);
	      free_var(&e_line_y, &e_line_y);
	      free_var(&e_line_cp_x1, &e_line_cp_x1);
	      free_var(&e_line_cp_y1, &e_line_cp_y1);
	      free_var(&e_line_cp_x2, &e_line_cp_x2);
	      free_var(&e_line_cp_y2, &e_line_cp_y2);
	    }
	  PDF_closepath(carta->p);
	  if (fill_color && line_color)
	    PDF_fill_stroke(carta->p);
	  else if (fill_color)
	    PDF_fill(carta->p);
	  else
	    PDF_stroke(carta->p);
	  PDF_restore(carta->p);
	}
      free_var(&e_x, &e_x);
      free_var(&e_y, &e_y);
      free_var(&e_l, &e_l);
      free_var(&e_d, &e_d);
      free_var(&e_fc, &e_fc);
      free_var(&e_lc, &e_lc);
      free_var(&e_d1, &e_d1);
      free_var(&e_d2, &e_d2);
    }
  free0(x);
  free0(y);
  free0(line_width);
  free0(line_color);
  free0(fill_color);
  free0(rotation);
  free0(dash1);
  free0(dash2);
  free0(opacity);
  free0(blendmode);
  return (res);
}

/*
** cg_put_circle()
** Put a circle in PDF.
*/
yerr_t cg_put_circle(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		     ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *x = NULL, *y = NULL, *radius = NULL, *line_width = NULL;
  char *line_color = NULL, *fill_color = NULL, *opacity = NULL;
  char *dash1 = NULL, *dash2 = NULL, *blendmode = NULL;
  yvalue_t v_x, v_y, v_r, v_l, v_d1, v_d2;
  yerr_t res = YENOERR;
  int gstate = -1;

  x = ydom_node_get_attr_value(node, "x");
  y = ydom_node_get_attr_value(node, "y");
  radius = ydom_node_get_attr_value(node, "radius");
  line_width = ydom_node_get_attr_value(node, "line-width");
  line_color = ydom_node_get_attr_value(node, "line-color");
  fill_color = ydom_node_get_attr_value(node, "fill-color");
  dash1 = ydom_node_get_attr_value(node, "dash1");
  dash2 = ydom_node_get_attr_value(node, "dash2");
  opacity = ydom_node_get_attr_value(node, "opacity");
  blendmode = ydom_node_get_attr_value(node, "blendmode");

  if (!x || !y || !radius || (!fill_color && !line_color))
    {
      YLOG_ADD(YLOG_ERR, "Empty circle declaration [line %d].", node->line_nbr);
      res = YEUNDEF;
    }
  else
    {
      cg_expr_var_t e_x = {0}, e_y = {0}, e_r = {0}, e_l = {0};
      cg_expr_var_t e_fc = {0}, e_lc = {0}, e_d1 = {0}, e_d2 = {0};

      if (cg_expr(carta, x, &e_x, carta->default_unit) != YENOERR ||
	  e_x.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('x' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, y, &e_y, carta->default_unit) != YENOERR ||
	       e_y.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('y' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, radius, &e_r, carta->default_unit) != YENOERR ||
	       e_r.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('radius' parameter) [line %d].",
		 node->line_nbr);
      else
	{
	  v_x = e_x.value.value;
	  v_y = e_y.value.value;
	  v_r = e_r.value.value;
	  v_x.value = YVAL2PT(v_x) + YVAL2PT(card_x);
	  v_y.value = YVAL2PT(v_y) + YVAL2PT(card_y);
	  v_x.unit = v_y.unit = YUNIT_PT;
	  PDF_save(carta->p);
	  if (opacity)
	    {
	      ystr_t ys = ys_new("");
	      ys_printf(&ys, "opacityfill %s opacitystroke %s", opacity, opacity);
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
	      if (cg_expr(carta, fill_color, &e_fc, carta->default_unit) != YENOERR ||
		  e_fc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('fill-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "fill", "rgb", e_fc.value.color.red,
			     e_fc.value.color.green, e_fc.value.color.blue, 0.0);
	    }
	  if (line_color)
	    {
	      if (cg_expr(carta, line_color, &e_lc, carta->default_unit) != YENOERR ||
		  e_lc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "stroke", "rgb", e_lc.value.color.red,
			     e_lc.value.color.green, e_lc.value.color.blue, 0.0);
	    }
	  if (line_width)
	    {
	      if (cg_expr(carta, line_width, &e_l, carta->default_unit) != YENOERR ||
		  e_l.type != CG_EXPR_VALUE)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-width' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  v_l = e_l.value.value;
		  PDF_setlinewidth(carta->p, YVAL2PT(v_l));
		}
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
	    }
	  PDF_circle(carta->p, YVAL2PT(v_x), YVAL2PT(v_y), YVAL2PT(v_r));
	  if (fill_color && line_color)
	    PDF_fill_stroke(carta->p);
	  else if (fill_color)
	    PDF_fill(carta->p);
	  else
	    PDF_stroke(carta->p);
	  PDF_restore(carta->p);
	}
      free_var(&e_x, &e_x);
      free_var(&e_y, &e_y);
      free_var(&e_r, &e_r);
      free_var(&e_l, &e_l);
      free_var(&e_fc, &e_fc);
      free_var(&e_lc, &e_lc);
      free_var(&e_d1, &e_d1);
      free_var(&e_d2, &e_d2);
    }
  free0(x);
  free0(y);
  free0(radius);
  free0(line_width);
  free0(line_color);
  free0(fill_color);
  free0(dash1);
  free0(dash2);
  free0(opacity);
  free0(blendmode);
  return (res);
}

/*
** cg_put_box()
** Put a box in PDF.
*/
yerr_t cg_put_box(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		  ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *x = NULL, *y = NULL, *width = NULL, *height = NULL, *radius = NULL;
  char *line_width = NULL, *line_color = NULL, *fill_color = NULL;
  char *rotation = NULL, *opacity = NULL, *blendmode = NULL;
  char *dash1 = NULL, *dash2 = NULL;
  yvalue_t v_x, v_y, v_w, v_h, v_r, v_l, v_d1, v_d2;
  yerr_t res = YENOERR;
  int gstate = -1;
  double tmp_x, tmp_y, degrees;
  ystr_t ys_rot;

  x = ydom_node_get_attr_value(node, "x");
  y = ydom_node_get_attr_value(node, "y");
  width = ydom_node_get_attr_value(node, "width");
  height = ydom_node_get_attr_value(node, "height");
  radius = ydom_node_get_attr_value(node, "radius");
  line_width = ydom_node_get_attr_value(node, "line-width");
  line_color = ydom_node_get_attr_value(node, "line-color");
  fill_color = ydom_node_get_attr_value(node, "fill-color");
  rotation = ydom_node_get_attr_value(node, "rotation");
  dash1 = ydom_node_get_attr_value(node, "dash1");
  dash2 = ydom_node_get_attr_value(node, "dash2");
  opacity = ydom_node_get_attr_value(node, "opacity");
  blendmode = ydom_node_get_attr_value(node, "blendmode");

  if (!x || !y || !width || !height || (!fill_color && !line_color))
    {
      YLOG_ADD(YLOG_WARN, "Empty box declaration [line %d].", node->line_nbr);
      res = YEUNDEF;
    }
  else
    {
      cg_expr_var_t e_x = {0}, e_y = {0}, e_w = {0}, e_h = {0};
      cg_expr_var_t e_r = {0}, e_l = {0}, e_d = {0}, e_fc = {0}, e_lc = {0};
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
	  v_x.unit = v_y.unit = YUNIT_PT;
	  PDF_save(carta->p);
	  if (opacity)
	    {
	      ystr_t ys = ys_new("");
	      ys_printf(&ys, "opacityfill %s opacitystroke %s", opacity, opacity);
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
	      if (cg_expr(carta, fill_color, &e_fc, carta->default_unit) != YENOERR ||
		  e_fc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('fill-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "fill", "rgb", e_fc.value.color.red,
			     e_fc.value.color.green, e_fc.value.color.blue, 0.0);
	    }
	  if (line_color)
	    {
	      if (cg_expr(carta, line_color, &e_lc, carta->default_unit) != YENOERR ||
		  e_lc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "stroke", "rgb", e_lc.value.color.red,
			     e_lc.value.color.green, e_lc.value.color.blue, 0.0);
	    }
	  if (line_width)
	    {
	      if (cg_expr(carta, line_width, &e_l, carta->default_unit) != YENOERR ||
		  e_l.type != CG_EXPR_VALUE)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-width' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  v_l = e_l.value.value;
		  PDF_setlinewidth(carta->p, YVAL2PT(v_l));
		}
	    }
	  if (rotation)
	    {
	      ys_rot = ys_new("");
	      ys_printf(&ys_rot, "(%s)", rotation);
	      if (cg_expr(carta, ys_rot, &e_d, carta->default_unit) != YENOERR ||
		  e_d.type != CG_EXPR_SCALAR)
		YLOG_ADD(YLOG_WARN, "Expression error ('rotation' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  degrees = e_d.value.scalar;
		  cg_rotate(&v_x, &v_y, degrees);
		  PDF_rotate(carta->p, degrees);
		}
	      ys_del(&ys_rot);
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
	    }
	  if (radius)
	    {
	      /* create a round-box */
	      tmp_x = YVAL2PT(v_x) + YVAL2PT(v_r);
	      tmp_y = YVAL2PT(v_y) + YVAL2PT(v_r);
	      PDF_arc(carta->p, tmp_x, tmp_y, YVAL2PT(v_r), 180.0, 270.0);
	      tmp_x = YVAL2PT(v_x) + YVAL2PT(v_w) - YVAL2PT(v_r);
	      tmp_y = YVAL2PT(v_y) + YVAL2PT(v_r);
	      PDF_arc(carta->p, tmp_x, tmp_y, YVAL2PT(v_r), 270.0, 0.0);
	      tmp_x = YVAL2PT(v_x) + YVAL2PT(v_w) - YVAL2PT(v_r);
	      tmp_y = YVAL2PT(v_y) + YVAL2PT(v_h) - YVAL2PT(v_r);
	      PDF_arc(carta->p, tmp_x, tmp_y, YVAL2PT(v_r), 0.0, 90.0);
	      tmp_x = YVAL2PT(v_x) + YVAL2PT(v_r);
	      tmp_y = YVAL2PT(v_y) + YVAL2PT(v_h) - YVAL2PT(v_r);
	      PDF_arc(carta->p, tmp_x, tmp_y, YVAL2PT(v_r), 90.0, 180.0);
	      PDF_closepath(carta->p);
	    }
	  else
	    {
	      /* create a straight box */
	      PDF_rect(carta->p, YVAL2PT(v_x), YVAL2PT(v_y), YVAL2PT(v_w),
		       YVAL2PT(v_h));
	    }
	  if (fill_color && line_color)
	    PDF_fill_stroke(carta->p);
	  else if (fill_color)
	    PDF_fill(carta->p);
	  else
	    PDF_stroke(carta->p);
	  PDF_restore(carta->p);
	}
      free_var(&e_x, &e_x);
      free_var(&e_y, &e_y);
      free_var(&e_w, &e_w);
      free_var(&e_h, &e_h);
      free_var(&e_l, &e_l);
      free_var(&e_d, &e_d);
      free_var(&e_fc, &e_fc);
      free_var(&e_lc, &e_lc);
      free_var(&e_d1, &e_d1);
      free_var(&e_d2, &e_d2);
    }
  free0(x);
  free0(y);
  free0(width);
  free0(height);
  free0(line_width);
  free0(line_color);
  free0(fill_color);
  free0(rotation);
  free0(dash1);
  free0(dash2);
  free0(opacity);
  free0(blendmode);
  return (res);
}

/*
** cg_put_grid()
** Put a grid in PDF.
*/
yerr_t cg_put_grid(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		   ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *x = NULL, *y = NULL, *cell_width = NULL, *cell_height = NULL;
  char *nbr_width = NULL, *nbr_height = NULL;
  char *line_width = NULL, *line_color = NULL, *fill_color = NULL;
  char *rotation = NULL, *opacity = NULL, *blendmode = NULL;
  char *dash1 = NULL, *dash2 = NULL;
  yvalue_t v_x, v_y, v_w, v_h, v_l, v_d1, v_d2;
  yerr_t res = YENOERR;
  int gstate = -1;
  double degrees;
  ystr_t ys_rot, ys_nbr_w, ys_nbr_h;

  x = ydom_node_get_attr_value(node, "x");
  y = ydom_node_get_attr_value(node, "y");
  cell_width = ydom_node_get_attr_value(node, "cell-width");
  cell_height = ydom_node_get_attr_value(node, "cell-height");
  nbr_width = ydom_node_get_attr_value(node, "nbr-width");
  nbr_height = ydom_node_get_attr_value(node, "nbr-height");
  line_width = ydom_node_get_attr_value(node, "line-width");
  line_color = ydom_node_get_attr_value(node, "line-color");
  fill_color = ydom_node_get_attr_value(node, "fill-color");
  rotation = ydom_node_get_attr_value(node, "rotation");
  dash1 = ydom_node_get_attr_value(node, "dash1");
  dash2 = ydom_node_get_attr_value(node, "dash2");
  opacity = ydom_node_get_attr_value(node, "opacity");
  blendmode = ydom_node_get_attr_value(node, "blendmode");

  ys_nbr_w = ys_new("");
  ys_nbr_h = ys_new("");
  if (!x || !y || !cell_width || !cell_height ||
      !nbr_width || !nbr_height || (!fill_color && !line_color))
    {
      YLOG_ADD(YLOG_WARN, "Empty grid declaration [line %d].", node->line_nbr);
      res = YEUNDEF;
    }
  else
    {
      cg_expr_var_t e_x = {0}, e_y = {0}, e_w = {0}, e_h = {0};
      cg_expr_var_t e_l = {0}, e_d = {0}, e_fc = {0}, e_lc = {0};
      cg_expr_var_t e_d1 = {0}, e_d2 = {0}, e_nw = {0}, e_nh = {0};

      if (cg_expr(carta, x, &e_x, carta->default_unit) != YENOERR ||
	  e_x.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('x' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, y, &e_y, carta->default_unit) != YENOERR ||
	       e_y.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('y' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, cell_width, &e_w, carta->default_unit) != YENOERR ||
	       e_w.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('cell-width' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, cell_height, &e_h, carta->default_unit) != YENOERR ||
	       e_w.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('cell-height' parameter) [line %d].",
		 node->line_nbr);
      else if (nbr_width && strlen(nbr_width) && ys_printf(&ys_nbr_w, "(%s)", nbr_width) &&
	       (cg_expr(carta, ys_nbr_w, &e_nw, carta->default_unit) != YENOERR ||
		e_nw.type != CG_EXPR_SCALAR))
	YLOG_ADD(YLOG_WARN, "Expression error ('nbr-width' parameter) [line %d].",
		 node->line_nbr);
      else if (nbr_height && strlen(nbr_height) && ys_printf(&ys_nbr_h, "(%s)", nbr_height) &&
	       (cg_expr(carta, ys_nbr_h, &e_nh, carta->default_unit) != YENOERR ||
		e_nh.type != CG_EXPR_SCALAR))
	YLOG_ADD(YLOG_WARN, "Expression error ('nbr-height' parameter) [line %d].",
		 node->line_nbr);
      else
	{
	  v_x = e_x.value.value;
	  v_y = e_y.value.value;
	  v_w = e_w.value.value;
	  v_h = e_h.value.value;
	  v_x.value = YVAL2PT(v_x) + YVAL2PT(card_x);
	  v_y.value = YVAL2PT(v_y) + YVAL2PT(card_y);
	  v_x.unit = v_y.unit = YUNIT_PT;
	  PDF_save(carta->p);
	  if (opacity)
	    {
	      ystr_t ys = ys_new("");
	      ys_printf(&ys, "opacityfill %s opacitystroke %s", opacity, opacity);
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
	      if (cg_expr(carta, fill_color, &e_fc, carta->default_unit) != YENOERR ||
		  e_fc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('fill-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "fill", "rgb", e_fc.value.color.red,
			     e_fc.value.color.green, e_fc.value.color.blue, 0.0);
	    }
	  if (line_color)
	    {
	      if (cg_expr(carta, line_color, &e_lc, carta->default_unit) != YENOERR ||
		  e_lc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "stroke", "rgb", e_lc.value.color.red,
			     e_lc.value.color.green, e_lc.value.color.blue, 0.0);
	    }
	  if (line_width)
	    {
	      if (cg_expr(carta, line_width, &e_l, carta->default_unit) != YENOERR ||
		  e_l.type != CG_EXPR_VALUE)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-width' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  v_l = e_l.value.value;
		  PDF_setlinewidth(carta->p, YVAL2PT(v_l));
		}
	    }
	  if (rotation)
	    {
	      ys_rot = ys_new("");
	      ys_printf(&ys_rot, "(%s)", rotation);
	      if (cg_expr(carta, ys_rot, &e_d, carta->default_unit) != YENOERR ||
		  e_d.type != CG_EXPR_SCALAR)
		YLOG_ADD(YLOG_WARN, "Expression error ('rotation' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  degrees = e_d.value.scalar;
		  cg_rotate(&v_x, &v_y, degrees);
		  PDF_rotate(carta->p, degrees);
		}
	      ys_del(&ys_rot);
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
	    }
	  /* create a grid */
	  {
	    double nb;
	    PDF_rect(carta->p, YVAL2PT(v_x), YVAL2PT(v_y),
		     YVAL2PT(v_w) * e_nw.value.scalar,
		     YVAL2PT(v_h) * e_nh.value.scalar);
	    for (nb = 1.0; nb < e_nw.value.scalar; nb += 1.0)
	      {
		PDF_moveto(carta->p, YVAL2PT(v_x) + (YVAL2PT(v_w) * nb),
			   YVAL2PT(v_y));
		PDF_lineto(carta->p, YVAL2PT(v_x) + (YVAL2PT(v_w) * nb),
			   YVAL2PT(v_y) + (YVAL2PT(v_h) * e_nh.value.scalar));
	      }
	    for (nb = 1.0; nb < e_nh.value.scalar; nb += 1.0)
	      {
		PDF_moveto(carta->p, YVAL2PT(v_x),
			   YVAL2PT(v_y) + (YVAL2PT(v_h) * nb));
		PDF_lineto(carta->p, YVAL2PT(v_x) +
			   (YVAL2PT(v_w) * e_nw.value.scalar),
			   YVAL2PT(v_y) + (YVAL2PT(v_h) * nb));
	      }
	  }
	  if (fill_color && line_color)
	    PDF_fill_stroke(carta->p);
	  else if (fill_color)
	    PDF_fill(carta->p);
	  else
	    PDF_stroke(carta->p);
	  PDF_restore(carta->p);
	}
      free_var(&e_x, &e_x);
      free_var(&e_y, &e_y);
      free_var(&e_w, &e_w);
      free_var(&e_l, &e_l);
      free_var(&e_d, &e_d);
      free_var(&e_fc, &e_fc);
      free_var(&e_lc, &e_lc);
      free_var(&e_d1, &e_d1);
      free_var(&e_d2, &e_d2);
    }
  ys_del(&ys_nbr_w);
  ys_del(&ys_nbr_h);
  free0(x);
  free0(y);
  free0(cell_width);
  free0(cell_height);
  free0(nbr_width);
  free0(nbr_height);
  free0(line_width);
  free0(line_color);
  free0(fill_color);
  free0(rotation);
  free0(dash1);
  free0(dash2);
  free0(opacity);
  free0(blendmode);
  return (res);
}

/*
** cg_put_hexagon()
** Put an hexagon in PDF.
*/
yerr_t cg_put_hexagon(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		      ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *x = NULL, *y = NULL, *width = NULL;
  char *line_width = NULL, *line_color = NULL, *fill_color = NULL;
  char *rotation = NULL, *opacity = NULL, *blendmode = NULL;
  char *dash1 = NULL, *dash2 = NULL;
  yvalue_t v_x, v_y, v_w, v_l, v_d1, v_d2;
  yerr_t res = YENOERR;
  int gstate = -1;
  double degrees;
  ystr_t ys_rot;

  x = ydom_node_get_attr_value(node, "x");
  y = ydom_node_get_attr_value(node, "y");
  width = ydom_node_get_attr_value(node, "width");
  line_width = ydom_node_get_attr_value(node, "line-width");
  line_color = ydom_node_get_attr_value(node, "line-color");
  fill_color = ydom_node_get_attr_value(node, "fill-color");
  rotation = ydom_node_get_attr_value(node, "rotation");
  dash1 = ydom_node_get_attr_value(node, "dash1");
  dash2 = ydom_node_get_attr_value(node, "dash2");
  opacity = ydom_node_get_attr_value(node, "opacity");
  blendmode = ydom_node_get_attr_value(node, "blendmode");

  if (!x || !y || !width || (!fill_color && !line_color))
    {
      YLOG_ADD(YLOG_WARN, "Empty hexagon declaration [line %d].", node->line_nbr);
      res = YEUNDEF;
    }
  else
    {
      cg_expr_var_t e_x = {0}, e_y = {0}, e_w = {0};
      cg_expr_var_t e_l = {0}, e_d = {0}, e_fc = {0}, e_lc = {0};
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
      else
	{
	  v_x = e_x.value.value;
	  v_y = e_y.value.value;
	  v_w = e_w.value.value;
	  v_x.value = YVAL2PT(v_x) + YVAL2PT(card_x);
	  v_y.value = YVAL2PT(v_y) + YVAL2PT(card_y);
	  v_x.unit = v_y.unit = YUNIT_PT;
	  PDF_save(carta->p);
	  if (opacity)
	    {
	      ystr_t ys = ys_new("");
	      ys_printf(&ys, "opacityfill %s opacitystroke %s", opacity, opacity);
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
	      if (cg_expr(carta, fill_color, &e_fc, carta->default_unit) != YENOERR ||
		  e_fc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('fill-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "fill", "rgb", e_fc.value.color.red,
			     e_fc.value.color.green, e_fc.value.color.blue, 0.0);
	    }
	  if (line_color)
	    {
	      if (cg_expr(carta, line_color, &e_lc, carta->default_unit) != YENOERR ||
		  e_lc.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_setcolor(carta->p, "stroke", "rgb", e_lc.value.color.red,
			     e_lc.value.color.green, e_lc.value.color.blue, 0.0);
	    }
	  if (line_width)
	    {
	      if (cg_expr(carta, line_width, &e_l, carta->default_unit) != YENOERR ||
		  e_l.type != CG_EXPR_VALUE)
		YLOG_ADD(YLOG_WARN, "Expression error ('line-width' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  v_l = e_l.value.value;
		  PDF_setlinewidth(carta->p, YVAL2PT(v_l));
		}
	    }
	  if (rotation)
	    {
	      ys_rot = ys_new("");
	      ys_printf(&ys_rot, "(%s)", rotation);
	      if (cg_expr(carta, ys_rot, &e_d, carta->default_unit) != YENOERR ||
		  e_d.type != CG_EXPR_SCALAR)
		YLOG_ADD(YLOG_WARN, "Expression error ('rotation' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  degrees = e_d.value.scalar;
		  cg_rotate(&v_x, &v_y, degrees);
		  PDF_rotate(carta->p, degrees);
		}
	      ys_del(&ys_rot);
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
	    }
	  /* create an hexagon */
	  {
	    yvalue_t delta, epsilon, v_h;
	    delta.unit = epsilon.unit = v_h.unit = YUNIT_PT;
	    delta.value = YVAL2PT(v_w) / 2;
	    epsilon.value = delta.value * tan(30 * M_PI / 180);
	    v_h.value = epsilon.value * 4;
	    PDF_moveto(carta->p, YVAL2PT(v_x) + YVAL2PT(delta), YVAL2PT(v_y));
	    PDF_lineto(carta->p, YVAL2PT(v_x), YVAL2PT(v_y) + YVAL2PT(epsilon));
	    PDF_lineto(carta->p, YVAL2PT(v_x), YVAL2PT(v_y) + YVAL2PT(v_h) -
		       YVAL2PT(epsilon));
	    PDF_lineto(carta->p, YVAL2PT(v_x) + YVAL2PT(delta),
		       YVAL2PT(v_y) + YVAL2PT(v_h));
	    PDF_lineto(carta->p, YVAL2PT(v_x) + YVAL2PT(v_w),
		       YVAL2PT(v_y) + YVAL2PT(v_h) - YVAL2PT(epsilon));
	    PDF_lineto(carta->p, YVAL2PT(v_x) + YVAL2PT(v_w),
		       YVAL2PT(v_y) + YVAL2PT(epsilon));
	    PDF_closepath(carta->p);
	  }
	  if (fill_color && line_color)
	    PDF_fill_stroke(carta->p);
	  else if (fill_color)
	    PDF_fill(carta->p);
	  else
	    PDF_stroke(carta->p);
	  PDF_restore(carta->p);
	}
      free_var(&e_x, &e_x);
      free_var(&e_y, &e_y);
      free_var(&e_w, &e_w);
      free_var(&e_l, &e_l);
      free_var(&e_d, &e_d);
      free_var(&e_fc, &e_fc);
      free_var(&e_lc, &e_lc);
      free_var(&e_d1, &e_d1);
      free_var(&e_d2, &e_d2);
    }
  free0(x);
  free0(y);
  free0(width);
  free0(line_width);
  free0(line_color);
  free0(fill_color);
  free0(rotation);
  free0(dash1);
  free0(dash2);
  free0(opacity);
  free0(blendmode);
  return (res);
}

/*
** cg_put_text()
** Put text in PDF.
*/
yerr_t cg_put_text(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		   ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *font = NULL, *size = NULL, *color = NULL, *adapt = NULL;
  char *x = NULL, *y = NULL, *width = NULL, *height = NULL, *radius = NULL;
  char *rotation = NULL, *align = NULL, *text = NULL, *text2 = NULL;
  char *space = NULL, *opacity = NULL, *blendmode = NULL;
  char *fill_color = NULL, *line_color = NULL, *line_width = NULL;
  char *border = NULL, *underline = NULL, *overline = NULL;
  char *strikeout = NULL, *char_space = NULL, *h_scale = NULL;
  char *dash1 = NULL, *dash2 = NULL, *ladder_color = NULL;
  char *ladder_rise = NULL, *ladder_width = NULL, *ladder_bottom = NULL;
  double tmp_x, tmp_y, degrees, f_size, sp;
  yvalue_t v_x, v_y, v_w, v_h, v_r, v_l, v_x2, v_y2, v_d1, v_d2, v_lar, v_law;
  yerr_t res = YENOERR;
  int font_handle = -1, i, gstate;
  ydom_node_t *subnode;
  ystr_t ys_space, ys_rot;

  font = ydom_node_get_attr_value(node, "font");
  size = ydom_node_get_attr_value(node, "size");
  color = ydom_node_get_attr_value(node, "color");
  adapt = ydom_node_get_attr_value(node, "adapt");
  x = ydom_node_get_attr_value(node, "x");
  y = ydom_node_get_attr_value(node, "y");
  width = ydom_node_get_attr_value(node, "width");
  height = ydom_node_get_attr_value(node, "height");
  radius = ydom_node_get_attr_value(node, "radius");
  space = ydom_node_get_attr_value(node, "space");
  rotation = ydom_node_get_attr_value(node, "rotation");
  align = ydom_node_get_attr_value(node, "align");
  opacity = ydom_node_get_attr_value(node, "opacity");
  blendmode = ydom_node_get_attr_value(node, "blendmode");
  fill_color = ydom_node_get_attr_value(node, "fill-color");
  line_color = ydom_node_get_attr_value(node, "line-color");
  line_width = ydom_node_get_attr_value(node, "line-width");
  border = ydom_node_get_attr_value(node, "border");
  underline = ydom_node_get_attr_value(node, "underline");
  overline = ydom_node_get_attr_value(node, "overline");
  strikeout = ydom_node_get_attr_value(node, "strikeout");
  char_space = ydom_node_get_attr_value(node, "char-space");
  h_scale = ydom_node_get_attr_value(node, "h-scale");
  dash1 = ydom_node_get_attr_value(node, "dash1");
  dash2 = ydom_node_get_attr_value(node, "dash2");
  ladder_color = ydom_node_get_attr_value(node, "ladder-color");
  ladder_rise = ydom_node_get_attr_value(node, "ladder-rise");
  ladder_width = ydom_node_get_attr_value(node, "ladder-width");
  ladder_bottom = ydom_node_get_attr_value(node, "ladder-bottom");

  text = ys_new("");
  ys_space = ys_new("");
  for (subnode = ydom_node_get_first_child(node); subnode;
       subnode = ydom_node_get_next(subnode))
    {
      if (ydom_node_is_text(subnode))
	{
	  char *string = ydom_node_get_value(subnode);
	  ys_cat(&text, string);
	  free0(string);
	}
      else if (ydom_node_is_element(subnode) && subnode->name)
	{
	  if (!strcasecmp(subnode->name, "br"))
	    ys_cat(&text, "\n");
	  else if (!strcasecmp(subnode->name, "p"))
	    ys_cat(&text, "\n\n");
	  else if (!strcasecmp(subnode->name, "value"))
	    {
	      ydom_node_t *subnode2;
	      char *precision = ydom_node_get_attr_value(subnode, "precision");
	      for (subnode2 = ydom_node_get_first_child(subnode); subnode2;
		   subnode2 = ydom_node_get_next(subnode2))
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
		  if (res_val == YENOERR)
		    ys_cat(&text, var2.value.string);
		  free_var(&var2, &var2);
		  carta->expr->precision = old_precision;
		}
	      free0(precision);
	    }
	}
    }
  text2 = ys_subs(text, "&nbsp;", " ");
  ys_del(&text);

  if (!size || !color || !x || !y || !width || !height)
    {
      YLOG_ADD(YLOG_WARN, "Empty text declaration [line %d].",
	       node->line_nbr);
      res = YEUNDEF;
    }
  else if ((font_handle = cg_get_font(carta, font)) != -1)
    {
      cg_expr_var_t e_x = {0}, e_y = {0}, e_w = {0}, e_h = {0};
      cg_expr_var_t e_r = {0}, e_s = {0}, e_sp = {0}, e_l = {0};
      cg_expr_var_t e_d = {0}, e_c = {0}, e_lc = {0}, e_fc = {0};
      cg_expr_var_t e_d1 = {0}, e_d2 = {0}, e_lac = {0}, e_lar = {0};
      cg_expr_var_t e_law = {0};

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
      else if (cg_expr(carta, size, &e_s, YUNIT_PT) != YENOERR ||
	       e_s.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('size' parameter) [line %d].",
		 node->line_nbr);
      else if (space && strlen(space) && ys_printf(&ys_space, "(%s)", space) &&
	       (cg_expr(carta, ys_space, &e_sp, carta->default_unit) != YENOERR ||
		e_sp.type != CG_EXPR_SCALAR))
	YLOG_ADD(YLOG_WARN, "Expression error ('space' parameter) [line %d].",
		 node->line_nbr);
      else
	{
	  if (radius &&
	      (cg_expr(carta, radius, &e_r, carta->default_unit) != YENOERR ||
	       e_r.type != CG_EXPR_VALUE))
	    {
	      YLOG_ADD(YLOG_WARN, "Expression error ('radius' parameter) [line %d].",
		       node->line_nbr);
	      free0(radius);
	    }
	  align = (!align) ? strdup("left") : align;
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
	  f_size = YVAL2PT(e_s.value.value);
	  sp = (space && ys_len(ys_space)) ? e_sp.value.scalar : 1;

	  /* put frame */
	  if (fill_color || line_color)
	    {
	      v_x2 = v_x;
	      v_y2 = v_y;
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
		  if (cg_expr(carta, fill_color, &e_fc,
			      carta->default_unit) != YENOERR ||
		      e_fc.type != CG_EXPR_COLOR)
		    YLOG_ADD(YLOG_WARN, "Expression error ('fill-color' parameter) [line %d].",
			     node->line_nbr);
		  else
		    PDF_setcolor(carta->p, "fill", "rgb", e_fc.value.color.red,
				 e_fc.value.color.green, e_fc.value.color.blue, 0.0);
		}
	      if (line_color)
		{
		  if (cg_expr(carta, line_color, &e_lc,
			      carta->default_unit) != YENOERR ||
		      e_lc.type != CG_EXPR_COLOR)
		    YLOG_ADD(YLOG_WARN, "Expression error ('line-color' parameter) [line %d].",
			     node->line_nbr);
		  else
		    PDF_setcolor(carta->p, "stroke", "rgb", e_lc.value.color.red,
				 e_lc.value.color.green, e_lc.value.color.blue, 0.0);
		}
	      if (line_width)
		{
		  if (cg_expr(carta, line_width, &e_l, carta->default_unit) != YENOERR ||
		      e_l.type != CG_EXPR_VALUE)
		    YLOG_ADD(YLOG_WARN, "Expression error ('line-width' parameter) [line %d].",
			     node->line_nbr);
		  else
		    {
		      v_l = e_l.value.value;
		      PDF_setlinewidth(carta->p, YVAL2PT(v_l));
		    }
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
		}
	      if (rotation)
		{
		  ys_rot = ys_new("");
		  ys_printf(&ys_rot, "(%s)", rotation);
		  if (cg_expr(carta, ys_rot, &e_d, carta->default_unit) != YENOERR ||
		      e_d.type != CG_EXPR_SCALAR)
		    YLOG_ADD(YLOG_WARN, "Expression error ('rotation' parameter) [line %d].",
			     node->line_nbr);
		  else
		    {
		      degrees = e_d.value.scalar;
		      cg_rotate(&v_x2, &v_y2, degrees);
		      PDF_rotate(carta->p, degrees);
		    }
		  ys_del(&ys_rot);
		}
	      if (radius)
		{
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
	  /* modifiers (on text and ladder) */
	  if (border)
	    {
	      if (cg_expr(carta, border, &e_l, carta->default_unit) != YENOERR ||
		  e_l.type != CG_EXPR_VALUE)
		{
		  YLOG_ADD(YLOG_WARN, "Expression error ('border' parameter) [line %d].",
			   node->line_nbr);
		  memset(&v_l, 0, sizeof(v_l));
		}
	      else
		{
		  v_l = e_l.value.value;
		  v_w.value -= 2 * YVAL2PT(v_l);
		  v_h.value -= 2 * YVAL2PT(v_l);
		}
	    }
	  if (char_space)
	    {
	      cg_expr_var_t e_cs = {0};
	      if (cg_expr(carta, char_space, &e_cs, YUNIT_PT) != YENOERR ||
		  e_cs.type != CG_EXPR_VALUE)
		YLOG_ADD(YLOG_WARN, "Expression error ('char-space' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_set_value(carta->p, "charspacing", YVAL2PT(e_cs.value.value));
	      free_var(&e_cs, &e_cs);
	    }
	  if (h_scale)
	    {
	      cg_expr_var_t e_hs = {0};
	      ystr_t hs_ys = ys_new("");
	      ys_printf(&hs_ys, "(%s)", h_scale);
	      if (cg_expr(carta, hs_ys, &e_hs, carta->default_unit) != YENOERR ||
		  e_hs.type != CG_EXPR_SCALAR)
		YLOG_ADD(YLOG_WARN, "Expression error ('h-scale' parameter) [line %d].",
			 node->line_nbr);
	      else
		PDF_set_value(carta->p, "horizscaling", e_hs.value.scalar);
	      free_var(&e_hs, &e_hs);
	      ys_del(&hs_ys);
	    }
	  if (adapt && (!strcasecmp(adapt, "yes") || !strcasecmp(adapt, "true")))
	    {
	      PDF_setfont(carta->p, font_handle, f_size);
	      PDF_set_value(carta->p, "leading", f_size * sp);
	      while (f_size > 2.0 &&
		     PDF_show_boxed(carta->p, text2, YVAL2PT(v_x), YVAL2PT(v_y),
				    YVAL2PT(v_w), YVAL2PT(v_h), align, "blind"))
		{
		  f_size -= 2.0;
		  PDF_setfont(carta->p, font_handle, f_size);
		  PDF_set_value(carta->p, "leading", f_size * sp);
		}
	    }
	  /* put ladder */
	  if (ladder_color)
	    {
	      float descender, line_y;

	      if (cg_expr(carta, ladder_color, &e_lac,
			  carta->default_unit) != YENOERR ||
		  e_lac.type != CG_EXPR_COLOR)
		YLOG_ADD(YLOG_WARN, "Expression error ('ladder-color' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  PDF_setcolor(carta->p, "stroke", "rgb", e_lac.value.color.red,
			       e_lac.value.color.green, e_lac.value.color.blue, 0.0);
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
		  if (ladder_width)
		    {
		      if (cg_expr(carta, ladder_width, &e_law, carta->default_unit) != YENOERR ||
			  e_law.type != CG_EXPR_VALUE)
			YLOG_ADD(YLOG_WARN, "Expression error ('ladder-width' parameter) [line %d].",
				 node->line_nbr);
		      else
			{
			  v_law = e_law.value.value;
			  PDF_setlinewidth(carta->p, YVAL2PT(v_law));
			}
		    }
		  if (ladder_rise)
		    {
		      if (cg_expr(carta, ladder_rise, &e_lar, carta->default_unit) != YENOERR ||
			  e_lar.type != CG_EXPR_VALUE)
			YLOG_ADD(YLOG_WARN, "Expression error ('ladder-rise' parameter) [line %d].",
				 node->line_nbr);
		      else
			v_lar = e_lar.value.value;
		    }
		  v_x2 = v_x;
		  v_y2 = v_y;
		  if (rotation)
		    {
		      ys_rot = ys_new("");
		      ys_printf(&ys_rot, "(%s)", rotation);
		      if (cg_expr(carta, ys_rot, &e_d, carta->default_unit) != YENOERR ||
			  e_d.type != CG_EXPR_SCALAR)
			YLOG_ADD(YLOG_WARN, "Expression error ('rotation' parameter) [line %d].",
				 node->line_nbr);
		      else
			{
			  degrees = e_d.value.scalar;
			  cg_rotate(&v_x2, &v_y2, degrees);
			  PDF_rotate(carta->p, degrees);
			}
		      ys_del(&ys_rot);
		    }
		  /* write lines */
		  descender = PDF_get_value(carta->p, "descender", font_handle) * f_size;
		  line_y = YVAL2PT(v_y2) + YVAL2PT(v_h) - (f_size * sp);
		  if (ladder_rise && e_lar.type == CG_EXPR_VALUE)
		    line_y += YVAL2PT(v_lar);
		  if (ladder_bottom && (!strcasecmp(ladder_bottom, "yes") ||
					!strcasecmp(ladder_bottom, "true")))
		      line_y += descender;
		  for (; line_y > YVAL2PT(v_y2); line_y -= (f_size * sp))
		    {
		      PDF_moveto(carta->p, YVAL2PT(v_x2), line_y);
		      PDF_lineto(carta->p, YVAL2PT(v_x2) + YVAL2PT(v_w), line_y);
		      PDF_stroke(carta->p);
		    }
		  PDF_restore(carta->p);
		}
	    }
	  /* put text */
	  PDF_save(carta->p);
	  if (opacity)
	    {
	      ystr_t ys = ys_new("");
	      ys_printf(&ys, "opacityfill %s opacitystroke %s", opacity, opacity);
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
	  if (underline && (!strcasecmp(underline, "yes") || !strcasecmp(underline, "true")))
	    PDF_set_parameter(carta->p, "underline", "true");
	  if (overline && (!strcasecmp(overline, "yes") || !strcasecmp(overline, "true")))
	    PDF_set_parameter(carta->p, "overline", "true");
	  if (strikeout && (!strcasecmp(strikeout, "yes") || !strcasecmp(strikeout, "true")))
	    PDF_set_parameter(carta->p, "strikeout", "true");
	  if (cg_expr(carta, color, &e_c, carta->default_unit) != YENOERR ||
	      e_c.type != CG_EXPR_COLOR)
	    YLOG_ADD(YLOG_WARN, "Expression error ('color' parameter) [line %d].",
		     node->line_nbr);
	  else
	    PDF_setcolor(carta->p, "fillstroke", "rgb", e_c.value.color.red,
			 e_c.value.color.green, e_c.value.color.blue, 0.0);
	  PDF_setfont(carta->p, font_handle, f_size);
	  PDF_set_value(carta->p, "leading", f_size * sp);
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
	      v_x.value += YVAL2PT(v_l);
	      v_y.value += YVAL2PT(v_l);
	    }
	  if ((i = PDF_show_boxed(carta->p, text2, YVAL2PT(v_x), YVAL2PT(v_y),
				  YVAL2PT(v_w), YVAL2PT(v_h), align, "")) > 0)
	    YLOG_ADD(YLOG_INFO, "There is %d characters out of the text box [line %d].",
		     i, node->line_nbr);
	  if (underline && (!strcasecmp(underline, "yes") || !strcasecmp(underline, "true")))
	    PDF_set_parameter(carta->p, "underline", "false");
	  if (overline && (!strcasecmp(overline, "yes") || !strcasecmp(overline, "true")))
	    PDF_set_parameter(carta->p, "overline", "false");
	  if (strikeout && (!strcasecmp(strikeout, "yes") || !strcasecmp(strikeout, "true")))
	    PDF_set_parameter(carta->p, "strikeout", "false");
	  PDF_restore(carta->p);
	  PDF_set_value(carta->p, "charspacing", 0.0);
	  PDF_set_value(carta->p, "wordspacing", 0.0);
	  PDF_set_value(carta->p, "horizscaling", 100);
	}
      free_var(&e_x, &e_x);
      free_var(&e_y, &e_y);
      free_var(&e_w, &e_w);
      free_var(&e_h, &e_h);
      free_var(&e_s, &e_s);
      free_var(&e_sp, &e_sp);
      free_var(&e_l, &e_l);
      free_var(&e_d, &e_d);
      free_var(&e_c, &e_c);
      free_var(&e_fc, &e_fc);
      free_var(&e_lc, &e_lc);
      free_var(&e_d1, &e_d1);
      free_var(&e_d2, &e_d2);
    }
  else
    YLOG_ADD(YLOG_WARN, "Unable to find font [line %d].", node->line_nbr);
  free0(font);
  free0(size);
  free0(color);
  free0(adapt);
  free0(x);
  free0(y);
  free0(width);
  free0(height);
  free0(space);
  free0(rotation);
  free0(align);
  free0(opacity);
  free0(blendmode);
  free0(fill_color);
  free0(line_color);
  free0(line_width);
  free0(border);
  free0(underline);
  free0(overline);
  free0(strikeout);
  free0(char_space);
  free0(h_scale);
  ys_del(&text2);
  ys_del(&ys_space);
  free0(dash1);
  free0(dash2);
  return (res);
}

/*
** cg_rotate()
** Convert coordinates depending of rotation.
*/
yerr_t cg_rotate(yvalue_t *x, yvalue_t *y, double angle)
{
  double x_pt, y_pt;

  angle = angle * M_PI / 180;
  x_pt = YVAL2PT(*x);
  y_pt = YVAL2PT(*y);
  x->value = (((double)cos(angle)) * x_pt) + (((double)sin(angle)) * y_pt);
  y->value = (((double)cos(angle)) * y_pt) - (((double)sin(angle)) * x_pt);
  x->unit = y->unit = YUNIT_PT;
  return (YENOERR);
}

/*
** cg_get_font()
** Retreive a font from the declared fonts list.
*/
int cg_get_font(cg_t *carta, const char *fontname)
{
  int i;
  cg_font_t *this_font;

  if (!fontname && !carta->default_font)
    {
      YLOG_ADD(YLOG_WARN, "No font name.");
      return (-1);
    }
  if (!fontname)
    fontname = carta->default_font;
  for (i = 0; i < yv_len(carta->fonts); ++i)
    {
      this_font = carta->fonts[i];
      if (!strcmp(this_font->id, fontname))
	return (this_font->f);
    }
  if (!strcmp(fontname, "Courier") ||
      !strcmp(fontname, "Courier-Bold") ||
      !strcmp(fontname, "Courier-Oblique") ||
      !strcmp(fontname, "Courier-BoldOblique") ||
      !strcmp(fontname, "Helvetica") ||
      !strcmp(fontname, "Helvetica-Bold") ||
      !strcmp(fontname, "Helvetica-Oblique") ||
      !strcmp(fontname, "Helvetica-BoldOblique") ||
      !strcmp(fontname, "Times-Roman") ||
      !strcmp(fontname, "Times-Bold") ||
      !strcmp(fontname, "Times-Italic") ||
      !strcmp(fontname, "Times-BoldItalic") ||
      !strcmp(fontname, "Symbol") ||
      !strcmp(fontname, "ZapfDingbats"))
    {
      this_font = malloc0(sizeof(cg_font_t));
      this_font->f = PDF_load_font(carta->p, fontname, 0, "host", "");
      /* this_font->f = PDF_load_font(carta->p, fontname, 0, "iso8859-15", ""); */
      this_font->id = strdup(fontname);
      yv_add(&carta->fonts, this_font);
      return (this_font->f);
    }
  YLOG_ADD(YLOG_WARN, "Unable to find the font '%s'.", fontname);
  return (-1);
}

/*
** cg_get_image()
** Retreive an image from the declared images list.
*/
cg_image_t *cg_get_image(cg_t *carta, const char *id)
{
  int i;
  cg_image_t *this_image;

  if (!id)
    return (NULL);
  for (i = 0; i < yv_len(carta->images); ++i)
    {
      this_image = carta->images[i];
      if (!strcmp(this_image->id, id))
	return (this_image);
    }
  return (NULL);
}

/*
** cg_put_image()
** Put an image in PDF.
*/
yerr_t cg_put_image(cg_t *carta, cg_deck_t *deck, ydom_node_t *card,
		   ydom_node_t *node, yvalue_t card_x, yvalue_t card_y)
{
  char *id = NULL, *x = NULL, *y = NULL, *width = NULL, *height = NULL;
  char *rotation = NULL, *file = NULL, *mask = NULL, *entire = NULL;
  char *mask_id = NULL;
  yvalue_t v_x, v_y, v_w, v_h;
  yerr_t res = YENOERR;
  cg_image_t *image_handle;
  ystr_t ys;
  double degrees;
  ystr_t ys_rot;

  id = ydom_node_get_attr_value(node, "id");
  x = ydom_node_get_attr_value(node, "x");
  y = ydom_node_get_attr_value(node, "y");
  width = ydom_node_get_attr_value(node, "width");
  height = ydom_node_get_attr_value(node, "height");
  rotation = ydom_node_get_attr_value(node, "rotation");
  file = ydom_node_get_attr_value(node, "file");
  mask = ydom_node_get_attr_value(node, "mask");
  mask_id = ydom_node_get_attr_value(node, "mask-id");
  entire = ydom_node_get_attr_value(node, "entire");

  if (file && width && height)
    {
      image_handle = malloc0(sizeof(cg_image_t));
      image_handle->file = file;
      image_handle->mask = mask;
      image_handle->mask_id = mask_id;
      image_handle->width = yvalue_read(width, carta->default_unit);
      image_handle->height = yvalue_read(height, carta->default_unit);
      image_handle->i = image_handle->m = -1;
      cg_load_image(carta, image_handle, YFALSE);
    }
  else
    image_handle = cg_get_image(carta, id);
  ys = ys_new("");

  if ((!id && (!file || !width || !height)) || !x || !y || !image_handle || !ys)
    {
      YLOG_ADD(YLOG_WARN, "Empty image declaration [line %d].", node->line_nbr);
      res = YEUNDEF;
    }
  else
    {
      cg_expr_var_t e_x = {0}, e_y = {0}, e_w = {0}, e_h = {0}, e_d = {0};

      if (cg_expr(carta, x, &e_x, carta->default_unit) != YENOERR ||
	  e_x.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('x' parameter) [line %d].",
		 node->line_nbr);
      else if (cg_expr(carta, y, &e_y, carta->default_unit) != YENOERR ||
	       e_y.type != CG_EXPR_VALUE)
	YLOG_ADD(YLOG_WARN, "Expression error ('y' parameter) [line %d].",
		 node->line_nbr);
      else if (width && (cg_expr(carta, width, &e_w, carta->default_unit) != YENOERR ||
			 e_w.type != CG_EXPR_VALUE))
	YLOG_ADD(YLOG_WARN, "Expression error ('width' parameter) [line %d].",
		 node->line_nbr);
      else if (height && (cg_expr(carta, height, &e_h, carta->default_unit) != YENOERR ||
			  e_h.type != CG_EXPR_VALUE))
	YLOG_ADD(YLOG_WARN, "Expression error ('height' parameter) [line %d].",
		 node->line_nbr);
      else
	{
	  char *fitmethod = NULL;
	  v_x = e_x.value.value;
	  v_y = e_y.value.value;
	  v_w = width ? e_w.value.value : image_handle->width;
	  v_h = height ? e_h.value.value : image_handle->height;
	  v_x.value = YVAL2PT(v_x) + YVAL2PT(card_x);
	  v_y.value = YVAL2PT(v_y) + YVAL2PT(card_y);
	  v_x.unit = v_y.unit = YUNIT_PT;
	  PDF_save(carta->p);
	  if (rotation)
	    {
	      ys_rot = ys_new("");
	      ys_printf(&ys_rot, "(%s)", rotation);
	      if (cg_expr(carta, ys_rot, &e_d, carta->default_unit) != YENOERR ||
		  e_d.type != CG_EXPR_SCALAR)
		YLOG_ADD(YLOG_WARN, "Expression error ('rotation' parameter) [line %d].",
			 node->line_nbr);
	      else
		{
		  degrees = e_d.value.scalar;
		  cg_rotate(&v_x, &v_y, degrees);
		  PDF_rotate(carta->p, degrees);
		}
	      ys_del(&ys_rot);
	    }
	  if (entire && (!strcasecmp(entire, "yes") || !strcasecmp(entire, "true")))
	    fitmethod = "entire";
	  else
	    fitmethod = "meet";
	  ys_printf(&ys, "boxsize {%f %f} position 0 fitmethod %s",
		    YVAL2PT(v_w), YVAL2PT(v_h), fitmethod);
	  PDF_fit_image(carta->p, image_handle->i, YVAL2PT(v_x),
			YVAL2PT(v_y), ys);
	  PDF_restore(carta->p);
	}
      free_var(&e_x, &e_x);
      free_var(&e_y, &e_y);
      free_var(&e_w, &e_w);
      free_var(&e_h, &e_h);
      free_var(&e_d, &e_d);
    }
  /* free memory */
  if (file && width && height)
    {
      cg_unload_image(carta, image_handle);
      free0(image_handle);
    }
  free0(id);
  free0(x);
  free0(y);
  free0(width);
  free0(height);
  free0(rotation);
  free0(file);
  free0(mask);
  free0(entire);
  ys_del(&ys);
  return (res);
}
