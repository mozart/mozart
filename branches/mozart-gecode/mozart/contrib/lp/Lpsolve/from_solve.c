  /* weird piece of code. 

  if(lp->anti_degen)
    {
      memcpy(lp->upbo,  upbo,        (lp->sum + 1)  * sizeof(REAL));
      memcpy(lp->lowbo, lowbo,       (lp->sum + 1)  * sizeof(REAL));
      memcpy(lp->rh,    lp->orig_rh, (lp->rows + 1) * sizeof(REAL));

      for(i = 1; i <= lp->columns; i++)
        if((theta = lp->lowbo[lp->rows + i]) != 0)
          {
	    if(lp->upbo[lp->rows + i] < lp->infinite)
              lp->upbo[lp->rows + i] -= theta;
            for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++)
              lp->rh[lp->mat[j].row_nr] -= theta * lp->mat[j].value;
          }
      invert(lp);
      lp->eta_valid = TRUE;
      failure = solvelp(lp);
    }
    */

