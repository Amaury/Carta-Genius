#include <time.h>
#include <math.h>
#include "cartagenius.h"

/*
** cg_expr()
** Launch an expression parsing.
*/
yerr_t cg_expr(cg_t *carta, char *expr, cg_expr_var_t *result,
	       yunit_t default_unit)
{
  ystr_t ys, save;
  char *pt, *pt2;
  yerr_t return_value;
  yunit_t unit_save;

  unit_save = carta->default_unit;
  carta->default_unit = default_unit;
  /* remove spaces */
  ys = ys_new("");
  for (pt = expr; IS_SPACE(*pt); ++pt)
    ;
  for (pt2 = expr + strlen(expr) - 1; pt2 > pt && IS_SPACE(*pt2); --pt2)
    ;
  ys_ncat(&ys, pt, pt2 - pt + 1);
  /* check for "pure" value */
  for (pt = ys; *pt && *pt == MINUS; ++pt)
    ;
  for (; *pt && (IS_NUM(*pt) || *pt == DOT); ++pt)
    ;
  if (!*pt || (strlen(pt) == 2 &&
	       (!strcasecmp(pt, "mm") || !strcasecmp(pt, "cm") ||
		!strcasecmp(pt, "in") || !strcasecmp(pt, "pt"))))
    {
      result->type = CG_EXPR_VALUE;
      result->value.value = yvalue_read(ys, carta->default_unit);
      ys_del(&ys);
      carta->default_unit = unit_save;
      return (YENOERR);
    }
  /* parse complex expression */
  save = ys;
  return_value = cg_expr_eval(carta, &ys, result);
  ys_del(&save);
  carta->default_unit = unit_save;
  return (return_value);
}

/*
** cg_expr_get_operand()
** Read an expression and extract the next operand.
*/
yerr_t cg_expr_get_operand(cg_t *carta, char **expr, cg_expr_var_t *result)
{
  ystr_t ys;
  ybool_t var_found = YFALSE, have_char = YFALSE;
  yerr_t ret_val;
  cg_expr_var_t var1 = {0};
  int nbr_dot = 0, nbr_char = 0, nbr_digit = 0;

  while (IS_SPACE(**expr))
    (*expr)++;
  if (!**expr)
    {
      YLOG_ADD(YLOG_WARN, "Empty expression.");
      return (YEINVAL);
    }
  /* search for sub-evaluation */
  if (**expr == LPAR)
    {
      (*expr)++;
      if ((ret_val = cg_expr_eval(carta, expr, &var1)) != YENOERR)
	return (ret_val);
      (*expr)++;
      *result = var1;
      return (YENOERR);
    }
  /* get operand */
  ys = ys_new("");
  for (; !var_found && **expr; )
    {
      if (**expr == DOLLAR)
	{
	  /* it is a variable */
	  (*expr)++;
	  while (IS_CHAR(**expr) || IS_NUM(**expr) || **expr == UNDERSCORE)
	    {
	      ys_addc(&ys, **expr);
	      (*expr)++;
	    }
	  if ((ret_val = cg_expr_get_var(carta, ys, &var1)) != YENOERR)
	    {
	      YLOG_ADD(YLOG_WARN, "Unknown variable (%s).", ys);
	      ys_del(&ys);
	      return (ret_val);
	    }
	  var_found = YTRUE;
	}
      else if (**expr == SHARP)
	{
	  unsigned int red, green, blue;
	  /* it is a color */
	  (*expr)++;
	  while (IS_HEXA(**expr))
	    {
	      ys_addc(&ys, **expr);
	      (*expr)++;
	    }
	  sscanf(ys, "%2x%2x%2x", &red, &green, &blue);
	  var1.type = CG_EXPR_COLOR;
	  var1.value.color.red = (double)red / 255;
	  var1.value.color.green = (double)green / 255;
	  var1.value.color.blue = (double)blue / 255;
	  var_found = YTRUE;
	}
      else if (**expr == DQUOTE)
	{
	  /* it is a string */
	  (*expr)++;
	  while (**expr != DQUOTE && **expr)
	    {
	      ys_addc(&ys, **expr);
	      (*expr)++;
	    }
	  if (!**expr)
	    {
	      ys_del(&ys);
	      YLOG_ADD(YLOG_WARN, "Unterminated string.");
	      return (YEINVAL);
	    }
	  (*expr)++;
	  var1.type = CG_EXPR_STRING;
	  var1.value.string = ys_string(ys);
	  var_found = YTRUE;
	}
      else
	{
	  if (**expr == DOT)
	    nbr_dot++;
	  else if (IS_CHAR(**expr))
	    {
	      have_char = YTRUE;
	      nbr_char++;
	    }
	  if ((!nbr_digit && nbr_char == 1 && **expr == '-') ||
	      IS_CHAR(**expr) || IS_NUM(**expr) || **expr == UNDERSCORE ||
	      **expr == DOT)
	    {
	      ys_addc(&ys, **expr);
	      (*expr)++;
	    }
	  else
	    break ;
	}
    }
  /* search for operand meaning */
  if (!var_found)
    {
      if (**expr == LPAR && !nbr_dot && have_char)
	{
	  /* it is a function */
	  (*expr)++;
	  if ((ret_val = cg_expr_do_func(carta, ys, expr, &var1)) == YENOERR)
	    var_found = YTRUE;
	  (*expr)++;
	}
      else if (nbr_dot < 2 && !have_char)
	{
	  /* it is a scalar */
	  var1.type = CG_EXPR_SCALAR;
	  var1.value.scalar = atof(ys);
	  var_found = YTRUE;
	}
      else if (nbr_dot < 2 && nbr_char == 2 &&
	       (!strcasecmp(ys + ys_len(ys) - 2, "pt") ||
		!strcasecmp(ys + ys_len(ys) - 2, "mm") ||
		!strcasecmp(ys + ys_len(ys) - 2, "cm") ||
		!strcasecmp(ys + ys_len(ys) - 2, "in")))
	{
	  /* it is a value */
	  var1.type = CG_EXPR_VALUE;
	  var1.value.value = yvalue_read(ys, carta->default_unit);
	  var_found = YTRUE;
	}
      else
	{
	  double f1, f2;
	  char c1, c2, sunit[3] = {'\0', '\0', '\0'};
	  if (sscanf(ys, "%lfx%lf%c%c", &f1, &f2, &c1, &c2) == 4 &&
	      (((c1 == 'p' || c1 == 'P') && (c2 == 't' || c2 == 'T')) ||
	       ((c1 == 'm' || c1 == 'M') && (c2 == 'm' || c2 == 'M')) ||
	       ((c1 == 'c' || c1 == 'C') && (c2 == 'm' || c2 == 'M')) ||
	       ((c1 == 'i' || c1 == 'I') && (c2 == 'n' || c2 == 'N'))))
	    {
	      /* it is an element */
	      sunit[0] = c1;
	      sunit[1] = c2;
	      var1.type = CG_EXPR_ELEMENT;
	      var1.value.element.width.value = f1;
	      var1.value.element.height.value = f2;
	      var1.value.element.width.unit =
		var1.value.element.height.unit =
		(!strcasecmp(sunit, "pt") ? YUNIT_PT :
		 !strcasecmp(sunit, "mm") ? YUNIT_MM :
		 !strcasecmp(sunit, "cm") ? YUNIT_CM : YUNIT_IN);
	      var_found = YTRUE;
	    }
	  else
	    YLOG_ADD(YLOG_WARN, "Unknown operand type (%s).", ys);
	}
    }
  ys_del(&ys);
  if (!var_found)
    return (YEINVAL);
  while (IS_SPACE(**expr))
    (*expr)++;
  *result = var1;
  return (YENOERR);
}

/*
** cg_expr_eval()
** Do the evaluation of an expresion.
*/
yerr_t cg_expr_eval(cg_t *carta, char **expr, cg_expr_var_t *result)
{
  char operator[2] = {'\0', '\0'}, first_op, *pt_operand2 = NULL;
  yerr_t ret_val = YENOERR;
  cg_expr_var_t var1 = {0}, var2 = {0}, var3 = {0};
  cg_expr_func_t func = {0};

  if (!expr || !*expr)
    {
      YLOG_ADD(YLOG_WARN, "Invalid expression.");
      return (YEINVAL);
    }
  /* get first operand */
  if ((ret_val = cg_expr_get_operand(carta, expr, &var1)) != YENOERR)
    return (ret_val);
  /* search for end of evaluation */
  while (**expr && **expr != RPAR && **expr != COMMA && ret_val == YENOERR)
    {
      while (IS_SPACE(**expr))
	(*expr)++;
      /* search the first operator */
      if (!CG_IS_OPERATOR(**expr))
	{
	  YLOG_ADD(YLOG_WARN, "Invalid operator.");
	  free_var(&var1, &var1);
	  return (YEINVAL);
	}
      operator[0] = first_op = **expr;
      (*expr)++;
      /* get the first operator and the second operand */
      if ((ret_val = cg_expr_get_func(carta, operator, 2, &func)) != YENOERR)
	{
	  free_var(&var1, &var1);
	  YLOG_ADD(YLOG_WARN, "Unknown operator (%s).", operator);
	}
      pt_operand2 = *expr;
      if ((ret_val = cg_expr_get_operand(carta, expr, &var2)) != YENOERR)
	{
	  YLOG_ADD(YLOG_WARN, "Unable to find second operand.");
	  free_func(&func, &func);
	  free_var(&var1, &var1);
	  return (YEINVAL);
	}
      /* search for end of evaluation */
      if (**expr == RPAR || **expr == COMMA || !**expr)
	{
	  ret_val = func.f(carta, &var3, var1, var2);
	  free_func(&func, &func);
	  free_var(&var1, &var1);
	  free_var(&var2, &var2);
	  *result = var3;
	  return (ret_val);
	}
      /* check first operator priority */
      if (CG_LEVEL4_OPERATOR(first_op))
	{
	  ret_val = func.f(carta, &var3, var1, var2);
	  free_func(&func, &func);
	  free_var(&var1, &var1);
	  free_var(&var2, &var2);
	  var1 = var3;
	  continue ;
	}
      while (IS_SPACE(**expr))
	(*expr)++;
      /* search the second operator */
      if (!CG_IS_OPERATOR(**expr))
	{
	  YLOG_ADD(YLOG_WARN, "Invalid operator.");
	  free_func(&func, &func);
	  free_var(&var1, &var1);
	  free_var(&var2, &var2);
	  return (YEINVAL);
	}
      /* apply operators */
      /* apply the first operator if it has an higher or equal level
	 than the second operator ; otherwise, process the second
	 operator prior to the first */
      if (CG_LEVEL4_OPERATOR(first_op) ||
	  (CG_LEVEL3_OPERATOR(first_op) && !CG_LEVEL4_OPERATOR(**expr)) ||
	  (CG_LEVEL2_OPERATOR(first_op) && !CG_LEVEL3_OPERATOR(**expr) &&
	   !CG_LEVEL4_OPERATOR(**expr)) || CG_LEVEL1_OPERATOR(**expr))
	{
	  /* first operation */
	  if ((ret_val = func.f(carta, &var3, var1, var2)) != YENOERR)
	    {
	      free_func(&func, &func);
	      free_var(&var1, &var1);
	      free_var(&var2, &var2);
	      return (ret_val);
	    }
	  free_func(&func, &func);
	  free_var(&var1, &var1);
	  free_var(&var2, &var2);
	  var1 = var3;
	}
      else
	{
	  *expr = pt_operand2;
	  free_var(&var2, &var2);
	  /* compute the rest of expression */
	  if ((ret_val = cg_expr_eval(carta, expr, &var2)) != YENOERR)
	    {
	      free_func(&func, &func);
	      free_var(&var1, &var1);
	      return (ret_val);
	    }
	  /* first operation */
	  ret_val = func.f(carta, &var3, var1, var2);
	  free_var(&var1, &var1);
	  free_var(&var2, &var2);
	  free_func(&func, &func);
	  var1 = var3;
	}
    }
  *result = var1;
  return (ret_val);
}

