#include "cartagenius.h"

cg_paper_t cg_get_paper(const char *paper_type)
{
  if (!paper_type || !strlen(paper_type))
    {
      YLOG_ADD(YLOG_WARN, "No paper type. Set to A4.");
      return (A4);
    }
  if (!strcasecmp(paper_type, "4a0"))
    return (FOUR_A0);
  if (!strcasecmp(paper_type, "2a0"))
    return (TWO_A0);
  if (!strcasecmp(paper_type, "a0"))
    return (A0);
  if (!strcasecmp(paper_type, "a1"))
    return (A1);
  if (!strcasecmp(paper_type, "a2"))
    return (A2);
  if (!strcasecmp(paper_type, "a3"))
    return (A3);
  if (!strcasecmp(paper_type, "a4"))
    return (A4);
  if (!strcasecmp(paper_type, "a5"))
    return (A5);
  if (!strcasecmp(paper_type, "a6"))
    return (A6);
  if (!strcasecmp(paper_type, "a7"))
    return (A7);
  if (!strcasecmp(paper_type, "a8"))
    return (A8);
  if (!strcasecmp(paper_type, "a9"))
    return (A9);
  if (!strcasecmp(paper_type, "a10"))
    return (A10);
  if (!strcasecmp(paper_type, "ra0"))
    return (RA0);
  if (!strcasecmp(paper_type, "ra1"))
    return (RA1);
  if (!strcasecmp(paper_type, "ra2"))
    return (RA2);
  if (!strcasecmp(paper_type, "ra3"))
    return (RA3);
  if (!strcasecmp(paper_type, "ra4"))
    return (RA4);
  if (!strcasecmp(paper_type, "sra0"))
    return (SRA0);
  if (!strcasecmp(paper_type, "sra1"))
    return (SRA1);
  if (!strcasecmp(paper_type, "sra2"))
    return (SRA2);
  if (!strcasecmp(paper_type, "sra3"))
    return (SRA3);
  if (!strcasecmp(paper_type, "sra4"))
    return (SRA4);
  if (!strcasecmp(paper_type, "b0"))
    return (B0);
  if (!strcasecmp(paper_type, "b1"))
    return (B1);
  if (!strcasecmp(paper_type, "b2"))
    return (B2);
  if (!strcasecmp(paper_type, "b3"))
    return (B3);
  if (!strcasecmp(paper_type, "b4"))
    return (B4);
  if (!strcasecmp(paper_type, "b5"))
    return (B5);
  if (!strcasecmp(paper_type, "b6"))
    return (B6);
  if (!strcasecmp(paper_type, "b7"))
    return (B7);
  if (!strcasecmp(paper_type, "b8"))
    return (B8);
  if (!strcasecmp(paper_type, "b9"))
    return (B9);
  if (!strcasecmp(paper_type, "b10"))
    return (B10);
  if (!strcasecmp(paper_type, "c0"))
    return (C0);
  if (!strcasecmp(paper_type, "c1"))
    return (C1);
  if (!strcasecmp(paper_type, "c2"))
    return (C2);
  if (!strcasecmp(paper_type, "c3"))
    return (C3);
  if (!strcasecmp(paper_type, "c4"))
    return (C4);
  if (!strcasecmp(paper_type, "c5"))
    return (C5);
  if (!strcasecmp(paper_type, "c6"))
    return (C6);
  if (!strcasecmp(paper_type, "c7"))
    return (C7);
  if (!strcasecmp(paper_type, "c8"))
    return (C8);
  if (!strcasecmp(paper_type, "c9"))
    return (C9);
  if (!strcasecmp(paper_type, "c10"))
    return (C10);
  if (!strcasecmp(paper_type, "d0"))
    return (D0);
  if (!strcasecmp(paper_type, "d1"))
    return (D1);
  if (!strcasecmp(paper_type, "d2"))
    return (D2);
  if (!strcasecmp(paper_type, "d3"))
    return (D3);
  if (!strcasecmp(paper_type, "d4"))
    return (D4);
  if (!strcasecmp(paper_type, "d5"))
    return (D5);
  if (!strcasecmp(paper_type, "d6"))
    return (D6);
  if (!strcasecmp(paper_type, "d7"))
    return (D7);
  if (!strcasecmp(paper_type, "d8"))
    return (D8);
  if (!strcasecmp(paper_type, "d9"))
    return (D9);
  if (!strcasecmp(paper_type, "d10"))
    return (D10);
  if (!strcasecmp(paper_type, "letter"))
    return (LETTER);
  if (!strcasecmp(paper_type, "legal"))
    return (LEGAL);
  if (!strcasecmp(paper_type, "ledger"))
    return (LEDGER);
  if (!strcasecmp(paper_type, "semi-letter"))
    return (SEMI_LETTER);
  if (!strcasecmp(paper_type, "executive"))
    return (EXECUTIVE);
  if (!strcasecmp(paper_type, "tabloid"))
    return (TABLOID);
  if (!strcasecmp(paper_type, "dl"))
    return (DL);
  if (!strcasecmp(paper_type, "com10"))
    return (COM10);
  if (!strcasecmp(paper_type, "monarch"))
    return (MONARCH);
  if (!strcasecmp(paper_type, "e0"))
    return (E0);
  if (!strcasecmp(paper_type, "e1"))
    return (E1);
  if (!strcasecmp(paper_type, "e2"))
    return (E2);
  if (!strcasecmp(paper_type, "e3"))
    return (E3);
  if (!strcasecmp(paper_type, "e4"))
    return (E4);
  if (!strcasecmp(paper_type, "e5"))
    return (E5);
  if (!strcasecmp(paper_type, "e6"))
    return (E6);
  if (!strcasecmp(paper_type, "e7"))
    return (E7);
  if (!strcasecmp(paper_type, "e8"))
    return (E8);
  if (!strcasecmp(paper_type, "e9"))
    return (E9);
  if (!strcasecmp(paper_type, "e10"))
    return (E10);
  if (!strcasecmp(paper_type, "e-a3"))
    return (E_A3);
  if (!strcasecmp(paper_type, "e-a3/2"))
    return (E_A3_2);
  if (!strcasecmp(paper_type, "e-a4"))
    return (E_A4);
  if (!strcasecmp(paper_type, "e-a4/2"))
    return (E_A4_2);
  if (!strcasecmp(paper_type, "e-a5"))
    return (E_A5);
  if (!strcasecmp(paper_type, "e-a5/2"))
    return (E_A5_2);
  if (!strcasecmp(paper_type, "e-a6"))
    return (E_A6);
  if (!strcasecmp(paper_type, "e-a7"))
    return (E_A7);
  if (!strcasecmp(paper_type, "e-b4"))
    return (E_B4);
  if (!strcasecmp(paper_type, "e-b4/2"))
    return (E_B4_2);
  if (!strcasecmp(paper_type, "e-b5"))
    return (E_B5);
  if (!strcasecmp(paper_type, "e-b6"))
    return (E_B6);
  if (!strcasecmp(paper_type, "e-b7"))
    return (E_B7);
  if (!strcasecmp(paper_type, "e-b8"))
    return (E_B8);
  if (!strcasecmp(paper_type, "id1"))
    return (ID1);
  if (!strcasecmp(paper_type, "id2"))
    return (ID2);
  if (!strcasecmp(paper_type, "id3"))
    return (ID3);
  if (!strcasecmp(paper_type, "business-card"))
    return (BUSINESS_CARD);
  YLOG_ADD(YLOG_WARN, "Unknown paper (). Set to A4.",
	   paper_type);
  return (A4);
}

