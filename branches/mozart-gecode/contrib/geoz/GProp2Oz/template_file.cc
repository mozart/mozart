

OZ_BI_define($MOZART_FN_NAME$,$NUM_ARGUMENTS$,0)
{
  $DECLARE_SPACE$
  try{
    $CREATE_FUNCTION_CALLS$

      //std::cout << "imposed propagator" << std::endl;
      //std::cout << std::flush;
    return PROCEED;
  }
  catch (Gecode::Exception e) {
    return OZ_raiseC("prop: $MOZART_FN_NAME$",0);
  }
}
OZ_BI_end