/*
** cg_expr_do_func()
** Process a function.
*/
yerr_t cg_expr_do_func(cg_t *carta, char *name, char **expr,
		       cg_expr_var_t *result)
{
  cg_expr_func_t func = {0};
  cg_expr_var_t *varp, var_res = {0};
  yvect_t array;
  yerr_t ret_val;
  ybool_t first_param = YTRUE;

  array = yv_new();
  if (**expr == RPAR)
    (*expr)++;
  else
    {
      while (first_param || **expr == COMMA)
	{
	  if (**expr == COMMA)
	    (*expr)++;
	  first_param = YFALSE;
	  varp = malloc0(sizeof(cg_expr_var_t));
	  yv_add(&array, varp);
	  if ((ret_val = cg_expr_eval(carta, expr, varp)) != YENOERR)
	    {
	      yv_del(&array, free_var, NULL);
	      return (YEINVAL);
	    }
	}
    }
  if ((ret_val = cg_expr_get_func(carta, name, yv_len(array), &func)) !=
      YENOERR)
    {
      yv_del(&array, free_var, NULL);
      return (YEINVAL);
    }
  switch (func.nbr_params)
    {
    case 0:
      ret_val = func.f(carta, &var_res);
      break ;
    case 1:
      ret_val = func.f(carta, &var_res, *((cg_expr_var_t*)array[0]));
      break ;
    case 2:
      ret_val = func.f(carta, &var_res, *((cg_expr_var_t*)array[0]),
		       *((cg_expr_var_t*)array[1]));
      break ;
    case 3:
      ret_val = func.f(carta, &var_res, *((cg_expr_var_t*)array[0]),
		       *((cg_expr_var_t*)array[1]),
		       *((cg_expr_var_t*)array[2]));
      break ;
    default:
      YLOG_ADD(YLOG_WARN, "Too much parameters for function '%s'.", name);
      ret_val = YEINVAL;
    }
  yv_del(&array, free_var, NULL);
  *result = var_res;
  return (ret_val);
}

/* ******************** setting functions ******************** */

/*
** cg_expr_set_precision()
** Update expression precision.
*/
yerr_t cg_expr_set_precision(cg_t *carta, char *str)
{
  if (!str || !strlen(str))
    {
      YLOG_ADD(YLOG_WARN, "Empty precision.");
      return (YEINVAL);
    }
  carta->expr->precision = atoi(str);
  return (YENOERR);
}

/*
** cg_expr_set_bool()
** Add or update a boolean variable.
*/
yerr_t cg_expr_set_bool(cg_t *carta, char *name, ybool_t boolean)
{
  cg_expr_t *expr;
  int i;
  cg_expr_var_t *var;

  if (!carta || !(expr = carta->expr) || !expr->vars)
    return (YEINVAL);
  for (i = 0; i < yv_len(expr->vars); ++i)
    {
      var = expr->vars[i];
      if (!strcmp(name, var->name))
	{
	  if (var->type == CG_EXPR_STRING)
	    free0(var->value.string);
	  var->type = CG_EXPR_BOOL;
	  var->value.boolean = boolean;
	  return (YENOERR);
	}
    }
  if (!(var = malloc0(sizeof(cg_expr_var_t))))
    return (YENOMEM);
  var->name = strdup(name);
  var->type = CG_EXPR_BOOL;
  var->value.boolean = boolean;
  yv_add(&expr->vars, var);
  return (YENOERR);
}

/*
** cg_expr_set_scalar()
** Add or update a scalar variable.
*/
yerr_t cg_expr_set_scalar(cg_t *carta, char *name, double scalar)
{
  cg_expr_t *expr;
  int i;
  cg_expr_var_t *var;

  if (!carta || !(expr = carta->expr) || !expr->vars)
    return (YEINVAL);
  for (i = 0; i < yv_len(expr->vars); ++i)
    {
      var = expr->vars[i];
      if (!strcmp(name, var->name))
	{
	  if (var->type == CG_EXPR_STRING)
	    free0(var->value.string);
	  var->type = CG_EXPR_SCALAR;
	  var->value.scalar = scalar;
	  return (YENOERR);
	}
    }
  if (!(var = malloc0(sizeof(cg_expr_var_t))))
    return (YENOMEM);
  var->name = strdup(name);
  var->type = CG_EXPR_SCALAR;
  var->value.scalar = scalar;
  yv_add(&expr->vars, var);
  return (YENOERR);
}

/*
** cg_expr_set_value()
** Add or update a value variable.
*/
yerr_t cg_expr_set_value(cg_t *carta, char *name, yvalue_t value)
{
  cg_expr_t *expr;
  int i;
  cg_expr_var_t *var;

  if (!carta || !(expr = carta->expr) || !expr->vars)
    return (YEINVAL);
  for (i = 0; i < yv_len(expr->vars); ++i)
    {
      var = expr->vars[i];
      if (!strcmp(name, var->name))
	{
	  if (var->type == CG_EXPR_STRING)
	    free0(var->value.string);
	  var->type = CG_EXPR_VALUE;
	  var->value.value = value;
	  return (YENOERR);
	}
    }
  if (!(var = malloc0(sizeof(cg_expr_var_t))))
    return (YENOMEM);
  var->name = strdup(name);
  var->type = CG_EXPR_VALUE;
  var->value.value = value;
  yv_add(&expr->vars, var);
  return (YENOERR);
}

/*
** cg_expr_set_string()
** Add or update a string variable.
*/
yerr_t cg_expr_set_string(cg_t *carta, char *name, char *string)
{
  cg_expr_t *expr;
  int i;
  cg_expr_var_t *var;

  if (!carta || !(expr = carta->expr) || !expr->vars)
    return (YEINVAL);
  for (i = 0; i < yv_len(expr->vars); ++i)
    {
      var = expr->vars[i];
      if (!strcmp(name, var->name))
	{
	  if (var->type == CG_EXPR_STRING)
	    free0(var->value.string);
	  var->type = CG_EXPR_STRING;
	  var->value.string = strdup(string ? string : "");
	  return (YENOERR);
	}
    }
  if (!(var = malloc0(sizeof(cg_expr_var_t))))
    return (YENOMEM);
  var->name = strdup(name);
  var->type = CG_EXPR_STRING;
  var->value.string = strdup(string ? string : "");
  yv_add(&expr->vars, var);
  return (YENOERR);
}

/*
** cg_expr_set_element()
** Add or update an element variable.
*/
yerr_t cg_expr_set_element(cg_t *carta, char *name, yvalue_t width,
			   yvalue_t height)
{
  cg_expr_t *expr;
  int i;
  cg_expr_var_t *var;

  if (!carta || !(expr = carta->expr) || !expr->vars)
    return (YEINVAL);
  for (i = 0; i < yv_len(expr->vars); ++i)
    {
      var = expr->vars[i];
      if (!strcmp(name, var->name))
	{
	  if (var->type == CG_EXPR_STRING)
	    free0(var->value.string);
	  var->type = CG_EXPR_ELEMENT;
	  var->value.element.width = width;
	  var->value.element.height = height;
	  return (YENOERR);
	}
    }
  if (!(var = malloc0(sizeof(cg_expr_var_t))))
    return (YENOMEM);
  var->name = strdup(name);
  var->type = CG_EXPR_ELEMENT;
  var->value.element.width = width;
  var->value.element.height = height;
  yv_add(&expr->vars, var);
  return (YENOERR);
}

/*
** cg_expr_set_color()
** Add or update a color variable.
*/
yerr_t cg_expr_set_color(cg_t *carta, char *name, double red,
			 double green, double blue)
{
  cg_expr_t *expr;
  int i;
  cg_expr_var_t *var;

  if (!carta || !(expr = carta->expr) || !expr->vars)
    return (YEINVAL);
  for (i = 0; i < yv_len(expr->vars); ++i)
    {
      var = expr->vars[i];
      if (!strcmp(name, var->name))
	{
	  if (var->type == CG_EXPR_STRING)
	    free0(var->value.string);
	  var->type = CG_EXPR_COLOR;
	  var->value.color.red = red;
	  var->value.color.green = green;
	  var->value.color.blue = blue;
	  return (YENOERR);
	}
    }
  if (!(var = malloc0(sizeof(cg_expr_var_t))))
    return (YENOMEM);
  var->name = strdup(name);
  var->type = CG_EXPR_COLOR;
  var->value.color.red = red;
  var->value.color.green = green;
  var->value.color.blue = blue;
  yv_add(&expr->vars, var);
  return (YENOERR);
}

/*
** cg_expr_set_func()
** Add or update a function pointer.
*/
yerr_t cg_expr_set_func(cg_t *carta, char *name, int nbr_params,
			cg_expr_func_ptr_t f)
{
  cg_expr_t *expr;
  int i;
  cg_expr_func_t *func;

  if (!carta || !(expr = carta->expr) || !expr->funcs)
    return (YEINVAL);
  for (i = 0; i < yv_len(expr->funcs); ++i)
    {
      func = expr->funcs[i];
      if (!strcmp(name, func->name))
	{
	  func->nbr_params = nbr_params;
	  func->f = f;
	  return (YENOERR);
	}
    }
  if (!(func = malloc0(sizeof(cg_expr_func_t))))
    return (YENOMEM);
  func->name = strdup(name);
  func->nbr_params = nbr_params;
  func->f = f;
  yv_add(&expr->funcs, func);
  return (YENOERR);
}

/*
** cg_expr_get_var()
** Return a variable from its name.
*/
yerr_t cg_expr_get_var(cg_t *carta, char *name, cg_expr_var_t *result)
{
  cg_expr_t *expr;
  int i;
  cg_expr_var_t *var;

  if (!carta || !(expr = carta->expr) || !expr->vars)
    return (YEINVAL);
  for (i = 0; i < yv_len(expr->vars); ++i)
    {
      var = expr->vars[i];
      if (!strcmp(name, var->name))
	{
	  result->name = strdup(var->name);
	  result->type = var->type;
	  if (var->type == CG_EXPR_SCALAR)
	    result->value.scalar = var->value.scalar;
	  else if (var->type == CG_EXPR_VALUE)
	    result->value.value = var->value.value;
	  else if (var->type == CG_EXPR_STRING)
	    result->value.string = var->value.string ?
	      strdup(var->value.string) : NULL;
	  else if (var->type == CG_EXPR_ELEMENT)
	    {
	      result->value.element.width = var->value.element.width;
	      result->value.element.height = var->value.element.height;
	    }
	  else if (var->type == CG_EXPR_COLOR)
	    {
	      result->value.color.red = var->value.color.red;
	      result->value.color.green = var->value.color.green;
	      result->value.color.blue = var->value.color.blue;
	    }
	  else if (var->type == CG_EXPR_BOOL)
	    result->value.boolean = var->value.boolean;
	  return (YENOERR);
	}
    }
  YLOG_ADD(YLOG_WARN, "No variable exists with the name '%s'.", name);
  return (YEINVAL);
}