yvalue_t cg_get_paper_width(cg_paper_t paper)
{
  yvalue_t res;

  res.unit = YUNIT_MM;
  switch (paper)
    {
    case FOUR_A0:
      res.value = 1682.0;
      break ;
    case TWO_A0:
      res.value = 1189.0;
      break ;
    case A0:
      res.value = 841.0;
      break ;
    case A1:
      res.value = 594.0;
      break ;
    case A2:
      res.value = 420.0;
      break ;
    case A3:
      res.value = 297.0;
      break ;
    case A5:
      res.value = 148.0;
      break ;
    case A6:
      res.value = 105.0;
      break ;
    case A7:
      res.value = 74.0;
      break ;
    case A8:
      res.value = 52.0;
      break ;
    case A9:
      res.value = 37.0;
      break ;
    case A10:
      res.value = 26.0;
      break ;
    case RA0:
      res.value = 860.0;
      break ;
    case RA1:
      res.value = 610.0;
      break ;
    case RA2:
      res.value = 430.0;
      break ;
    case RA3:
      res.value = 305.0;
      break ;
    case RA4:
      res.value = 215.0;
      break ;
    case SRA0:
      res.value = 900.0;
      break ;
    case SRA1:
      res.value = 640.0;
      break ;
    case SRA2:
      res.value = 450.0;
      break ;
    case SRA3:
      res.value = 320.0;
      break ;
    case SRA4:
      res.value = 225.0;
      break ;
    case B0:
      res.value = 1000.0;
      break ;
    case B1:
      res.value = 707.0;
      break ;
    case B2:
      res.value = 500.0;
      break ;
    case B3:
      res.value = 353.0;
      break ;
    case B4:
      res.value = 250.0;
      break ;
    case B5:
      res.value = 176.0;
      break ;
    case B6:
      res.value = 125.0;
      break ;
    case B7:
      res.value = 88.0;
      break ;
    case B8:
      res.value = 62.0;
      break ;
    case B9:
      res.value = 44.0;
      break ;
    case B10:
      res.value = 31.0;
      break ;
    case C0:
      res.value = 917.0;
      break ;
    case C1:
      res.value = 648.0;
      break ;
    case C2:
      res.value = 458.0;
      break ;
    case C3:
      res.value = 324.0;
      break ;
    case C4:
      res.value = 229.0;
      break ;
    case C5:
      res.value = 162.0;
      break ;
    case C6:
      res.value = 114.0;
      break ;
    case C7:
      res.value = 81.0;
      break ;
    case C8:
      res.value = 57.0;
      break ;
    case C9:
      res.value = 40.0;
      break ;
    case C10:
      res.value = 28.0;
      break ;
    case D0:
      res.value = 779.0;
      break ;
    case D1:
      res.value = 545.0;
      break ;
    case D2:
      res.value = 385.0;
      break ;
    case D3:
      res.value = 272.0;
      break ;
    case D4:
      res.value = 192.0;
      break ;
    case D5:
      res.value = 136.0;
      break ;
    case D6:
      res.value = 96.0;
      break ;
    case D7:
      res.value = 68.0;
      break ;
    case D8:
      res.value = 48.0;
      break ;
    case D9:
      res.value = 34.0;
      break ;
    case D10:
      res.value = 24.0;
      break ;
    case LETTER:
      res.unit = YUNIT_IN;
      res.value = 8.5;
      break ;
    case LEGAL:
      res.unit = YUNIT_IN;
      res.value = 8.5;
      break ;
    case LEDGER:
      res.unit = YUNIT_IN;
      res.value = 11.0;
      break ;
    case SEMI_LETTER:
      res.unit = YUNIT_IN;
      res.value = 5.5;
      break ;
    case EXECUTIVE:
      res.unit = YUNIT_IN;
      res.value = 7.25;
      break ;
    case TABLOID:
      res.value = 279.4;
      break ;
    case DL:
      res.value = 110.0;
      break ;
    case COM10:
      res.value = 104.8;
      break ;
    case MONARCH:
      res.value = 98.4;
      break ;
    case E0:
      res.value = 1120.0;
      break ;
    case E1:
      res.value = 800.0;
      break ;
    case E2:
      res.value = 560.0;
      break ;
    case E3:
      res.value = 400.0;
      break ;
    case E4:
      res.value = 280.0;
      break ;
    case E5:
      res.value = 200.0;
      break ;
    case E6:
      res.value = 140.0;
      break ;
    case E7:
      res.value = 100.0;
      break ;
    case E8:
      res.value = 70.0;
      break ;
    case E9:
      res.value = 50.0;
      break ;
    case E10:
      res.value = 35.0;
      break ;
    case E_A3:
      res.value = 312.0;
      break;
    case E_A3_2:
      res.value = 156.0;
      break ;
    case E_A4:
      res.value = 220.0;
      break ;
    case E_A4_2:
      res.value = 115.0;
      break ;
    case E_A5:
      res.value = 156.0;
      break ;
    case E_A5_2:
      res.value = 110.0;
      break ;
    case E_A6:
      res.value = 110.0;
      break ;
    case E_A7:
      res.value = 78.0;
      break ;
    case E_B4:
      res.value = 262.0;
      break ;
    case E_B4_2:
      res.value = 131.0;
      break ;
    case E_B5:
      res.value = 185.0;
      break ;
    case E_B6:
      res.value = 131.0;
      break ;
    case E_B7:
      res.value = 92.0;
      break ;
    case E_B8:
      res.value = 65.0;
      break ;
    case ID1:
      res.value = 53.98;
      break ;
    case ID2:
      res.value = 74.0;
      break ;
    case ID3:
      res.value = 88.0;
      break ;
    case BUSINESS_CARD:
      res.value = 51.0;
      break ;
    case A4:
    default:
      res.value = 210.0;
    }
  return (res);
}

