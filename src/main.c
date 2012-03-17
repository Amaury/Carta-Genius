#include "cartagenius.h"

/* private prototype */
static void _usage(void);

int main(int argc, char **argv)
{
  int optch, return_value = 0;
  char *input = NULL, *output = NULL, *inc_paths = NULL;
  char *font_paths = NULL, *img_paths = NULL;
  cg_back_t do_back = BACK_NO;
  cg_paper_t do_paper = A4;
  yvalue_t do_width, do_height, do_margin_width, do_margin_height;
  ybool_t set_back = YFALSE, set_paper = YFALSE, set_pdf = YFALSE;
  ybool_t set_width = YFALSE, set_height = YFALSE, set_margin = YFALSE;
  ybool_t set_margin_width = YFALSE, set_margin_height = YFALSE;
  ybool_t do_reverse = YFALSE, set_reverse = YFALSE;
  ybool_t set_landscape = YFALSE, set_quiet = YFALSE;
  cg_pdf_version_t pdf_version = PDF_14;
  char *author = NULL, *title = NULL, *log_file = NULL;;
  cg_t *carta;

  /* parse the command line */
  while ((optch = getopt(argc, argv,
			 "p:w:h:l:m:x:y:b:v:r:i:o:d:e:t:L:q")) != -1)
    switch (optch)
      {
      case 'p': /* paper type */
	set_paper = YTRUE;
	do_paper = cg_get_paper(optarg);
	do_width = cg_get_paper_width(do_paper);
	do_height = cg_get_paper_height(do_paper);
	break ;
      case 'w': /* paper width */
	set_width = YTRUE;
	do_width = yvalue_read(optarg, YUNIT_MM);
	break ;
      case 'h': /* paper height */
	set_height = YTRUE;
	do_height = yvalue_read(optarg, YUNIT_MM);
	break ;
      case 'l': /* landscape mode */
	if (!strcasecmp(optarg, "yes") || !strcasecmp(optarg, "true"))
	  set_landscape = YTRUE;
	break ;
      case 'm': /* margin size */
	set_margin = YTRUE;
	do_margin_width = yvalue_read(optarg, YUNIT_MM);
	do_margin_height = do_margin_width;
	break ;
      case 'x': /* margin width */
	set_margin_width = YTRUE;
	do_margin_width = yvalue_read(optarg, YUNIT_MM);
	break ;
      case 'y': /* margin height */
	set_margin_height = YTRUE;
	do_margin_height = yvalue_read(optarg, YUNIT_MM);
	break ;
      case 'b': /* cards' back configuration */
	set_back = YTRUE;
	if (!strcasecmp(optarg, "width"))
	  do_back = BACK_WIDTH;
	else if (!strcasecmp(optarg, "height"))
	  do_back = BACK_HEIGHT;
	else if (!strcasecmp(optarg, "no"))
	  do_back = BACK_NO;
	else
	  set_back = YFALSE;
	break ;
      case 'v': /* PDF version */
	set_pdf = YTRUE;
	if (!strcmp(optarg, "1.3"))
	  pdf_version = PDF_13;
	else if (!strcmp(optarg, "1.4"))
	  pdf_version = PDF_14;
	else if (!strcmp(optarg, "1.5"))
	  pdf_version = PDF_15;
	else
	  set_pdf = YFALSE;
	break ;
      case 'r': /* inverse page order */
	if (!strcasecmp(optarg, "yes") || !strcasecmp(optarg, "true"))
	  {
	    set_reverse = YTRUE;
	    do_reverse = YTRUE;
	  }
	else if (!strcasecmp(optarg, "no") || !strcasecmp(optarg, "false"))
	  {
	    set_reverse = YTRUE;
	    do_reverse = YFALSE;
	  }
	break ;
      case 'd': /* include path directories */
	inc_paths = strdup(optarg);
	break ;
      case 'f': /* font path directories */
	font_paths = strdup(optarg);
	break ;
      case 'g': /* image path directories */
	img_paths = strdup(optarg);
	break ;
      case 'i': /* input file */
        input = strdup(optarg);
        break ;
      case 'o': /* output file */
        output = strdup(optarg);
        break ;
      case 'e': /* author */
	author = strdup(optarg);
	break ;
      case 't': /* title */
	title = strdup(optarg);
	break ;
      case 'L': /* log file */
	log_file = strdup(optarg);
	break ;
      case 'q': /* quiet mode */
	set_quiet = YTRUE;
	break ;
      default:
	_usage();
        exit(1);
      }

  if (!input || !output)
    {
      _usage();
      exit(1);
    }

  /* logging init */
  if (set_quiet && log_file)
    YLOG_INIT_FILE(log_file);
  else if (set_quiet && !log_file)
    YLOG_INIT_SYSLOG();
  else if (!set_quiet && log_file)
    YLOG_INIT_STDERR_FILE(log_file);
  else
    YLOG_INIT_STDERR();

  /* read input file */
  if (!(carta = cg_init(input, inc_paths, font_paths, img_paths)))
    {
      YLOG_ADD(YLOG_ERR, "Unable to read input file '%s'", input);
      exit(1);
    }
  /* update with command line options */
  if (set_back)
    carta->do_back = do_back;
  if (set_paper || set_width || set_height || set_landscape ||
      set_margin || set_margin_width || set_margin_height)
    {
      cg_deck_t *deck;
      int i;
      for (i = 0; i < yv_len(carta->decks); ++i)
	{
	  deck = carta->decks[i];
	  if (set_paper || set_width)
	    deck->paper_width = do_width;
	  if (set_paper || set_height)
	    deck->paper_height = do_height;
	  if (set_landscape)
	    {
	      yvalue_t tmp_val = deck->paper_width;
	      deck->paper_width = deck->paper_height;
	      deck->paper_height = tmp_val;
	    }
	  if (set_margin || set_margin_width)
	    deck->paper_margin_w = do_margin_width;
	  if (set_margin || set_margin_height)
	    deck->paper_margin_h = do_margin_height;
	}
    }
  if (set_pdf)
    carta->pdf_version = pdf_version;
  if (set_reverse)
    carta->reverse = do_reverse;
  if (author)
    {
      free0(carta->author);
      carta->author = author;
    }
  if (title)
    {
      free0(carta->title);
      carta->title = title;
    }

  /* compute cols and rows */
  if (cg_compute_cols_rows(carta) == YENOERR)
    {
      /* create the PDF file */
      if (cg_create_document(output, carta) != YENOERR)
	return_value = 2;
    }
  else
    return_value = 3;

  /* free memory */
  yv_del(&carta->xml_doms, free_dom, NULL);
  yv_del(&carta->decks, free_deck, NULL);
  yv_del(&carta->fonts, free_font, NULL);
  yv_del(&carta->images, free_image, NULL);
  yv_del(&carta->masks, free_image, NULL);
  yv_del(&carta->templates, NULL, NULL);
  yv_del(&carta->variables, NULL, NULL);
  yv_del(&carta->inc_paths, free_path, NULL);
  yv_del(&carta->img_paths, free_path, NULL);
  yv_del(&carta->text_defines, free_text_define, NULL);
  if (carta->expr)
    {
      yv_del(&carta->expr->vars, free_var, NULL);
      yv_del(&carta->expr->funcs, free_func, NULL);
      free0(carta->expr);
    }
  free0(carta->author);
  free0(carta->title);
  free0(carta->subject);
  free0(carta->keywords);
  free0(carta->copyright);
  free0(carta->version);
  free0(carta->language);
  free0(carta->note);
  free0(carta->master_password);
  free0(carta->user_password);
  free0(carta);
  return (return_value);
}

/*
** _usage()
** Show program's usage.
*/
static void _usage()
{
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "\tcartagenius -i file.xml -o file.pdf\n");
  fprintf(stderr, "Read documentation for more options.\n");
}
