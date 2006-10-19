
#if !defined(__SYMBOLS_CONSTANTS_INCLUDE__)
#define __SYMBOLS_CONSTANTS_INCLUDE__


//#define USE_STANDARD_SYMBOLS  
#if defined(USE_STANDARD_SYMBOLS)
  namespace {
    // local constants
    const std::string SCOPE_SOLVER    = "::";
    const std::string TEMPLATE_LEFT   = "<";
    const std::string TEMPLATE_RIGHT  = ">";
    const std::string WORD_CONST      = " const";
    const std::string WORD_POINTER    = "*";
    const std::string WORD_REFERENCE  = "&";
  }
#else   // defined(USE_STANDARD_SYMBOLS)
  namespace {
    // local constants
    const std::string SCOPE_SOLVER    = ".";
    const std::string TEMPLATE_LEFT   = "{";
    const std::string TEMPLATE_RIGHT  = "}";
    const std::string WORD_CONST      = " CONST";
    const std::string WORD_POINTER    = " POINTER";
    const std::string WORD_REFERENCE  = " REFERENCE";
  }  
#endif  // defined(USE_STANDARD_SYMBOLS)


#endif // __SYMBOLS_CONSTANTS_INCLUDE__