/*
** cg_expr_get_func()
** Return a function from its name.
*/
yerr_t cg_expr_get_func(cg_t *carta, char *name, int nbr_params,
			cg_expr_func_t *result)
{
  cg_expr_t *expr;
  int i;
  cg_expr_func_t *func;

  if (!carta || !(expr = carta->expr) || !expr->funcs)
    return (YEINVAL);
  for (i = 0; i < yv_len(expr->funcs); ++i)
    {
      func = expr->funcs[i];
      if (!strcmp(name, func->name))
	{
	  if (func->nbr_params != nbr_params)
	    {
	      YLOG_ADD(YLOG_WARN, "Wrong arguments count to function '%s'.", name);
	      return (YEINVAL);
	    }
	  result->name = strdup(func->name);
	  result->nbr_params = func->nbr_params;
	  result->f = func->f;
	  return (YENOERR);
	}
    }
  YLOG_ADD(YLOG_WARN, "Unknown function '%s'", name);
  return (YEINVAL);
}

/* *********************** expr functions ********************** */

/*
** cg_expr_f_var()
** Return the value of a variable from its name.
*/
yerr_t cg_expr_f_var(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};
  ystr_t ys_expr;
  yerr_t res = YENOERR;

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type != CG_EXPR_STRING)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  ys_expr = ys_new("");
  ys_printf(&ys_expr, "(%s)", var.value.string);
  res = cg_expr(carta, ys_expr, result, carta->default_unit);
  ys_del(&ys_expr);
  return (res);
}

/*
** cg_expr_f_width()
** Return the width of an element.
*/
yerr_t cg_expr_f_width(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type != CG_EXPR_ELEMENT)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  result->type = CG_EXPR_VALUE;
  result->value.value = var.value.element.width;
  return (YENOERR);
}

/*
** cg_expr_f_height()
** Return the height of an element.
*/
yerr_t cg_expr_f_height(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type != CG_EXPR_ELEMENT)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  result->type = CG_EXPR_VALUE;
  result->value.value = var.value.element.height;
  return (YENOERR);
}

/*
** cg_expr_f_landscape()
** Invert width and height of an element.
*/
yerr_t cg_expr_f_landscape(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type != CG_EXPR_ELEMENT)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  result->type = CG_EXPR_ELEMENT;
  result->value.element.width = var.value.element.height;
  result->value.element.height = var.value.element.width;
  return (YENOERR);
}

/*
** cg_expr_f_plus()
** Add a value to another one.
*/
yerr_t cg_expr_f_plus(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  result->type = var1.type;
  if (var1.type == CG_EXPR_STRING)
    {
      ystr_t ys1, ys2, ys3;
      ys1 = ys_new("");
      ys_printf(&ys1, "%%.%df", carta->expr->precision);
      ys2 = ys_new("");
      if (var2.type == CG_EXPR_STRING)
	ys_cat(&ys2, var2.value.string ? var2.value.string : "");
      else if (var2.type == CG_EXPR_SCALAR)
	ys_printf(&ys2, (carta->expr->precision < 0) ? "%f" : ys1,
		  var2.value.scalar);
      else if (var2.type == CG_EXPR_VALUE)
	ys_printf(&ys2, (carta->expr->precision < 0) ? "%f" : ys1,
		  yvalue_get(var2.value.value, carta->default_unit));
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  ys_del(&ys2);
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter (element).");
	  return (YEINVAL);
	}
      else if (var2.type == CG_EXPR_COLOR)
	ys_printf(&ys2, "#%x%x%x", (unsigned int)(var2.value.color.red * 255),
		  (unsigned int)(var2.value.color.green * 255),
		  (unsigned int)(var2.value.color.blue * 255));
      ys_del(&ys1);
      ys1 = ys_new(var1.value.string ? var1.value.string : "");
      ys3 = ys_new("");
      ys_printf(&ys3, "%s%s", ys1, ys2);
      ys_del(&ys1);
      ys_del(&ys2);
      result->value.string = ys_string(ys3);
      ys_del(&ys3);
    }
  else if (var1.type == CG_EXPR_ELEMENT)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value + var2.value.scalar;
	  result->value.element.height.value =
	    var1.value.element.height.value + var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value +
	    yvalue_get(var2.value.value, var1.value.element.width.unit);
	  result->value.element.height.value =
	    var1.value.element.height.value +
	    yvalue_get(var2.value.value, var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value +
	    yvalue_get(var2.value.element.width,
		       var1.value.element.width.unit);
	  result->value.element.height.value =
	    var1.value.element.height.value +
	    yvalue_get(var2.value.element.height,
		       var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_STRING || var2.type == CG_EXPR_COLOR)
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_SCALAR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	result->value.scalar = var1.value.scalar + var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.scalar = var1.value.scalar +
	  yvalue_get(var2.value.value, carta->default_unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      result->value.value = var1.value.value;
      if (var2.type == CG_EXPR_SCALAR)
	result->value.value.value += var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.value.value += yvalue_get(var2.value.value,
						var1.value.value.unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.color.red = var1.value.color.red + var2.value.scalar;
	  result->value.color.green = var1.value.color.green + var2.value.scalar;
	  result->value.color.blue = var1.value.color.blue + var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  result->value.color.red = var1.value.color.red + var2.value.color.red;
	  result->value.color.green = var1.value.color.green + var2.value.color.green;
	  result->value.color.blue = var1.value.color.blue + var2.value.color.blue;
	}
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
      result->value.color.red = (result->value.color.red > 1.0) ? 1.0 :
	 result->value.color.red;
      result->value.color.green = (result->value.color.green > 1.0) ? 1.0 :
	 result->value.color.green;
      result->value.color.blue = (result->value.color.blue > 1.0) ? 1.0 :
	 result->value.color.blue;
    }
  return (YENOERR);
}