yvalue_t cg_get_paper_height(cg_paper_t paper)
{
  yvalue_t res;

  res.unit = YUNIT_MM;
  switch (paper)
    {
    case FOUR_A0:
      res.value = 2378.0;
      break ;
    case TWO_A0:
      res.value = 1682.0;
      break ;
    case A0:
      res.value = 1189.0;
      break ;
    case A1:
      res.value = 841.0;
      break ;
    case A2:
      res.value = 594.0;
      break ;
    case A3:
      res.value = 420.0;
      break ;
    case A5:
      res.value = 210.0;
      break ;
    case A6:
      res.value = 148.0;
      break ;
    case A7:
      res.value = 105.0;
      break ;
    case A8:
      res.value = 74.0;
      break ;
    case A9:
      res.value = 52.0;
      break ;
    case A10:
      res.value = 37.0;
      break ;
    case RA0:
      res.value = 1120.0;
      break ;
    case RA1:
      res.value = 860.0;
      break ;
    case RA2:
      res.value = 610.0;
      break ;
    case RA3:
      res.value = 430.0;
      break ;
    case RA4:
      res.value = 305.0;
      break ;
    case SRA0:
      res.value = 1280.0;
      break;
    case SRA1:
      res.value = 900.0;
      break ;
    case SRA2:
      res.value = 640.0;
      break;
    case SRA3:
      res.value = 450.0;
      break;
    case SRA4:
      res.value = 320.0;
      break;
    case B0:
      res.value = 1414.0;
      break ;
    case B1:
      res.value = 1000.0;
      break ;
    case B2:
      res.value = 707.0;
      break ;
    case B3:
      res.value = 500.0;
      break ;
    case B4:
      res.value = 353.0;
      break ;
    case B5:
      res.value = 250.0;
      break ;
    case B6:
      res.value = 176.0;
      break ;
    case B7:
      res.value = 125.0;
      break ;
    case B8:
      res.value = 88.0;
      break ;
    case B9:
      res.value = 62.0;
      break ;
    case B10:
      res.value = 44.0;
      break ;
    case C0:
      res.value = 1297.0;
      break ;
    case C1:
      res.value = 917.0;
      break ;
    case C2:
      res.value = 648.0;
      break ;
    case C3:
      res.value = 458.0;
      break ;
    case C4:
      res.value = 324.0;
      break ;
    case C5:
      res.value = 229.0;
      break ;
    case C6:
      res.value = 162.0;
      break ;
    case C7:
      res.value = 114.0;
      break ;
    case C8:
      res.value = 81.0;
      break ;
    case C9:
      res.value = 57.0;
      break ;
    case C10:
      res.value = 40.0;
      break ;
    case D0:
      res.value = 1090.0;
      break ;
    case D1:
      res.value = 779.0;
      break ;
    case D2:
      res.value = 545.0;
      break ;
    case D3:
      res.value = 385.0;
      break ;
    case D4:
      res.value = 272.0;
      break ;
    case D5:
      res.value = 192.0;
      break ;
    case D6:
      res.value = 136.0;
      break ;
    case D7:
      res.value = 96.0;
      break ;
    case D8:
      res.value = 68.0;
      break ;
    case D9:
      res.value = 48.0;
      break ;
    case D10:
      res.value = 34.0;
      break ;
    case LETTER:
      res.unit = YUNIT_IN;
      res.value = 11.0;
      break ;
    case LEGAL:
      res.unit = YUNIT_IN;
      res.value = 14.0;
      break ;
    case LEDGER:
      res.unit = YUNIT_IN;
      res.value = 17.0;
      break ;
    case SEMI_LETTER:
      res.unit = YUNIT_IN;
      res.value = 8.5;
      break ;
    case EXECUTIVE:
      res.unit = YUNIT_IN;
      res.value = 10.5;
      break ;
    case TABLOID:
      res.value = 431.8;
      break ;
    case DL:
      res.value = 220.0;
      break ;
    case COM10:
      res.value = 241.3;
      break ;
    case MONARCH:
      res.value = 190.5;
      break ;
    case E0:
      res.value = 1600.0;
      break ;
    case E1:
      res.value = 1120.0;
      break ;
    case E2:
      res.value = 800.0;
      break ;
    case E3:
      res.value = 560.0;
      break ;
    case E4:
      res.value = 400.0;
      break ;
    case E5:
      res.value = 280.0;
      break ;
    case E6:
      res.value = 200.0;
      break ;
    case E7:
      res.value = 140.0;
      break ;
    case E8:
      res.value = 100.0;
      break ;
    case E9:
      res.value = 70.0;
      break ;
    case E10:
      res.value = 50.0;
      break ;
    case E_A3:
      res.value = 441.0;
      break ;
    case E_A3_2:
      res.value = 441.0;
      break ;
    case E_A4:
      res.value = 312.0;
      break ;
    case E_A4_2:
      res.value = 312.0;
      break ;
    case E_A5:
      res.value = 220.0;
      break ;
    case E_A5_2:
      res.value = 220.0;
      break ;
    case E_A6:
      res.value = 156.0;
      break ;
    case E_A7:
      res.value = 110.0;
      break ;
    case E_B4:
      res.value = 371.0;
      break ;
    case E_B4_2:
      res.value = 371.0;
      break ;
    case E_B5:
      res.value = 262.0;
      break ;
    case E_B6:
      res.value = 185.0;
      break ;
    case E_B7:
      res.value = 131.0;
      break ;
    case E_B8:
      res.value = 92.0;
      break ;
    case ID1:
      res.value = 85.6;
      break ;
    case ID2:
      res.value = 105.0;
      break ;
    case ID3:
      res.value = 125.0;
      break ;
    case BUSINESS_CARD:
      res.value = 90.0;
      break ;
    case A4:
    default:
      res.value = 297.0;
    }
  return (res);
}