/*
** cg_expr_f_minus()
** Substract a value to another one.
*/
yerr_t cg_expr_f_minus(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type == CG_EXPR_STRING)
    {
      YLOG_ADD(YLOG_WARN, "Invalid first parameter (string).");
      return (YEINVAL);
    }
  result->type = var1.type;
  if (var1.type == CG_EXPR_ELEMENT)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value - var2.value.scalar;
	  result->value.element.height.value =
	    var1.value.element.height.value - var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value -
	    yvalue_get(var2.value.value, var1.value.element.width.unit);
	  result->value.element.height.value =
	    var1.value.element.height.value -
	    yvalue_get(var2.value.value, var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value -
	    yvalue_get(var2.value.element.width,
		       var1.value.element.width.unit);
	  result->value.element.height.value =
	    var1.value.element.height.value -
	    yvalue_get(var2.value.element.height,
		       var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter (color).");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_SCALAR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	result->value.scalar = var1.value.scalar - var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.scalar = var1.value.scalar -
	  yvalue_get(var2.value.value, carta->default_unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      result->value.value = var1.value.value;
      if (var2.type == CG_EXPR_SCALAR)
	result->value.value.value -= var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.value.value -= yvalue_get(var2.value.value,
						var1.value.value.unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.color.red = var1.value.color.red - var2.value.scalar;
	  result->value.color.green = var1.value.color.green - var2.value.scalar;
	  result->value.color.blue = var1.value.color.blue - var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  result->value.color.red = var1.value.color.red - var2.value.color.red;
	  result->value.color.green = var1.value.color.green - var2.value.color.green;
	  result->value.color.blue = var1.value.color.blue - var2.value.color.blue;
	}
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
      result->value.color.red = (result->value.color.red > 1.0) ? 1.0 :
	 result->value.color.red;
      result->value.color.green = (result->value.color.green > 1.0) ? 1.0 :
	 result->value.color.green;
      result->value.color.blue = (result->value.color.blue > 1.0) ? 1.0 :
	 result->value.color.blue;
    }
  return (YENOERR);
}

/*
** cg_expr_f_mult()
** Multiply a value to another one.
*/
yerr_t cg_expr_f_mult(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type == CG_EXPR_STRING)
    {
      YLOG_ADD(YLOG_WARN, "Invalid first parameter (string).");
      return (YEINVAL);
    }
  result->type = var1.type;
  if (var1.type == CG_EXPR_ELEMENT)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value * var2.value.scalar;
	  result->value.element.height.value =
	    var1.value.element.height.value * var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value *
	    yvalue_get(var2.value.value, var1.value.element.width.unit);
	  result->value.element.height.value =
	    var1.value.element.height.value *
	    yvalue_get(var2.value.value, var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value *
	    yvalue_get(var2.value.element.width,
		       var1.value.element.width.unit);
	  result->value.element.height.value =
	    var1.value.element.height.value *
	    yvalue_get(var2.value.element.height,
		       var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_SCALAR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	result->value.scalar = var1.value.scalar * var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.scalar = var1.value.scalar *
	  yvalue_get(var2.value.value, carta->default_unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      result->value.value = var1.value.value;
      if (var2.type == CG_EXPR_SCALAR)
	result->value.value.value *= var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.value.value *= yvalue_get(var2.value.value,
						var1.value.value.unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.color.red = var1.value.color.red * var2.value.scalar;
	  result->value.color.green = var1.value.color.green * var2.value.scalar;
	  result->value.color.blue = var1.value.color.blue * var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  result->value.color.red = var1.value.color.red * var2.value.color.red;
	  result->value.color.green = var1.value.color.green * var2.value.color.green;
	  result->value.color.blue = var1.value.color.blue * var2.value.color.blue;
	}
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
      result->value.color.red = (result->value.color.red > 1.0) ? 1.0 :
	 result->value.color.red;
      result->value.color.green = (result->value.color.green > 1.0) ? 1.0 :
	 result->value.color.green;
      result->value.color.blue = (result->value.color.blue > 1.0) ? 1.0 :
	 result->value.color.blue;
    }
  return (YENOERR);
}

/*
** cg_expr_f_div()
** Divide a value to another one.
*/
yerr_t cg_expr_f_div(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type == CG_EXPR_STRING)
    {
      YLOG_ADD(YLOG_WARN, "Invalid first parameter (string).");
      return (YEINVAL);
    }
  if ((var2.type == CG_EXPR_SCALAR && !var2.value.scalar) ||
      (var2.type == CG_EXPR_VALUE && !var2.value.value.value) ||
      (var2.type == CG_EXPR_ELEMENT &&
       (!var2.value.element.width.value || !var2.value.element.height.value)) ||
      (var2.type == CG_EXPR_COLOR &&
       (!var2.value.color.red || !var2.value.color.green || !var2.value.color.blue)))
    {
      YLOG_ADD(YLOG_WARN, "Invalid second parameter (division by 0).");
      return (YEINVAL);
    }
  result->type = var1.type;
  if (var1.type == CG_EXPR_ELEMENT)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value / var2.value.scalar;
	  result->value.element.height.value =
	    var1.value.element.height.value / var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value /
	    yvalue_get(var2.value.value, var1.value.element.width.unit);
	  result->value.element.height.value =
	    var1.value.element.height.value /
	    yvalue_get(var2.value.value, var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    var1.value.element.width.value /
	    yvalue_get(var2.value.element.width,
		       var1.value.element.width.unit);
	  result->value.element.height.value =
	    var1.value.element.height.value /
	    yvalue_get(var2.value.element.height,
		       var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter (color).");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_SCALAR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	result->value.scalar = var1.value.scalar / var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.scalar = var1.value.scalar /
	  yvalue_get(var2.value.value, carta->default_unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      result->value.value = var1.value.value;
      if (var2.type == CG_EXPR_SCALAR)
	result->value.value.value /= var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.value.value /= yvalue_get(var2.value.value,
						var1.value.value.unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.color.red = var1.value.color.red / var2.value.scalar;
	  result->value.color.green = var1.value.color.green / var2.value.scalar;
	  result->value.color.blue = var1.value.color.blue / var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  result->value.color.red = var1.value.color.red / var2.value.color.red;
	  result->value.color.green = var1.value.color.green / var2.value.color.green;
	  result->value.color.blue = var1.value.color.blue / var2.value.color.blue;
	}
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
      result->value.color.red = (result->value.color.red > 1.0) ? 1.0 :
	 result->value.color.red;
      result->value.color.green = (result->value.color.green > 1.0) ? 1.0 :
	 result->value.color.green;
      result->value.color.blue = (result->value.color.blue > 1.0) ? 1.0 :
	 result->value.color.blue;
    }
  return (YENOERR);
}

/*
** cg_expr_f_mod()
** Add a value to another one.
*/
yerr_t cg_expr_f_mod(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type == CG_EXPR_STRING)
    {
      YLOG_ADD(YLOG_WARN, "Invalid first parameter (string).");
      return (YEINVAL);
    }
  result->type = var1.type;
  if (var1.type == CG_EXPR_ELEMENT)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    (int)var1.value.element.width.value % (int)var2.value.scalar;
	  result->value.element.height.value =
	    (int)var1.value.element.height.value % (int)var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    (int)var1.value.element.width.value %
	    (int)yvalue_get(var2.value.value, var1.value.element.width.unit);
	  result->value.element.height.value =
	    (int)var1.value.element.height.value %
	    (int)yvalue_get(var2.value.value, var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value =
	    (int)var1.value.element.width.value %
	    (int)yvalue_get(var2.value.element.width,
			    var1.value.element.width.unit);
	  result->value.element.height.value =
	    (int)var1.value.element.height.value %
	    (int)yvalue_get(var2.value.element.height,
		       var1.value.element.height.unit);
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter (color).");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_SCALAR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	result->value.scalar = (int)var1.value.scalar %
	  (int)var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.scalar = (int)var1.value.scalar %
	  (int)yvalue_get(var2.value.value, carta->default_unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      result->value.value = var1.value.value;
      if (var2.type == CG_EXPR_SCALAR)
	result->value.value.value = (int)result->value.value.value %
	  (int)var2.value.scalar;
      else if (var2.type == CG_EXPR_VALUE)
	result->value.value.value = (int)result->value.value.value %
	  (int)yvalue_get(var2.value.value, var1.value.value.unit);
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      int red, green, blue;
      red = (int)(var1.value.color.red * 255);
      green = (int)(var1.value.color.green * 255);
      blue = (int)(var1.value.color.blue * 255);
      if (var2.type == CG_EXPR_SCALAR)
	{
	  int scalar = (int)(var2.value.scalar * 255);
	  result->value.color.red = (double)(red % scalar) / 255;
	  result->value.color.green = (double)(green % scalar) / 255;
	  result->value.color.blue = (double)(blue % scalar) / 255;
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  int red2, green2, blue2;
	  red2 = (int)(var2.value.color.red * 255);
	  green2 = (int)(var2.value.color.green * 255);
	  blue2 = (int)(var2.value.color.blue * 255);
	  result->value.color.red = (double)(red % red2) / 255;
	  result->value.color.green = (double)(green % green2) / 255;
	  result->value.color.blue = (double)(blue % blue2) / 255;
	}
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
      result->value.color.red = (result->value.color.red > 1.0) ? 1.0 :
	 result->value.color.red;
      result->value.color.green = (result->value.color.green > 1.0) ? 1.0 :
	 result->value.color.green;
      result->value.color.blue = (result->value.color.blue > 1.0) ? 1.0 :
	 result->value.color.blue;
    }
  return (YENOERR);
}

/*
** cg_expr_f_sqrt()
** Compute the square root of a value.
*/
yerr_t cg_expr_f_sqrt(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type == CG_EXPR_STRING || var1.type == CG_EXPR_COLOR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter type.");
      return (YEINVAL);
    }
  result->type = var1.type;
  if (var1.type == CG_EXPR_ELEMENT)
    {
      if (var1.value.element.width.value < 0 ||
	  var1.value.element.height.value < 0)
	{
	  YLOG_ADD(YLOG_WARN, "Invalid parameter value (negative).");
	  return (YEINVAL);
	}
      result->value.element.width.unit = var1.value.element.width.unit;
      result->value.element.height.unit = var1.value.element.height.unit;
      result->value.element.width.value = sqrt(var1.value.element.width.value);
      result->value.element.height.value = sqrt(var1.value.element.height.value);
    }
  else if (var1.type == CG_EXPR_SCALAR)
    {
      if (var1.value.scalar < 0)
	{
	  YLOG_ADD(YLOG_WARN, "Invalid parameter value (negative).");
	  return (YEINVAL);
	}
      result->value.scalar = sqrt(var1.value.scalar);
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      if (var1.value.value.value < 0)
	{
	  YLOG_ADD(YLOG_WARN, "Invalid parameter value (negative).");
	  return (YEINVAL);
	}
      result->value.value = var1.value.value;
      result->value.value.value = sqrt(var1.value.value.value);
    }
  return (YENOERR);
}

/*
** cg_expr_f_pow()
** Compute the power of a variable by another.
*/
yerr_t cg_expr_f_pow(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type == CG_EXPR_STRING || var1.type == CG_EXPR_COLOR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid first parameter.");
      return (YEINVAL);
    }
  result->type = var1.type;
  if (var1.type == CG_EXPR_ELEMENT)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value = pow(var1.value.element.width.value,
						  var2.value.scalar);
	  result->value.element.height.value = pow(var1.value.element.height.value,
						   var2.value.scalar);
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value = pow(var1.value.element.width.value,
						  yvalue_get(var2.value.value,
							     var1.value.element.width.unit));
	  result->value.element.height.value = pow(var1.value.element.height.value,
						   yvalue_get(var2.value.value,
							      var1.value.element.height.unit));
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  result->value.element.width.unit = var1.value.element.width.unit;
	  result->value.element.height.unit = var1.value.element.height.unit;
	  result->value.element.width.value = pow(var1.value.element.width.value,
						  yvalue_get(var2.value.element.width,
							     var1.value.element.width.unit));
	  result->value.element.height.value = pow(var1.value.element.height.value,
						   yvalue_get(var2.value.element.height,
							      var1.value.element.height.unit));
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter (color).");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_SCALAR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	result->value.scalar = pow(var1.value.scalar, var2.value.scalar);
      else if (var2.type == CG_EXPR_VALUE)
	result->value.scalar = pow(var1.value.scalar,
				   yvalue_get(var2.value.value,
					      carta->default_unit));
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      result->value.value = var1.value.value;
      if (var2.type == CG_EXPR_SCALAR)
	result->value.value.value = pow(result->value.value.value,
					var2.value.scalar);
      else if (var2.type == CG_EXPR_VALUE)
	result->value.value.value = pow(result->value.value.value,
					yvalue_get(var2.value.value,
						   var1.value.value.unit));
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid second parameter.");
	  return (YEINVAL);
	}
    }
  return (YENOERR);
}

/*
** cg_expr_f_strlen()
** Return the length of a string.
*/
yerr_t cg_expr_f_strlen(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t text = {0};

  va_start(p_list, result);
  text = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (text.type != CG_EXPR_STRING)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  result->type = CG_EXPR_SCALAR;
  result->value.scalar = text.value.string ?
    (double)strlen(text.value.string) : 0.0;
  return (YENOERR);
}

/*
** cg_expr_f_strwidth()
** Return the width of a string.
*/
yerr_t cg_expr_f_strwidth(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t font_name = {0}, font_size = {0}, text = {0};
  int font_handle;

  va_start(p_list, result);
  font_name = va_arg(p_list, cg_expr_var_t);
  font_size = va_arg(p_list, cg_expr_var_t);
  text = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (font_name.type != CG_EXPR_STRING ||
      font_size.type != CG_EXPR_VALUE ||
      text.type != CG_EXPR_STRING)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  if ((font_handle = cg_get_font(carta, font_name.value.string)) == -1)
    {
      YLOG_ADD(YLOG_WARN, "Unknown font '%s'.", font_name.value.string);
      return (YEINVAL);
    }
  result->type = CG_EXPR_VALUE;
  result->value.value.unit = YUNIT_PT;
  result->value.value.value = PDF_stringwidth(carta->p, text.value.string,
					      font_handle,
					      YVAL2PT(font_size.value.value));
  return (YENOERR);
}

/*
** cg_expr_f_sin()
** Return the sine of a variable.
*/
yerr_t cg_expr_f_sin(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};
  double angle;

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type == CG_EXPR_SCALAR)
    {
      result->type = CG_EXPR_SCALAR;
      angle = var.value.scalar * M_PI / 180;
      result->value.scalar = sin(angle);
      return (YENOERR);
    }
  YLOG_ADD(YLOG_WARN, "Invalid parameter.");
  return (YEINVAL);
}

/*
** cg_expr_f_cos()
** Return the cosine of a variable.
*/
yerr_t cg_expr_f_cos(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};
  double angle;

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type == CG_EXPR_SCALAR)
    {
      result->type = CG_EXPR_SCALAR;
      angle = var.value.scalar * M_PI / 180;
      result->value.scalar = cos(angle);
      return (YENOERR);
    }
  YLOG_ADD(YLOG_WARN, "Invalid parameter.");
  return (YEINVAL);
}

/*
** cg_expr_f_tan()
** Return the tangent of a variable.
*/
yerr_t cg_expr_f_tan(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};
  double angle;

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type == CG_EXPR_SCALAR)
    {
      result->type = CG_EXPR_SCALAR;
      angle = var.value.scalar * M_PI / 180;
      result->value.scalar = tan(angle);
      return (YENOERR);
    }
  YLOG_ADD(YLOG_WARN, "Invalid parameter.");
  return (YEINVAL);
}

/*
** cg_expr_f_asin()
** Return the arc sine of a variable.
*/
yerr_t cg_expr_f_asin(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};
  double angle;

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type == CG_EXPR_SCALAR)
    {
      result->type = CG_EXPR_SCALAR;
      angle = var.value.scalar * M_PI / 180;
      result->value.scalar = asin(angle);
      return (YENOERR);
    }
  YLOG_ADD(YLOG_WARN, "Invalid parameter.");
  return (YEINVAL);
}

/*
** cg_expr_f_acos()
** Return the arc cosine of a variable.
*/
yerr_t cg_expr_f_acos(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};
  double angle;

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type == CG_EXPR_SCALAR)
    {
      result->type = CG_EXPR_SCALAR;
      angle = var.value.scalar * M_PI / 180;
      result->value.scalar = acos(angle);
      return (YENOERR);
    }
  YLOG_ADD(YLOG_WARN, "Invalid parameter.");
  return (YEINVAL);
}

/*
** cg_expr_f_tan()
** Return the arc tangent of a variable.
*/
yerr_t cg_expr_f_atan(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};
  double angle;

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type == CG_EXPR_SCALAR)
    {
      result->type = CG_EXPR_SCALAR;
      angle = var.value.scalar * M_PI / 180;
      result->value.scalar = atan(angle);
      return (YENOERR);
    }
  YLOG_ADD(YLOG_WARN, "Invalid parameter.");
  return (YEINVAL);
}

/*
** cg_expr_f_string()
** Cast a variable to string.
*/
yerr_t cg_expr_f_string(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};
  ystr_t ys, fs;

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  result->type = CG_EXPR_STRING;
  fs = ys_new("");
  ys_printf(&fs, "%%.%df", carta->expr->precision);
  ys = ys_new("");
  if (var.type == CG_EXPR_SCALAR)
    {
      ys_printf(&ys, (carta->expr->precision < 0) ? "%f" : fs,
		var.value.scalar);
      result->value.string = ys_string(ys);
    }
  else if (var.type == CG_EXPR_VALUE)
    {
      ystr_t ys1;
      ys1 = ys_new("");
      ys_printf(&ys1, (carta->expr->precision < 0) ? "%f" : fs,
		yvalue_get(var.value.value, carta->default_unit));
      ys_printf(&ys, "%s%s", ys1,
		(carta->default_unit == YUNIT_PT) ? "pt" :
		(carta->default_unit == YUNIT_MM) ? "mm" :
		(carta->default_unit == YUNIT_CM) ? "cm" : "in");
      result->value.string = ys_string(ys);
      ys_del(&ys1);
    }
  else if (var.type == CG_EXPR_STRING)
    result->value.string = strdup(var.value.string);
  else if (var.type == CG_EXPR_ELEMENT)
    {
      ystr_t ys1, ys2;
      ys1 = ys_new("");
      ys2 = ys_new("");
      ys_printf(&ys1, (carta->expr->precision < 0) ? "%f" : fs,
		yvalue_get(var.value.element.width, carta->default_unit));
      ys_printf(&ys2, (carta->expr->precision < 0) ? "%f" : fs,
		yvalue_get(var.value.element.height, carta->default_unit));
      ys_printf(&ys, "%sx%s%s", ys1, ys2,
		(carta->default_unit == YUNIT_PT) ? "pt" :
		(carta->default_unit == YUNIT_MM) ? "mm" :
		(carta->default_unit == YUNIT_CM) ? "cm" : "in");
      result->value.string = ys_string(ys);
      ys_del(&ys1);
      ys_del(&ys2);
    }
  else if (var.type == CG_EXPR_COLOR)
    {
      ystr_t ys;
      unsigned int red, green, blue;
      red = (unsigned int)(var.value.color.red * 255);
      green = (unsigned int)(var.value.color.green * 255);
      blue = (unsigned int)(var.value.color.blue * 255);
      ys_printf(&ys, "#%x%x%x", red, green, blue);
      result->value.string = ys_string(ys);
      ys_del(&ys);
    }
  ys_del(&fs);
  ys_del(&ys);
  return (YENOERR);
}

/*
** cg_expr_f_bool()
** Cast a variable to boolean.
*/
yerr_t cg_expr_f_bool(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  result->type = CG_EXPR_BOOL;
  result->value.boolean = YTRUE;
  if (var.type == CG_EXPR_BOOL)
    result->value.boolean = var.value.boolean;
  else if (var.type == CG_EXPR_SCALAR)
    {
      if (var.value.scalar < 1.0 && var.value.scalar > -1.0)
	result->value.boolean = YFALSE;
    }
  else if (var.type == CG_EXPR_VALUE)
    {
      double tmp_scalar = yvalue_get(var.value.value, carta->default_unit);
      if (tmp_scalar < 1.0 && tmp_scalar > -1.0)
	result->value.boolean = YFALSE;
    }
  else if (var.type == CG_EXPR_STRING)
    {
      double tmp_scalar = atof(var.value.string);
      if (tmp_scalar < 1.0 && tmp_scalar > -1.0)
	result->value.boolean = YFALSE;
    }
  else if (var.type == CG_EXPR_ELEMENT)
    {
      if (!yvalue_get(var.value.element.width, carta->default_unit) ||
	  !yvalue_get(var.value.element.height, carta->default_unit))
	result->value.boolean = YFALSE;
    }
  else if (var.type == CG_EXPR_COLOR)
    {
      if (!var.value.color.red && !var.value.color.green &&
	  !var.value.color.blue)
	result->value.boolean = YFALSE;
    }
  return (YENOERR);
}

/*
** cg_expr_f_scalar()
** Cast a variable to scalar.
*/
yerr_t cg_expr_f_scalar(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  result->type = CG_EXPR_SCALAR;
  if (var.type == CG_EXPR_SCALAR)
    result->value.scalar = var.value.scalar;
  else if (var.type == CG_EXPR_VALUE)
    result->value.scalar = yvalue_get(var.value.value, carta->default_unit);
  else if (var.type == CG_EXPR_STRING)
    result->value.scalar = atof(var.value.string);
  else if (var.type == CG_EXPR_ELEMENT)
    result->value.scalar = yvalue_get(var.value.element.width,
				      carta->default_unit) *
      yvalue_get(var.value.element.height, carta->default_unit);
  else if (var.type == CG_EXPR_COLOR)
    result->value.scalar = (var.value.color.red + var.value.color.green +
			    var.value.color.blue) / 3;
  return (YENOERR);
}

/*
** cg_expr_f_value()
** Cast a variable to a value.
*/
yerr_t cg_expr_f_value(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type == CG_EXPR_SCALAR)
    {
      result->value.value.value = var.value.scalar;
      result->value.value.unit = carta->default_unit;
    }
  else if (var.type == CG_EXPR_VALUE)
    result->value.value = var.value.value;
  else if (var.type == CG_EXPR_STRING)
    result->value.value = yvalue_read(var.value.string, carta->default_unit);
  else if (var.type == CG_EXPR_ELEMENT || var.type == CG_EXPR_COLOR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  result->type = CG_EXPR_VALUE;
  return (YENOERR);
}

/*
** cg_expr_f_element()
** Cast a variable to an element.
*/
yerr_t cg_expr_f_element(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type == CG_EXPR_SCALAR || var.type == CG_EXPR_VALUE ||
      var.type == CG_EXPR_COLOR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  else if (var.type == CG_EXPR_STRING)
    {
      double f1, f2;
      char c1, c2, sunit[3] = {'\0', '\0', '\0'};
      if (sscanf(var.value.string, "%lfx%lf%c%c", &f1, &f2, &c1, &c2) == 4 &&
	  (((c1 == 'p' || c1 == 'P') && (c2 == 't' || c2 == 'T')) ||
	   ((c1 == 'm' || c1 == 'M') && (c2 == 'm' || c2 == 'M')) ||
	   ((c1 == 'c' || c1 == 'C') && (c2 == 'm' || c2 == 'M')) ||
	   ((c1 == 'i' || c1 == 'I') && (c2 == 'n' || c2 == 'N'))))
	{
	  sunit[0] = c1;
	  sunit[0] = c2;
	  result->value.element.width.value = f1;
	  result->value.element.height.value = f2;
	  result->value.element.width.unit =
	    result->value.element.width.unit =
	    (!strcasecmp(sunit, "pt") ? YUNIT_PT :
	     !strcasecmp(sunit, "mm") ? YUNIT_MM :
	     !strcasecmp(sunit, "cm") ? YUNIT_CM : YUNIT_IN);
	}
      else
	{
	  YLOG_ADD(YLOG_WARN, "Invalid string content.");
	  return (YEINVAL);
	}
    }
  else if (var.type == CG_EXPR_ELEMENT)
    {
      result->value.element.width = var.value.element.width;
      result->value.element.height = var.value.element.height;
    }
  result->type = CG_EXPR_ELEMENT;
  return (YENOERR);
}

/*
** cg_expr_f_color()
** Cast a variable to a color.
*/
yerr_t cg_expr_f_color(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var = {0};

  va_start(p_list, result);
  var = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var.type == CG_EXPR_SCALAR)
    {
      result->value.color.red = result->value.color.green =
	result->value.color.blue = var.value.scalar;
    }
  else if (var.type == CG_EXPR_COLOR)
    {
      result->value.color.red = var.value.color.red;
      result->value.color.green = var.value.color.green;
      result->value.color.blue = var.value.color.blue;
    }
  else if (var.type == CG_EXPR_STRING)
    {
      if (!var.value.string || var.value.string[0] != SHARP ||
	  strlen(var.value.string) < 7)
	YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      else
	{
	  unsigned int red, green, blue;
	  sscanf(var.value.string, "#%2x%2x%2x", &red, &green, &blue);
	  result->value.color.red = red;
	  result->value.color.green = green;
	  result->value.color.blue = blue;
	}
    }
  else if (var.type == CG_EXPR_VALUE || var.type == CG_EXPR_ELEMENT)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  result->type = CG_EXPR_COLOR;
  return (YENOERR);
}

/*
** cg_expr_f_min()
** Return the minimal of two values.
*/
yerr_t cg_expr_f_min(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type == CG_EXPR_STRING || var1.type == CG_EXPR_COLOR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid first parameter.");
      return (YEINVAL);
    }
  if (var1.type == CG_EXPR_SCALAR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->type = var1.type;
	  if (var1.value.scalar <= var2.value.scalar)
	    result->value.scalar = var1.value.scalar;
	  else
	    result->value.scalar = var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp_val = yvalue_get(var2.value.value, carta->default_unit);
	  if (var1.value.scalar <= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.scalar = var1.value.scalar;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.value = var2.value.value;
	    }
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  if (var1.value.scalar <= atof(var2.value.string))
	    {
	      result->type = var1.type;
	      result->value.scalar = var1.value.scalar;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.string = strdup(var2.value.string);
	    }
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (var1.value.scalar <= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.scalar = var1.value.scalar;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  if (var1.value.scalar <= ((var2.value.color.red +
				     var2.value.color.green +
				     var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.scalar = var1.value.scalar;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  double tmp_val = yvalue_get(var1.value.value, carta->default_unit);
	  if (tmp_val <= var2.value.scalar)
	    {
	      result->type = var1.type;
	      result->value.value = var1.value.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.scalar = var2.value.scalar;
	    }
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp1 = yvalue_get(var1.value.value, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.value, carta->default_unit);
	  result->type = var1.type;
	  if (tmp1 <= tmp2)
	    result->value.value = var1.value.value;
	  else
	    result->value.value = var2.value.value;
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  double tmp1 = yvalue_get(var1.value.value, carta->default_unit);
	  double tmp2 = atof(var2.value.string);
	  if (tmp1 <= tmp2)
	    {
	      result->type = var1.type;
	      result->value.value = var1.value.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.string = strdup(var2.value.string);
	    }
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp_val = yvalue_get(var1.value.value, carta->default_unit);
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (tmp_val <= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.value = var1.value.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  double tmp_val = yvalue_get(var1.value.value, carta->default_unit);
	  if (tmp_val <= ((var2.value.color.red +
			   var2.value.color.green +
			   var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.value = var1.value.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  else if (var1.type == CG_EXPR_STRING)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  if (atof(var1.value.string) <= var2.value.scalar)
	    {
	      result->type = var1.type;
	      result->value.string = strdup(var1.value.string);
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.scalar = var2.value.scalar;
	    }
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp_val = yvalue_get(var2.value.value, carta->default_unit);
	  if (atof(var1.value.string) <= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.string = strdup(var1.value.string);
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.value = var2.value.value;
	    }
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  result->type = var1.type;
	  if (strcmp(var1.value.string, var2.value.string) <= 0)
	    result->value.string = strdup(var1.value.string);
	  else
	    result->value.string = strdup(var2.value.string);
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp_val = atof(var1.value.string);
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (tmp_val <= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.string = strdup(var1.value.string);
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  double tmp_val = atof(var1.value.string);
	  if (tmp_val <= ((var2.value.color.red +
			   var2.value.color.green +
			   var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.string = strdup(var1.value.string);
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  else if (var1.type == CG_EXPR_ELEMENT)
    {
      double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
      double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
      double tmp_avg = (tmp1 + tmp2) / 2;
      if (var2.type == CG_EXPR_SCALAR)
	{
	  if (tmp_avg <= var2.value.scalar)
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.scalar = var2.value.scalar;
	    }
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp_val = yvalue_get(var2.value.value, carta->default_unit);
	  if (tmp_avg <= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.value = var2.value.value;
	    }
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  double tmp_val = atof(var2.value.string);
	  if (tmp_avg <= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.string = strdup(var2.value.string);
	    }
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (tmp_avg <= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  if (tmp_avg <= ((var2.value.color.red +
			   var2.value.color.green +
			   var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      double tmp_avg = (var1.value.color.red + var1.value.color.green +
			var1.value.color.blue) / 3;
      if (var2.type == CG_EXPR_SCALAR)
	{
	  if (tmp_avg <= var2.value.scalar)
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.scalar = var2.value.scalar;
	    }
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp_val = yvalue_get(var2.value.value, carta->default_unit);
	  if (tmp_avg <= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.value = var2.value.value;
	    }
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  double tmp_val = atof(var1.value.string);
	  if (tmp_avg <= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.string = strdup(var2.value.string);
	    }
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (tmp_avg <= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  if (tmp_avg <= ((var2.value.color.red +
			   var2.value.color.green +
			   var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  return (YENOERR);
}

/*
** cg_expr_f_max()
** Return the maximal of two values.
*/
yerr_t cg_expr_f_max(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type == CG_EXPR_STRING || var1.type == CG_EXPR_COLOR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid first parameter.");
      return (YEINVAL);
    }
  if (var1.type == CG_EXPR_SCALAR)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  result->type = var1.type;
	  if (var1.value.scalar >= var2.value.scalar)
	    result->value.scalar = var1.value.scalar;
	  else
	    result->value.scalar = var2.value.scalar;
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp_val = yvalue_get(var2.value.value, carta->default_unit);
	  if (var1.value.scalar >= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.scalar = var1.value.scalar;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.value = var2.value.value;
	    }
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  if (var1.value.scalar >= atof(var2.value.string))
	    {
	      result->type = var1.type;
	      result->value.scalar = var1.value.scalar;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.string = strdup(var2.value.string);
	    }
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (var1.value.scalar >= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.scalar = var1.value.scalar;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  if (var1.value.scalar >= ((var2.value.color.red +
				     var2.value.color.green +
				     var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.scalar = var1.value.scalar;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  double tmp_val = yvalue_get(var1.value.value, carta->default_unit);
	  if (tmp_val >= var2.value.scalar)
	    {
	      result->type = var1.type;
	      result->value.value = var1.value.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.scalar = var2.value.scalar;
	    }
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp1 = yvalue_get(var1.value.value, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.value, carta->default_unit);
	  result->type = var1.type;
	  if (tmp1 >= tmp2)
	    result->value.value = var1.value.value;
	  else
	    result->value.value = var2.value.value;
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  double tmp1 = yvalue_get(var1.value.value, carta->default_unit);
	  double tmp2 = atof(var2.value.string);
	  if (tmp1 >= tmp2)
	    {
	      result->type = var1.type;
	      result->value.value = var1.value.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.string = strdup(var2.value.string);
	    }
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp_val = yvalue_get(var1.value.value, carta->default_unit);
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (tmp_val >= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.value = var1.value.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  double tmp_val = yvalue_get(var1.value.value, carta->default_unit);
	  if (tmp_val >= ((var2.value.color.red +
			   var2.value.color.green +
			   var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.value = var1.value.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  else if (var1.type == CG_EXPR_STRING)
    {
      if (var2.type == CG_EXPR_SCALAR)
	{
	  if (atof(var1.value.string) >= var2.value.scalar)
	    {
	      result->type = var1.type;
	      result->value.string = strdup(var1.value.string);
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.scalar = var2.value.scalar;
	    }
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp_val = yvalue_get(var2.value.value, carta->default_unit);
	  if (atof(var1.value.string) >= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.string = strdup(var1.value.string);
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.value = var2.value.value;
	    }
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  result->type = var1.type;
	  if (strcmp(var1.value.string, var2.value.string) >= 0)
	    result->value.string = strdup(var1.value.string);
	  else
	    result->value.string = strdup(var2.value.string);
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp_val = atof(var1.value.string);
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (tmp_val >= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.string = strdup(var1.value.string);
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  double tmp_val = atof(var1.value.string);
	  if (tmp_val >= ((var2.value.color.red +
			   var2.value.color.green +
			   var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.string = strdup(var1.value.string);
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  else if (var1.type == CG_EXPR_ELEMENT)
    {
      double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
      double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
      double tmp_avg = (tmp1 + tmp2) / 2;
      if (var2.type == CG_EXPR_SCALAR)
	{
	  if (tmp_avg >= var2.value.scalar)
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.scalar = var2.value.scalar;
	    }
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp_val = yvalue_get(var2.value.value, carta->default_unit);
	  if (tmp_avg >= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.value = var2.value.value;
	    }
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  double tmp_val = atof(var2.value.string);
	  if (tmp_avg >= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.string = strdup(var2.value.string);
	    }
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (tmp_avg >= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  if (tmp_avg >= ((var2.value.color.red +
			   var2.value.color.green +
			   var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.element.width.unit = var1.value.element.width.unit;
	      result->value.element.height.unit = var1.value.element.height.unit;
	      result->value.element.width.value = var1.value.element.width.value;
	      result->value.element.height.value = var1.value.element.height.value;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      double tmp_avg = (var1.value.color.red + var1.value.color.green +
			var1.value.color.blue) / 3;
      if (var2.type == CG_EXPR_SCALAR)
	{
	  if (tmp_avg >= var2.value.scalar)
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.scalar = var2.value.scalar;
	    }
	}
      else if (var2.type == CG_EXPR_VALUE)
	{
	  double tmp_val = yvalue_get(var2.value.value, carta->default_unit);
	  if (tmp_avg >= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.value = var2.value.value;
	    }
	}
      else if (var2.type == CG_EXPR_STRING)
	{
	  double tmp_val = atof(var1.value.string);
	  if (tmp_avg >= tmp_val)
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.string = strdup(var2.value.string);
	    }
	}
      else if (var2.type == CG_EXPR_ELEMENT)
	{
	  double tmp1 = yvalue_get(var2.value.element.width, carta->default_unit);
	  double tmp2 = yvalue_get(var2.value.element.height, carta->default_unit);
	  if (tmp_avg >= ((tmp1 + tmp2) / 2))
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.element.width.unit = var2.value.element.width.unit;
	      result->value.element.height.unit = var2.value.element.height.unit;
	      result->value.element.width.value = var2.value.element.width.value;
	      result->value.element.height.value = var2.value.element.height.value;
	    }
	}
      else if (var2.type == CG_EXPR_COLOR)
	{
	  if (tmp_avg >= ((var2.value.color.red +
			   var2.value.color.green +
			   var2.value.color.blue) / 3))
	    {
	      result->type = var1.type;
	      result->value.color.red = var1.value.color.red;
	      result->value.color.green = var1.value.color.green;
	      result->value.color.blue = var1.value.color.blue;
	    }
	  else
	    {
	      result->type = var2.type;
	      result->value.color.red = var2.value.color.red;
	      result->value.color.green = var2.value.color.green;
	      result->value.color.blue = var2.value.color.blue;
	    }
	}
    }
  return (YENOERR);
}

/*
** cg_expr_f_strftime()
** Format a timestamp.
*/
yerr_t cg_expr_f_strftime(cg_t*carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t timestamp = {0}, format = {0};
  time_t the_time;
  struct tm st;
  char *str;

  va_start(p_list, result);
  timestamp = va_arg(p_list, cg_expr_var_t);
  format = va_arg(p_list, cg_expr_var_t);
  if (timestamp.type != CG_EXPR_SCALAR ||
      format.type != CG_EXPR_STRING)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  the_time = (time_t)timestamp.value.scalar;
  localtime_r(&the_time, &st);
  str = malloc0(256);
  strftime(str, 255, format.value.string, &st);
  result->type = CG_EXPR_STRING;
  result->value.string = strdup(str);
  free0(str);
  return (YENOERR);
}

/*
** cg_expr_f_time()
** Return the current timestamp.
*/
yerr_t cg_expr_f_time(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type != CG_EXPR_SCALAR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  result->type = CG_EXPR_SCALAR;
  result->value.scalar = (double)time(NULL) +
    (var1.value.scalar * 3600);
  return (YENOERR);
}

/*
** cg_expr_f_srand()
** Initialize random seed.
*/
yerr_t cg_expr_f_srand(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type != CG_EXPR_SCALAR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  srand((unsigned int)var1.value.scalar);
  result->type = CG_EXPR_SCALAR;
  result->value.scalar = (double)(rand() / (RAND_MAX + 1.0));
  return (YENOERR);
}

/*
** cg_expr_f_rand()
** Return a random scalar.
*/
yerr_t cg_expr_f_rand(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type != CG_EXPR_SCALAR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  result->type = CG_EXPR_SCALAR;
  result->value.scalar = (double)((var1.value.scalar * rand()) /
				  (RAND_MAX + 1.0));
  return (YENOERR);
}

/* *********** boolean functions ************* */

/*
** cg_expr_f_not()
** Inverse boolean value.
*/
yerr_t cg_expr_f_not(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  if (var1.type != CG_EXPR_BOOL)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter.");
      return (YEINVAL);
    }
  result->type = CG_EXPR_BOOL;
  result->value.boolean = YTRUE;
  if (var1.value.boolean)
    result->value.boolean = YFALSE;
  return (YENOERR);
}

/*
** cg_expr_f_and()
** Compute and boolean func of a variable by another.
*/
yerr_t cg_expr_f_and(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0}, bool1 = {0}, bool2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  result->type = CG_EXPR_BOOL;
  result->value.boolean = YTRUE;
  if (cg_expr_f_bool(carta, &bool1, var1) != YENOERR ||
      cg_expr_f_bool(carta, &bool2, var2) != YENOERR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter (or).");
      return (YEINVAL);
    }
  if (!bool1.value.boolean || !bool2.value.boolean)
    result->value.boolean = YFALSE;
  return (YENOERR);
}

/*
** cg_expr_f_or()
** Compute or boolean func of a variable by another.
*/
yerr_t cg_expr_f_or(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0}, bool1 = {0}, bool2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);
  result->type = CG_EXPR_BOOL;
  result->value.boolean = YTRUE;
  if (cg_expr_f_bool(carta, &bool1, var1) != YENOERR ||
      cg_expr_f_bool(carta, &bool2, var2) != YENOERR)
    {
      YLOG_ADD(YLOG_WARN, "Invalid parameter (or).");
      return (YEINVAL);
    }
  if (!bool1.value.boolean && !bool2.value.boolean)
    result->value.boolean = YFALSE;
  return (YENOERR);
}

/*
** cg_expr_f_equal()
** Compute comparison of a variable by another.
*/
yerr_t cg_expr_f_equal(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);

  if (var1.type != var2.type)
    {
      YLOG_ADD(YLOG_WARN, "Parameters with different types (equal).");
      return (YEINVAL);
    }
  result->type = CG_EXPR_BOOL;
  result->value.boolean = YTRUE;
  if (var1.type == CG_EXPR_SCALAR)
    {
      if (var1.value.scalar != var2.value.scalar)
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      double tmp_val1 = yvalue_get(var1.value.value, carta->default_unit);
      double tmp_val2 = yvalue_get(var2.value.value, carta->default_unit);
      if (tmp_val1 != tmp_val2)
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_STRING)
    {
      if (strcmp(var1.value.string, var2.value.string))
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_ELEMENT)
    {
      double tmp_v1[2], tmp_v2[2];

      tmp_v1[0] = yvalue_get(var1.value.element.width, carta->default_unit);
      tmp_v1[1] = yvalue_get(var1.value.element.height, carta->default_unit);
      tmp_v2[0] = yvalue_get(var2.value.element.width, carta->default_unit);
      tmp_v2[1] = yvalue_get(var2.value.element.height, carta->default_unit);
      if (tmp_v1[0] != tmp_v2[0] || tmp_v1[1] != tmp_v2[1])
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      if (var1.value.color.red != var2.value.color.red ||
	  var1.value.color.green != var2.value.color.green ||
	  var1.value.color.blue != var2.value.color.blue)
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_BOOL)
    {
      if (var1.value.boolean != var2.value.boolean)
	result->value.boolean = YFALSE;
    }
  return (YENOERR);
}

/*
** cg_expr_f_greater_than()
** Compute comparison of a variable by another.
*/
yerr_t cg_expr_f_greater_than(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);

  if (var1.type != var2.type)
    {
      YLOG_ADD(YLOG_WARN, "Parameters with different types (greater than).");
      return (YEINVAL);
    }
  result->type = CG_EXPR_BOOL;
  result->value.boolean = YTRUE;
  if (var1.type == CG_EXPR_SCALAR)
    {
      if (var1.value.scalar < var2.value.scalar)
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      double tmp_val1 = yvalue_get(var1.value.value, carta->default_unit);
      double tmp_val2 = yvalue_get(var2.value.value, carta->default_unit);
      if (tmp_val1 < tmp_val2)
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_STRING)
    {
      if (atof(var1.value.string) < atof(var2.value.string))
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_ELEMENT)
    {
      double tmp_v1[2], tmp_v2[2];

      tmp_v1[0] = yvalue_get(var1.value.element.width, carta->default_unit);
      tmp_v1[1] = yvalue_get(var1.value.element.height, carta->default_unit);
      tmp_v2[0] = yvalue_get(var2.value.element.width, carta->default_unit);
      tmp_v2[1] = yvalue_get(var2.value.element.height, carta->default_unit);
      if ((tmp_v1[0] * tmp_v1[1]) < (tmp_v2[0] * tmp_v2[1]))
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      double luminance1, luminance2;

      luminance1 = (var1.value.color.red * LUMINANCE_RED) +
	(var1.value.color.green * LUMINANCE_GREEN) +
	(var1.value.color.blue * LUMINANCE_BLUE);
      luminance2 = (var2.value.color.red * LUMINANCE_RED) +
	(var2.value.color.green * LUMINANCE_GREEN) +
	(var2.value.color.blue * LUMINANCE_BLUE);
      if (luminance1 < luminance2)
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_BOOL)
    {
      if (var1.value.boolean <= var2.value.boolean)
	result->value.boolean = YFALSE;
    }
  return (YENOERR);
}

/*
** cg_expr_f_less_than()
** Compute comparison of a variable by another.
*/
yerr_t cg_expr_f_less_than(cg_t *carta, cg_expr_var_t *result, ...)
{
  va_list p_list;
  cg_expr_var_t var1 = {0}, var2 = {0};

  va_start(p_list, result);
  var1 = va_arg(p_list, cg_expr_var_t);
  var2 = va_arg(p_list, cg_expr_var_t);
  va_end(p_list);

  if (var1.type != var2.type)
    {
      YLOG_ADD(YLOG_WARN, "Parameters with different types (less than).");
      return (YEINVAL);
    }
  result->type = CG_EXPR_BOOL;
  result->value.boolean = YTRUE;
  if (var1.type == CG_EXPR_SCALAR)
    {
      if (var1.value.scalar > var2.value.scalar)
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_VALUE)
    {
      double tmp_val1 = yvalue_get(var1.value.value, carta->default_unit);
      double tmp_val2 = yvalue_get(var2.value.value, carta->default_unit);
      if (tmp_val1 > tmp_val2)
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_STRING)
    {
      if (atof(var1.value.string) < atof(var2.value.string))
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_ELEMENT)
    {
      double tmp_v1[2], tmp_v2[2];

      tmp_v1[0] = yvalue_get(var1.value.element.width, carta->default_unit);
      tmp_v1[1] = yvalue_get(var1.value.element.height, carta->default_unit);
      tmp_v2[0] = yvalue_get(var2.value.element.width, carta->default_unit);
      tmp_v2[1] = yvalue_get(var2.value.element.height, carta->default_unit);
      if ((tmp_v1[0] * tmp_v1[1]) > (tmp_v2[0] * tmp_v2[1]))
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_COLOR)
    {
      double luminance1, luminance2;

      luminance1 = (var1.value.color.red * LUMINANCE_RED) +
	(var1.value.color.green * LUMINANCE_GREEN) +
	(var1.value.color.blue * LUMINANCE_BLUE);
      luminance2 = (var2.value.color.red * LUMINANCE_RED) +
	(var2.value.color.green * LUMINANCE_GREEN) +
	(var2.value.color.blue * LUMINANCE_BLUE);
      if (luminance1 > luminance2)
	result->value.boolean = YFALSE;
    }
  else if (var1.type == CG_EXPR_BOOL)
    {
      if (var1.value.boolean >= var2.value.boolean)
	result->value.boolean = YFALSE;
    }
  return (YENOERR);
}

/* ************************ expr management ******************** */

/*
** cg_expr_init()
** Initialization of expression parser.
*/
yerr_t cg_expr_init(cg_t *carta)
{
  cg_expr_t *expr;

  if (!carta)
    return (YEINVAL);
  if (!(expr = malloc0(sizeof(cg_expr_t))))
    return (YENOMEM);
  expr->precision = -1;
  carta->expr = expr;
  if (!(expr->vars = yv_new()))
    {
      free0(expr);
      return (YENOMEM);
    }
  if (!(expr->funcs = yv_new()))
    {
      yv_del(&expr->vars, NULL, NULL);
      free0(expr);
      return (YENOMEM);
    }
  cg_expr_set_bool(carta, "FALSE", YFALSE);
  cg_expr_set_bool(carta, "TRUE", YTRUE);
  cg_expr_set_scalar(carta, "DECK_TOTAL", (double)yv_len(carta->decks));
  cg_expr_set_scalar(carta, "PI", M_PI);
  cg_expr_set_color(carta, "BLACK", 0, 0, 0);
  cg_expr_set_color(carta, "WHITE", 1, 1, 1);
  cg_expr_set_color(carta, "GREY", 0.5, 0.5, 0.5);
  cg_expr_set_color(carta, "RED", 1, 0, 0);
  cg_expr_set_color(carta, "GREEN", 0, 1, 0);
  cg_expr_set_color(carta, "BLUE", 0, 0, 1);
  cg_expr_set_color(carta, "YELLOW", 1, 1, 0);
  cg_expr_set_color(carta, "TURQUOISE", 0, 1, 1);
  cg_expr_set_color(carta, "PURPLE", 1, 0, 1);
  cg_expr_set_color(carta, "DARK_GREY", 0.25, 0.25, 0.25);
  cg_expr_set_color(carta, "DARK_RED", 0.5, 0, 0);
  cg_expr_set_color(carta, "DARK_GREEN", 0, 0.5, 0);
  cg_expr_set_color(carta, "DARK_BLUE", 0, 0, 0.5);
  cg_expr_set_color(carta, "DARK_YELLOW", 0.5, 0.5, 0);
  cg_expr_set_color(carta, "DARK_TURQUOISE", 0, 0.5, 0.5);
  cg_expr_set_color(carta, "DARK_PURPLE", 0.5, 0, 0.5);
  cg_expr_set_color(carta, "LIGHT_GREY", 0.75, 0.75, 0.75);
  cg_expr_set_color(carta, "LIGHT_RED", 1, 0.5, 0.5);
  cg_expr_set_color(carta, "LIGHT_GREEN", 0.5, 1, 0.5);
  cg_expr_set_color(carta, "LIGHT_BLUE", 0.5, 0.5, 1);
  cg_expr_set_color(carta, "LIGHT_YELLOW", 1, 1, 0.5);
  cg_expr_set_color(carta, "LIGHT_TURQUOISE", 0.5, 1, 1);
  cg_expr_set_color(carta, "LIGHT_PURPLE", 1, 0.5, 1);
  cg_expr_set_element(carta, "4A0", cg_get_paper_width(FOUR_A0), cg_get_paper_height(FOUR_A0));
  cg_expr_set_element(carta, "2A0", cg_get_paper_width(TWO_A0), cg_get_paper_height(TWO_A0));
  cg_expr_set_element(carta, "A0", cg_get_paper_width(A0), cg_get_paper_height(A0));
  cg_expr_set_element(carta, "A1", cg_get_paper_width(A1), cg_get_paper_height(A1));
  cg_expr_set_element(carta, "A2", cg_get_paper_width(A2), cg_get_paper_height(A2));
  cg_expr_set_element(carta, "A3", cg_get_paper_width(A3), cg_get_paper_height(A3));
  cg_expr_set_element(carta, "A4", cg_get_paper_width(A4), cg_get_paper_height(A4));
  cg_expr_set_element(carta, "A5", cg_get_paper_width(A5), cg_get_paper_height(A5));
  cg_expr_set_element(carta, "A6", cg_get_paper_width(A6), cg_get_paper_height(A6));
  cg_expr_set_element(carta, "A7", cg_get_paper_width(A7), cg_get_paper_height(A7));
  cg_expr_set_element(carta, "A8", cg_get_paper_width(A8), cg_get_paper_height(A8));
  cg_expr_set_element(carta, "A9", cg_get_paper_width(A9), cg_get_paper_height(A9));
  cg_expr_set_element(carta, "A10", cg_get_paper_width(A10), cg_get_paper_height(A10));
  cg_expr_set_element(carta, "RA0", cg_get_paper_width(RA0), cg_get_paper_height(RA0));
  cg_expr_set_element(carta, "RA1", cg_get_paper_width(RA1), cg_get_paper_height(RA1));
  cg_expr_set_element(carta, "RA2", cg_get_paper_width(RA2), cg_get_paper_height(RA2));
  cg_expr_set_element(carta, "RA3", cg_get_paper_width(RA3), cg_get_paper_height(RA3));
  cg_expr_set_element(carta, "RA4", cg_get_paper_width(RA4), cg_get_paper_height(RA4));
  cg_expr_set_element(carta, "SRA0", cg_get_paper_width(SRA0), cg_get_paper_height(SRA0));
  cg_expr_set_element(carta, "SRA1", cg_get_paper_width(SRA1), cg_get_paper_height(SRA1));
  cg_expr_set_element(carta, "SRA2", cg_get_paper_width(SRA2), cg_get_paper_height(SRA2));
  cg_expr_set_element(carta, "SRA3", cg_get_paper_width(SRA3), cg_get_paper_height(SRA3));
  cg_expr_set_element(carta, "SRA4", cg_get_paper_width(SRA4), cg_get_paper_height(SRA4));
  cg_expr_set_element(carta, "B0", cg_get_paper_width(B0), cg_get_paper_height(B0));
  cg_expr_set_element(carta, "B1", cg_get_paper_width(B1), cg_get_paper_height(B1));
  cg_expr_set_element(carta, "B2", cg_get_paper_width(B2), cg_get_paper_height(B2));
  cg_expr_set_element(carta, "B3", cg_get_paper_width(B3), cg_get_paper_height(B3));
  cg_expr_set_element(carta, "B4", cg_get_paper_width(B4), cg_get_paper_height(B4));
  cg_expr_set_element(carta, "B5", cg_get_paper_width(B5), cg_get_paper_height(B5));
  cg_expr_set_element(carta, "B6", cg_get_paper_width(B6), cg_get_paper_height(B6));
  cg_expr_set_element(carta, "B7", cg_get_paper_width(B7), cg_get_paper_height(B7));
  cg_expr_set_element(carta, "B8", cg_get_paper_width(B8), cg_get_paper_height(B8));
  cg_expr_set_element(carta, "B9", cg_get_paper_width(B9), cg_get_paper_height(B9));
  cg_expr_set_element(carta, "B10", cg_get_paper_width(B10), cg_get_paper_height(B10));
  cg_expr_set_element(carta, "C0", cg_get_paper_width(C0), cg_get_paper_height(C0));
  cg_expr_set_element(carta, "C1", cg_get_paper_width(C1), cg_get_paper_height(C1));
  cg_expr_set_element(carta, "C2", cg_get_paper_width(C2), cg_get_paper_height(C2));
  cg_expr_set_element(carta, "C3", cg_get_paper_width(C3), cg_get_paper_height(C3));
  cg_expr_set_element(carta, "C4", cg_get_paper_width(C4), cg_get_paper_height(C4));
  cg_expr_set_element(carta, "C5", cg_get_paper_width(C5), cg_get_paper_height(C5));
  cg_expr_set_element(carta, "C6", cg_get_paper_width(C6), cg_get_paper_height(C6));
  cg_expr_set_element(carta, "C7", cg_get_paper_width(C7), cg_get_paper_height(C7));
  cg_expr_set_element(carta, "C8", cg_get_paper_width(C8), cg_get_paper_height(C8));
  cg_expr_set_element(carta, "C9", cg_get_paper_width(C9), cg_get_paper_height(C9));
  cg_expr_set_element(carta, "C10", cg_get_paper_width(C10), cg_get_paper_height(C10));
  cg_expr_set_element(carta, "D0", cg_get_paper_width(D0), cg_get_paper_height(D0));
  cg_expr_set_element(carta, "D1", cg_get_paper_width(D1), cg_get_paper_height(D1));
  cg_expr_set_element(carta, "D2", cg_get_paper_width(D2), cg_get_paper_height(D2));
  cg_expr_set_element(carta, "D3", cg_get_paper_width(D3), cg_get_paper_height(D3));
  cg_expr_set_element(carta, "D4", cg_get_paper_width(D4), cg_get_paper_height(D4));
  cg_expr_set_element(carta, "D5", cg_get_paper_width(D5), cg_get_paper_height(D5));
  cg_expr_set_element(carta, "D6", cg_get_paper_width(D6), cg_get_paper_height(D6));
  cg_expr_set_element(carta, "D7", cg_get_paper_width(D7), cg_get_paper_height(D7));
  cg_expr_set_element(carta, "D8", cg_get_paper_width(D8), cg_get_paper_height(D8));
  cg_expr_set_element(carta, "D9", cg_get_paper_width(D9), cg_get_paper_height(D9));
  cg_expr_set_element(carta, "D10", cg_get_paper_width(D10), cg_get_paper_height(D10));
  cg_expr_set_element(carta, "LETTER", cg_get_paper_width(LETTER), cg_get_paper_height(LETTER));
  cg_expr_set_element(carta, "LEGAL", cg_get_paper_width(LEGAL), cg_get_paper_height(LEGAL));
  cg_expr_set_element(carta, "LEDGER", cg_get_paper_width(LEDGER), cg_get_paper_height(LEDGER));
  cg_expr_set_element(carta, "SEMI_LETTER", cg_get_paper_width(SEMI_LETTER), cg_get_paper_height(SEMI_LETTER));
  cg_expr_set_element(carta, "EXECUTIVE", cg_get_paper_width(EXECUTIVE), cg_get_paper_height(EXECUTIVE));
  cg_expr_set_element(carta, "TABLOID", cg_get_paper_width(TABLOID), cg_get_paper_height(TABLOID));
  cg_expr_set_element(carta, "DL", cg_get_paper_width(DL), cg_get_paper_height(DL));
  cg_expr_set_element(carta, "COM10", cg_get_paper_width(COM10), cg_get_paper_height(COM10));
  cg_expr_set_element(carta, "MONARCH", cg_get_paper_width(MONARCH), cg_get_paper_height(MONARCH));
  cg_expr_set_element(carta, "E0", cg_get_paper_width(E0), cg_get_paper_height(E0));
  cg_expr_set_element(carta, "E1", cg_get_paper_width(E1), cg_get_paper_height(E1));
  cg_expr_set_element(carta, "E2", cg_get_paper_width(E2), cg_get_paper_height(E2));
  cg_expr_set_element(carta, "E3", cg_get_paper_width(E3), cg_get_paper_height(E3));
  cg_expr_set_element(carta, "E4", cg_get_paper_width(E4), cg_get_paper_height(E4));
  cg_expr_set_element(carta, "E5", cg_get_paper_width(E5), cg_get_paper_height(E5));
  cg_expr_set_element(carta, "E6", cg_get_paper_width(E6), cg_get_paper_height(E6));
  cg_expr_set_element(carta, "E7", cg_get_paper_width(E7), cg_get_paper_height(E7));
  cg_expr_set_element(carta, "E8", cg_get_paper_width(E8), cg_get_paper_height(E8));
  cg_expr_set_element(carta, "E9", cg_get_paper_width(E9), cg_get_paper_height(E9));
  cg_expr_set_element(carta, "E10", cg_get_paper_width(E10), cg_get_paper_height(E10));
  cg_expr_set_element(carta, "E_A3", cg_get_paper_width(E_A3), cg_get_paper_height(E_A3));
  cg_expr_set_element(carta, "E_A3_2", cg_get_paper_width(E_A3_2), cg_get_paper_height(E_A3_2));
  cg_expr_set_element(carta, "E_A4", cg_get_paper_width(E_A4), cg_get_paper_height(E_A4));
  cg_expr_set_element(carta, "E_A4_2", cg_get_paper_width(E_A4_2), cg_get_paper_height(E_A4_2));
  cg_expr_set_element(carta, "E_A5", cg_get_paper_width(E_A5), cg_get_paper_height(E_A5));
  cg_expr_set_element(carta, "E_A5_2", cg_get_paper_width(E_A5_2), cg_get_paper_height(E_A5_2));
  cg_expr_set_element(carta, "E_A6", cg_get_paper_width(E_A6), cg_get_paper_height(E_A6));
  cg_expr_set_element(carta, "E_A7", cg_get_paper_width(E_A7), cg_get_paper_height(E_A7));
  cg_expr_set_element(carta, "E_B4", cg_get_paper_width(E_B4), cg_get_paper_height(E_B4));
  cg_expr_set_element(carta, "E_B4_2", cg_get_paper_width(E_B4_2), cg_get_paper_height(E_B4_2));
  cg_expr_set_element(carta, "E_B5", cg_get_paper_width(E_B5), cg_get_paper_height(E_B5));
  cg_expr_set_element(carta, "E_B6", cg_get_paper_width(E_B6), cg_get_paper_height(E_B6));
  cg_expr_set_element(carta, "E_B7", cg_get_paper_width(E_B7), cg_get_paper_height(E_B7));
  cg_expr_set_element(carta, "E_B8", cg_get_paper_width(E_B8), cg_get_paper_height(E_B8));
  cg_expr_set_element(carta, "ID1", cg_get_paper_width(ID1), cg_get_paper_height(ID1));
  cg_expr_set_element(carta, "ID2", cg_get_paper_width(ID2), cg_get_paper_height(ID2));
  cg_expr_set_element(carta, "ID3", cg_get_paper_width(ID3), cg_get_paper_height(ID3));
  cg_expr_set_element(carta, "BUSINESS_CARD", cg_get_paper_width(BUSINESS_CARD), cg_get_paper_height(BUSINESS_CARD));
  cg_expr_set_func(carta, "var", 1, cg_expr_f_var);
  cg_expr_set_func(carta, "width", 1, cg_expr_f_width);
  cg_expr_set_func(carta, "height", 1, cg_expr_f_height);
  cg_expr_set_func(carta, "landscape", 1, cg_expr_f_landscape);
  cg_expr_set_func(carta, "plus", 2, cg_expr_f_plus);
  cg_expr_set_func(carta, "minus", 2, cg_expr_f_minus);
  cg_expr_set_func(carta, "mult", 2, cg_expr_f_mult);
  cg_expr_set_func(carta, "div", 2, cg_expr_f_div);
  cg_expr_set_func(carta, "mod", 2, cg_expr_f_mod);
  cg_expr_set_func(carta, "+", 2, cg_expr_f_plus);
  cg_expr_set_func(carta, "-", 2, cg_expr_f_minus);
  cg_expr_set_func(carta, "*", 2, cg_expr_f_mult);
  cg_expr_set_func(carta, "/", 2, cg_expr_f_div);
  cg_expr_set_func(carta, "%", 2, cg_expr_f_mod);
  cg_expr_set_func(carta, "&", 2, cg_expr_f_and);
  cg_expr_set_func(carta, "|", 2, cg_expr_f_or);
  cg_expr_set_func(carta, "=", 2, cg_expr_f_equal);
  cg_expr_set_func(carta, "<", 2, cg_expr_f_less_than);
  cg_expr_set_func(carta, ">", 2, cg_expr_f_greater_than);
  cg_expr_set_func(carta, "not", 1, cg_expr_f_not);
  cg_expr_set_func(carta, "and", 2, cg_expr_f_and);
  cg_expr_set_func(carta, "or", 2, cg_expr_f_or);
  cg_expr_set_func(carta, "eq", 2, cg_expr_f_equal);
  cg_expr_set_func(carta, "lt", 2, cg_expr_f_less_than);
  cg_expr_set_func(carta, "gt", 2, cg_expr_f_greater_than);
  cg_expr_set_func(carta, "sqrt", 1, cg_expr_f_sqrt);
  cg_expr_set_func(carta, "pow", 2, cg_expr_f_pow);
  cg_expr_set_func(carta, "strlen", 1, cg_expr_f_strlen);
  cg_expr_set_func(carta, "strwidth", 3, cg_expr_f_strwidth);
  cg_expr_set_func(carta, "sin", 1, cg_expr_f_sin);
  cg_expr_set_func(carta, "cos", 1, cg_expr_f_cos);
  cg_expr_set_func(carta, "tan", 1, cg_expr_f_tan);
  cg_expr_set_func(carta, "asin", 1, cg_expr_f_asin);
  cg_expr_set_func(carta, "acos", 1, cg_expr_f_acos);
  cg_expr_set_func(carta, "atan", 1, cg_expr_f_atan);
  cg_expr_set_func(carta, "string", 1, cg_expr_f_string);
  cg_expr_set_func(carta, "bool", 1, cg_expr_f_bool);
  cg_expr_set_func(carta, "scalar", 1, cg_expr_f_scalar);
  cg_expr_set_func(carta, "value", 1, cg_expr_f_value);
  cg_expr_set_func(carta, "element", 1, cg_expr_f_element);
  cg_expr_set_func(carta, "color", 1, cg_expr_f_color);
  cg_expr_set_func(carta, "min", 2, cg_expr_f_min);
  cg_expr_set_func(carta, "max", 2, cg_expr_f_max);
  cg_expr_set_func(carta, "strftime", 2, cg_expr_f_strftime);
  cg_expr_set_func(carta, "time", 1, cg_expr_f_time);
  cg_expr_set_func(carta, "srand", 1, cg_expr_f_srand);
  cg_expr_set_func(carta, "rand", 1, cg_expr_f_rand);
  return (YENOERR);
}

/*
** free_var()
** Free memory allocated for a variable.
*/
void free_var(void *var, void *data)
{
  cg_expr_var_t *v = var;

  if (!var)
    return ;
  free0(v->name);
  if (v->type == CG_EXPR_STRING && v->value.string)
    free0(v->value.string);
  if (!data)
    free0(v);
}

/*
** free_func()
** Free memory allocated for a function.
*/
void free_func(void *func, void *data)
{
  cg_expr_func_t *f = func;

  if (!func)
    return ;
  free0(f->name);
  if (!data)
    free0(f);
}
