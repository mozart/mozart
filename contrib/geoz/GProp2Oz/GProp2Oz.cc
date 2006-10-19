/*
 * GProp2Oz is a tool to create an interface between Mozart and Gecode
 * propagators. This tools create the definitions that Mozart needs
 * to call a Gecode propagator using the GeOz integration package.
 *
 *
 * GProp2Oz was created by Javier Andrés Mena Zapata.
 * Copyright (2006) Javier Andrés Mena Zapata.
 * e-mail: javimena@univalle.edu.co
 *
 * Current file encoding: UTF8
 * 
 * Last modified:
 *   $Date: 2006-09-07 17:06:53 -0500 (Thu, 07 Sep 2006) $ by $Author: javimena $
 *   $Revision: 229 $
 */


#include <fstream>
#include <list>
#include <string>
#include <iostream>
#include <cstdio>
#include <map>
#include <vector>
#include <set>
#include <sstream>


template<class T>
void print_something(const T& tp) {
  typename T::const_iterator it = tp.begin();
  if (it != tp.end()) {
    std::cerr << *it;
  }
  for (it++; it != tp.end(); it++) {
    std::cerr << "," << *it;
  }
}

namespace GecodePropagatorsToMozart {

#include "symbols_constants.hh"

  using std::map;
  using std::string;
  using std::list;
  using std::pair;
  using std::set;
  using std::stringstream;
  using std::vector;


  struct simple_type;
  bool operator!=(const simple_type& st1, const simple_type& st2);
  bool operator==(const simple_type& st1, const simple_type& st2);
  bool operator<(const simple_type& st1, const simple_type& st2);
  
  class indent {
    int my_indent;
    
  public:
    indent(int n) {
      my_indent = n;
    }
    friend std::ostream& operator<<(std::ostream& os, const indent& ind);
  };

  bool checkFileExists(const string fn) {
    std::ifstream ifs;

    ifs.open(fn.c_str(), std::ifstream::in);
    ifs.close();
    bool fail = ifs.fail();
    if (fail) {
      std::cerr << "Fatal Error: The file '" << fn << "' doesn't exists" << std::endl;
      ifs.clear(std::ios::failbit);
    }
    return !fail;
  }
  
  std::ostream& operator<<(std::ostream& os, const indent& ind) {
    for (int i=0; i < ind.my_indent; i++) {
      os << " ";
    }
    return os;
  }
  
  struct simple_type {
    bool is_template;
    bool is_pointer;
    bool is_reference;
    bool is_const;
    list<string> name;
    list<simple_type> template_args;

    simple_type() {
      is_template  = false;
      is_pointer   = false;
      is_reference = false;
      is_const     = false;
    }

    string to_string() const {
      string s = get_call_name();
      string r;
      if (is_template) {
      	list<simple_type>::const_iterator st_it = template_args.begin();
      	s += "<";
      	while (st_it != template_args.end()) {
          s += st_it->to_string() + ",";
          ++st_it;
        }
        s.resize(s.length()-1);
      	s += ">";
      }

      return s;
    }
    
    string get_call_name() const {
      assert(name.size()>0);
      list<string>::const_iterator lit = name.begin();
      string s;
      while (lit != name.end()) {
        s += (*lit) + "::";
        ++lit;
      }
      s.resize(s.length()-2);
      return s;
    }
    
    string get_last_name() const {
      return name.back();
    }

    void print(std::ostream& out) const {
      list<string>::const_iterator lit = name.begin();
      out << (*lit);
      lit++;
      while (lit != name.end()) {
      	out << SCOPE_SOLVER << (*lit);
        ++lit;
      }

      if (is_template) {
      	list<simple_type>::const_iterator st_it = template_args.begin();
      	out << TEMPLATE_LEFT;
      	while (st_it != template_args.end()) {
          st_it->print(out);
          ++st_it;
        }
      	out << TEMPLATE_RIGHT;
      }
      
      if (is_const)     out << WORD_CONST;
      if (is_pointer)   out << WORD_POINTER;
      if (is_reference) out << WORD_REFERENCE;
    }
  };
  
  bool operator<(const simple_type& st1, const simple_type& st2) {
    return st1.to_string() < st2.to_string();
  }

  
  bool operator==(const simple_type& st1, const simple_type& st2) {
    if (st1.name.size() != st2.name.size()) return false;
    {
      /* Compare the contents of the "name" variable */
      list<string>::const_iterator lit1 = st1.name.begin();
      list<string>::const_iterator lit2 = st2.name.begin();
      while (lit1 != st1.name.end()) {
        if ((*lit1) != (*lit2)) return false;
        ++lit1; ++lit2;
      }
    }
    if (st1.is_template != st2.is_template) return false;
    if (st1.template_args.size() != st2.template_args.size()) return false;
    {
      /* Compare the contents of template arguments */
      list<simple_type>::const_iterator st_it1 = st1.template_args.begin();
      list<simple_type>::const_iterator st_it2 = st2.template_args.begin();
      while (st_it1 != st1.template_args.end()) {
        if ((*st_it1) != (*st_it2)) return false;
        ++st_it1; ++st_it2;
      }
    }
    return true;
  }
  
  bool operator!=(const simple_type& st1, const simple_type& st2) {
    return !(st1 == st2);
  }
  
    
  struct replace_information {
    //string short_type;
    string checker;
    string decl_upcast;
    string decl_strict;
    bool must_have_same_space;  
    string to_obtain_space;
    string to_obtain_real_var;
    bool operator<(const replace_information& ri) {
      if (checker     != ri.checker)  return checker < ri.checker;
      if (decl_upcast != ri.decl_upcast) return decl_upcast < ri.decl_upcast;
      if (decl_strict != ri.decl_strict) return decl_strict < ri.decl_strict;
      if (must_have_same_space != ri.must_have_same_space)
        return must_have_same_space;
    }
  };
  
  typedef map<string,replace_information> replace_map_type;
  
  struct function {
    simple_type name;
    list<simple_type> args;

    void print(std::ostream& out) {
      out << "name:\n";
      out << "   '"; name.print(out);
      out << "'\n";
      list<simple_type>::iterator it = args.begin();
      out << "args:\n";
      while (it != args.end()) {
        out << "   '"; it->print(out);
        out << "'\n";
        ++it;
      }
    }
  };
  
  
  
  string trim(const string& s) {
    int from=0,to=s.length()-1;
    while (s[from]==' ') from++;
    while (s[to]  ==' ') to--;
    if (to < from) return "";
    else return s.substr(from,to-from+1);
  }

  void break_string(const string& s, const string& tok, list<string>& res) {
    int from=0,to;
    while ((to = s.find(tok,from)) != -1) {
      res.push_back(trim(s.substr(from,to-from)));
      from = to + tok.length();
    }
    res.push_back(trim(s.substr(from)));
  }

  void read_name(const string& s, list<string>& res) {
    break_string(s,"::",res);
  }
  
  void read_simple_type(const string& s, simple_type& st);
  void read_args(const string& s, list<simple_type>& res) {
    list<string> ss;
    break_string(s,",",ss);
    list<string>::iterator it = ss.begin();
    while (it != ss.end()) {
      simple_type st;
      read_simple_type(*it,st);
      res.push_back(st);
      it++;
    }
  }
  
  
  bool ends_with(const string& s, const string& e) {
    if (s.length() < e.length()) return false;
    else return (s.substr(s.length()-e.length())==e);
  }

  void read_simple_type(const string& sparam, simple_type& st) {
    string s = trim(sparam);
    int from, to;
    from = s.find('<');
    {
      if (ends_with(s," const*")) {
        st.is_pointer   = true;
	      st.is_const     = true;
	      s.resize(s.length()-sizeof(" const*")+1);
      }
      else if (ends_with(s," const&")) {
        st.is_reference = true;
        st.is_const     = true;
        s.resize(s.length()-sizeof(" const&")+1);
      }
      else if (ends_with(s,"*")) {
        st.is_pointer   = true;
        s.resize(s.length()-sizeof("*")+1);
      }
      else if (ends_with(s,"&")) {
        st.is_reference = true;
        s.resize(s.length()-sizeof("&")+1);
      }
    }
    
    if (from==-1) {
      read_name(s,st.name);
    }
    else {
      st.is_template = true;
      read_name(s.substr(0,from),st.name);
      to = s.rfind('>');
      read_args(s.substr(from+1,to-from-1),st.template_args);
    }
  }
  
  string print_spaces(const int n) {
    string s;
    s.resize(n,' ');
    return s;
  }

  void analyse_line(const string& s, function& res) {
  
    if (s=="") return;
    /*if (s[0] != 'T') {
      std::cerr << "Warning: don't know how to handle this:\n\"" << s << "\"\n";
    }*/

    list<string> lst_args;
    int par_pos = s.find('(');
    read_simple_type(s.substr(0,par_pos),res.name);
    
    read_args(s.substr(par_pos+1,s.length()-par_pos-2),res.args);
  }
  
  string replace_param_string(const string& toReplace,
			      const string& what,
			      const string& with)
  {
    int pos;
    int last=0;
    
    string s;
    string swhat = "$" + what +"$";
    while ((pos=toReplace.find(swhat,last)) != -1) {
      s += toReplace.substr(last,pos-last);
      s += with;
      last = pos+swhat.length();
    }
    if ((unsigned)last < toReplace.length())
      s += toReplace.substr(last);

    return s;
  }
  
  string i2string(int i) {
    char cstr[16];
    std::sprintf(cstr,"%d",i);
    return string(cstr);
  }
  
  void print_usage(char* me) {
    std::cout << "Usage:"<< std::endl;
    std::cout << "    "<< me << " -i input_file -t template_function_file [-o output_file] [-p var_prefix]"
                 " [-v variable_equivalence_file] [-b basic_definition_file]" << std::endl;
    std::cout << "    "<< me << " -i input_file --just-print   print the functions names and parameters of each one.";
    std::cout << std::endl;
  }
  
  struct mozart_function_description {
    int input_params;
    int output_params;
    string mozart_fn_name;
    mozart_function_description() {
      input_params = output_params = 0;
    }
  };

  const simple_type& get_param(size_t pos, const function& fn) {
    size_t i=0;
    list<simple_type>::const_iterator it = fn.args.begin();
    for (; it != fn.args.end(); it++, i++) {
      if (i==pos) return *it;
    }
    return *it; 
  }

  
  void obtain_diff_params(size_t pos, const list<function>& params, vector<simple_type>& res) {
    set<simple_type> diff_params;
    list<function>::const_iterator cit = params.begin();
    for (; cit != params.end(); cit++) {
      diff_params.insert(get_param(pos,*cit));
    }
    set<simple_type>::iterator sit = diff_params.begin();
    for (; sit != diff_params.end(); sit++) {
      res.push_back(*sit);
    }
  }
  
  template<class T>
  const T& nth(size_t pos, const list<T>& lst) {
    typename list<T>::const_iterator it = lst.begin();
    size_t i=0;
    //std::cout << "list size: " << lst.size() << std::endl;
    for (; it != lst.end(); it++,i++) {
      if (i==pos) return *it;
    }
    std::cerr << "trying to access element: " << static_cast<int>(pos)
              << "  but the size of lst is:" << lst.size() << std::endl;
    assert(false);
    // BUG: throw exception
    throw 1;
  }
  
  size_t variant_exists(const list<int>& variant, const list<function>& params) {
    /* possibles = set(1,length(params))
     * for i in 1..length variant do
     *   let s = diff_params(i,params)
     *   let expected_type = s[variant[i]]
     *   remove from possibles all the functions which its i argument type is diffentent of variant;
     *   if possibles is empty
     *   then return false;
     * return ok;
     */
    
    //std::cout << "variant size: " << variant.size() << std::endl;
    set<size_t> possibles;
    for (size_t i=0; i < params.size(); i++)
      possibles.insert(i);
    
    //std::cout << "variant size: " << variant.size() << std::endl;
    list<int>::const_iterator lit = variant.begin();
    for (size_t i=0; lit != variant.end(); i++,lit++) {
      vector<simple_type> s;
      obtain_diff_params(i,params,s);
      /*std::cerr << "HERE!: size(s) = " << s.size() << std::endl;
      std::cerr << "variant size: " << variant.size() << std::endl;*/
      
      size_t pp = nth(i,variant);
      /*std::cerr << "asking s[" << pp << "]" << std::endl;*/
      const simple_type& expected_type = s.at(pp);
      /*std::cerr << "expected_type: " << expected_type.to_string() << std::endl;*/
      set<size_t>::iterator sit = possibles.begin();
      
      for (; sit != possibles.end(); sit++) {
        const function& fn = nth(*sit,params);
        if (expected_type != get_param(i,fn)) {
          possibles.erase(sit);
        }
      }
      if (possibles.size()==0) return 0;
    }
    if (possibles.size() == 1) return 1+*(possibles.begin());
    else return 0xFFFFFFFF;
  }
  
  string replace_name_ozparam(const string& where, const string& var_name, int current_oz) {
    string s = where;
    s = replace_param_string(s,"VAR_NAME",var_name);
    s = replace_param_string(s,"OZ_PARAM_NUMBER",i2string(current_oz));
    return s;
  }
  
  bool ignored_type(const simple_type& st) {
    simple_type mt;
    read_simple_type("Gecode::Space",mt);
    return st == mt;
  }
  
  void create_space_check_code(
    const replace_information& ri,
    string var_name,
    int oz_param,
    int ind,
    std::ostream& os)
  {
    if (ri.must_have_same_space) {
      string var_space;

      /* WARNING: It's possible that the variable name is not related with oz_param */
      var_space = replace_name_ozparam(ri.to_obtain_space,var_name,oz_param);
      
      /* generate code for checking same space */
      os
      //<<indent(ind)<< "if (curr_space == NULL) curr_space = " << var_space << ";" << std::endl
      << indent(ind)<< "if (" << var_space << " != curr_space)" << std::endl
      << indent(ind)<< "  RAISE_EXCEPTION(\"The variables are not in the same space\");" << std::endl
      << std::endl;
    }
  }

  template<class A,class B>
  void print_map(const map<A,B>& v) {
    typename map<A,B>::const_iterator cit = v.begin();
    std::cout << "printing a map" << std::endl;
    for (; cit != v.end(); cit++) {
      std::cout << "  map[" << cit->first << "] = " /*<< cit->second*/ << std::endl;
    }
  }
  
  const replace_information& find_replace_map(
          const string& search_string,
          const replace_map_type& replace_map)
  {
    replace_map_type::const_iterator cit = replace_map.find(search_string);
    //print_map(replace_map);
    assert(cit != replace_map.end());
    const replace_information& ri = cit->second;
    return ri;
  }


  void create_code(size_t current_real,
                   size_t current_oz,
                   size_t ind,
		   const list<function>& params,
		   list<int>& variant,
		   set< pair<int,int> >& unchanged_params,
		   std::ostream& ocheck,
		   const replace_map_type& replace_map,
		   const string& var_prefix,
		   int quietLevel
		   )
  {
  
    
    list<function>::const_iterator it = params.begin();

    // all the functions has the same number of arguments
    if (current_real >= it->args.size()) {
      size_t var_number = variant_exists(variant,params);
      assert(var_number != 0xFFFFFFFF);
      
      {
        // print the parameters list before calling or failing
        ocheck << indent(ind) << "// generating code for the current variant" << std::endl;
	list<int>::iterator lit = variant.begin();
	for (int i=0;lit != variant.end(); lit++, i++) {
          vector<simple_type> diff_params;
       	  obtain_diff_params(i,params,diff_params);
          ocheck << indent(ind) << "//   " << diff_params.at(*lit).to_string() << std::endl;
	}
      }
      if ( var_number-- > 0 ) {
        /* the current variant exists, then generate code for calling the function */
        const function& fn = nth(var_number,params);
        ocheck
	  << indent(ind) << "if (curr_space==NULL)" << std::endl
	  << indent(ind) << "  RAISE_EXCEPTION(\"No variable has the GeSpace object\");" << std::endl;
        
        /* BUG: The program supposes that the Space is the first argument */
	ocheck << indent(ind) << fn.name.to_string() << "(";
	if (current_real>0) ocheck << "curr_space->getSpace()," << std::endl;
	int function_name_length = fn.name.to_string().length();
	for (size_t i=1; i < current_real; i++) {
          // for each argument: generate the code.
          const simple_type& current_type = nth(i,fn.args);
          
          /* find the replace information in the table */
          const replace_information& ri = find_replace_map(current_type.to_string(),replace_map);
          
          string var_call;
          if (ri.must_have_same_space) {
            var_call = replace_name_ozparam(ri.to_obtain_real_var,var_prefix + i2string(i),i);
          }
          else {
            var_call = var_prefix+i2string(i);
          }
          ocheck << indent(function_name_length+ind+1) << var_call;
          if ((i+1) != current_real) ocheck << "," << std::endl;
	}
	ocheck << ");" << std::endl;
      }
      else {
	/* the current variant doesn't exists */
	ocheck << indent(ind) << "// error: variant doesn't exists" << std::endl;
	ocheck << indent(ind) << "RAISE_EXCEPTION(\"variant doesn't exists\");" << std::endl;
      }
      return;
    }
    
    vector<simple_type> diff_params;
    obtain_diff_params(current_real,params,diff_params);
    
    if (diff_params.size()==1) {
      //oglobal << indent(start_ind) << "declare_type_for_var_" << current << ";" << std::endl;
      unchanged_params.insert(pair<int,int>(current_real,current_oz));
      const simple_type& st = diff_params[0];
      variant.push_back(0);  /* there is only one variant: the first = 0 */
      int curr_oz_code = current_oz + (ignored_type(st) ? 0 : 1);
      create_code(current_real+1,
		  curr_oz_code,
		  ind,
		  params,
		  variant,
		  unchanged_params,
		  ocheck,
		  replace_map,
		  var_prefix,
		  quietLevel);

      variant.pop_back();
      return;
    }
    
    vector<simple_type>::iterator sit = diff_params.begin();
    for (int current_type_idx=0; sit != diff_params.end(); current_type_idx++, sit++) {
      // for each different possible variant of the function, then, make the respective
      // code. First, we must check the type of the parameter (the Oz term).
      simple_type& current_type = diff_params.at(current_type_idx);
      if (!ignored_type(current_type)) {
        const replace_information& ri = find_replace_map(current_type.to_string(),replace_map);
        
        string var_name = var_prefix + i2string(current_real);
        string decl_strict = replace_name_ozparam(ri.decl_strict,var_name,current_oz);
        string checker = replace_name_ozparam(ri.checker,var_name,current_oz);

        ocheck << indent(ind);
        if (sit != diff_params.begin()) {
          ocheck << "else ";
        }        
        ocheck << "if ( " << checker <<  " ) {" << std::endl;
        //ocheck << indent(ind) << "  // current:   param: "<< current << "  type:  " << current_type_idx << "\n";
        /* create the variable declaration in the local scope (the if scope) */
        ocheck << indent(ind+2) << decl_strict << ";" << std::endl;
        
        /* chech that the variable is in the same space of the others */
        create_space_check_code(ri, var_prefix+i2string(current_oz),current_oz, ind+2, ocheck);
        
        /* choose the current variant type for generating the final call.
         * The current variant is determined by current_type_idx
         */
        
        variant.push_back(current_type_idx);
        create_code(current_real+1,
                    current_oz+1,
                    ind+2,
                    params,
                    variant,
                    unchanged_params,
                    ocheck,
                    replace_map,
                    var_prefix,
		    quietLevel);
        variant.pop_back();
        ocheck << indent(ind) << "}" << std::endl;
      }
      else {
	if (quietLevel < 2)
	  std::cerr << "WARNING: I don't know how to handle this. Line: " << __LINE__ << std::endl;
      }
    }
    ocheck << indent(ind)   << "else {" << std::endl;
    ocheck << indent(ind+2) << "// error: unrecognized or non expected type" << std::endl;
    ocheck << indent(ind)   << "}" << std::endl;
  }
  
  void create_interface(const string& mozart_fn_name,
			const list<function>& fns,
                        const replace_map_type& replace_map,
                        const list<string>& template_file,
                        const string& var_prefix,
                        list<mozart_function_description>& lst_mozart_functions,
                        std::ostream& output_file,
			int quietLevel
			)
  {
    const function& fn = *(fns.begin());
    int num_fn_args = fn.args.size();
    std::vector<replace_information> vec_replaced_info;
    
    map<string,string> replace_table;
    {
    
      std::string declare_space_code;
      
      declare_space_code  = "GeSpace *curr_space = NULL;\n";
        
      replace_table["MOZART_FN_NAME"]          = mozart_fn_name;
      replace_table["FUNCTION_REAL_NAME"]      = fn.name.get_call_name();

      // BUG: num_fn_args - 1 is not correct
      replace_table["NUM_ARGUMENTS"]           = i2string(num_fn_args-1);
      replace_table["DECLARE_SPACE"]           = declare_space_code;

      
      {
        mozart_function_description mfd;
        mfd.input_params   = num_fn_args;
        mfd.output_params  = 0;
        mfd.mozart_fn_name = mozart_fn_name;
        lst_mozart_functions.push_back(mfd);
      }
    }
    
    // read the file and print to out with every line replaced.
    {
      list<string>::const_iterator s_it = template_file.begin();
      for (; s_it != template_file.end(); ++s_it) {
        string s = *s_it;
        stringstream create_function_calls;
        set< pair<int,int> > unchanged_params;

        int indent_pos = s.find("$CREATE_FUNCTION_CALLS$");
        if (indent_pos != -1) {
          create_function_calls << std::endl;
          list<int> variant;
          list<function> newfns = fns;
          /* change newfns to accept upcastings */
          
          /* end of changing to newfns*/

	  /* create the function call */
          create_code(0,0,
		      indent_pos,newfns,
		      variant,unchanged_params,
		      create_function_calls,
		      replace_map,var_prefix,
		      quietLevel);

	  /* create the declarations for the variables */
          assert(variant.size()==0);
          stringstream decls_for_unchanged_vars;
          decls_for_unchanged_vars << std::endl;
          set< pair<int,int> >::iterator it = unchanged_params.begin();
          for (; it != unchanged_params.end(); it++) {
            const function& any_fn = *(fns.begin());
            const simple_type& current_type = nth(it->first, any_fn.args);
            if (!ignored_type(current_type)) {
              string curr_type_string = current_type.to_string();
              const replace_information& ri = find_replace_map(curr_type_string,replace_map);
  
              string var_decl;
              var_decl = replace_name_ozparam(ri.decl_upcast, var_prefix + i2string(it->first), it->second);
              
              /* create the declaration for the variables that doesn't
               * change in the formal parameters
               */
              decls_for_unchanged_vars << indent(indent_pos) << var_decl <<  ";" << std::endl;
              create_space_check_code(ri,
				      var_prefix+i2string(it->first),
				      it->second,
				      indent_pos,
				      decls_for_unchanged_vars);
            }
          }
          
          replace_table["CREATE_FUNCTION_CALLS"] = decls_for_unchanged_vars.str() + create_function_calls.str();
          
        }
	
        s = replace_param_string(s,"DECLARE_SPACE",        replace_table["DECLARE_SPACE"]);
        s = replace_param_string(s,"MOZART_FN_NAME",       replace_table["MOZART_FN_NAME"]);
        s = replace_param_string(s,"FUNCTION_REAL_NAME",   replace_table["FUNCTION_REAL_NAME"]);
        s = replace_param_string(s,"NUM_ARGUMENTS",        replace_table["NUM_ARGUMENTS"]);        
        s = replace_param_string(s,"CREATE_FUNCTION_CALLS",replace_table["CREATE_FUNCTION_CALLS"]);
        output_file << s << std::endl;
      }
    }
  }


  bool read_one_line(std::ifstream& ifs, string& s, int& lc) {
    while (!ifs.eof()) {
      std::getline(ifs,s);
      lc++;
      if (s[0]!='#') return true;
    }
    return false;
  }

  string real_mozart_name(const string s) {
    return s.substr(2);
  }

  void main_function(int quietLevel,
		     const list<string>& template_file,
		     const string& var_prefix,
		     std::ifstream& input_file,
		     replace_map_type& replace_map,
		     bool just_print,
		     const string& basic_definitions_file,
		     std::ostream& os )
  {
    typedef map< string,list<function> > function_map_type;
    list<mozart_function_description> lst_mozart_functions;
    function_map_type function_map;
    
    if (!just_print) {
      os << "/" << string(80,'*') << std::endl;
      os << " * This file was automatically generated by GProp2Oz." << std::endl;
      os << " * GProp2Oz is a tool to create an interface between Mozart and Gecode" << std::endl;
      os << " * propagators." << std::endl;
      os << " *" << std::endl;
      os << " * GProp2Oz was created by Javier Andrés Mena Zapata." << std::endl;
      os << " * e-mail: javimena@univalle.edu.co" << std::endl;
      os << " *" << std::endl;
      os << " " << string(80,'*') << "/" << std::endl;
      os << std::endl;
      os << "#include \"" << basic_definitions_file << "\"" << std::endl;
      os << std::endl;
    }
    
    while (!input_file.eof()) {
      string s;

      std::getline(input_file,s);
      
      function fn;
      if (trim(s)=="") continue;
      analyse_line(s,fn);
      if (just_print) {
      	os << "Function" << std::endl;
    	  fn.print(os);
      	os << std::endl;
      }
      else {
      	//string stored_name = fn.name.get_last_name()
        string stored_name = fn.name.get_last_name();
        for(;;) {
	  function_map_type::const_iterator it = function_map.find(stored_name);
	  
	  if (it == function_map.end()) {
	    function_map.insert(function_map_type::value_type(stored_name,list<function>()));
	    break;
	  }
	  else {
	    const function& fnaux = it->second.back();
	    if (fnaux.args.size() == fn.args.size())  break;
	    
	    // BUG: fn.args.size()-1 is bad
	    stored_name = fn.name.get_last_name() + i2string(fn.args.size()-1);
	  }
        }
  	
      	list<function>& lst = function_map[stored_name];
      	lst.push_back(fn);
      }
    }
    if (!just_print) {
      input_file.close();
      
      function_map_type::const_iterator it = function_map.begin();
      for (; it != function_map.end(); it++) {
	create_interface(string("__")+it->first,  // C function name
			 it->second,              // functions 
			 replace_map,             //
			 template_file,           //
			 var_prefix,              //
			 lst_mozart_functions,    //
			 os,                      //
			 quietLevel
			 );
      }
    }
    
    
    if (!just_print) { 
      string module_name = "module_name";
      os << "char oz_module_name[] = \"" << module_name << "\";" << std::endl;
      os << std::endl;
      os << "extern \"C\"" << std::endl;
      os << "{" << std::endl;
      os << "  OZ_C_proc_interface * oz_init_module(void)" << std::endl;
      os << "  {" << std::endl;
      os << "    static OZ_C_proc_interface i_table[] = {" << std::endl;
      list<mozart_function_description>::iterator it = lst_mozart_functions.begin();
      string export_lines;
      for (;it != lst_mozart_functions.end(); ++it) {
        os << indent(6) << "{\"";
        os << real_mozart_name(it->mozart_fn_name) << "\",";
        os << it->input_params-1   << ",";  // BUG: it is not always true
        os << it->output_params  << ",";
        os << it->mozart_fn_name << "}," << std::endl;
      }
      os << "      {0,0,0,0}" << std::endl;
      os << "    };" << std::endl;
      os << std::endl;
      os << std::endl;
      os << "    return i_table;" << std::endl;
      os << "  }" << std::endl;
      os << "}" << std::endl;
    }
  }
}



int main(int argc, char* argv[]) {
  using namespace GecodePropagatorsToMozart;
  string template_filename;
  string var_prefix;
  string input_filename;
  string output_filename;
  string var_equivalence_filename;
  string basic_definitions_filename;
  
  char *me = argv[0];
  bool use_cout = false;
  bool just_print = false;
  int quietLevel = 0;
  while (argv++, --argc) {
    if (std::strcmp(*argv,"-t")==0) {
      // template file name
      argv++, --argc;
      template_filename = *argv;
    }
    else if (std::strcmp(*argv,"--just-print")==0) {
      just_print = true;
    }
    else if (std::strcmp(*argv,"-p")==0) {
      // var prefix
      argv++, --argc;
      var_prefix = *argv;
    }
    else if (std::strcmp(*argv,"-i")==0) {
      // input file name
      argv++, --argc;
      input_filename = *argv;
    }
    else if (std::strcmp(*argv,"-o")==0) {
      // output file name
      argv++, --argc;
      output_filename = *argv;
    }
    else if (std::strcmp(*argv,"-v")==0) {
      // variable equivalence replacement file name
      argv++, --argc;
      var_equivalence_filename = *argv;
    }
    else if (std::strcmp(*argv,"-b")==0) {
      // basic definitions file name
      argv++, --argc;
      basic_definitions_filename = *argv;
    }
    else if (std::strcmp(*argv,"-q")==0) {
      /* quiet mode -- no warnings on arguments */
      quietLevel = 1;
    }
    else if (std::strcmp(*argv,"-vq")==0) {
      /* quiet mode -- no warnings at all */
      quietLevel = 2;
    }
    else {
      // unrecognized argument
      std::cerr << "Unrecognized argument: '" << *argv << "'" << std::endl;
      return 1;
    }
  }

  if (input_filename=="") {
    print_usage(me);
    return 1;
  }  
  if (!just_print && template_filename=="") {
    if (quietLevel < 1)
      std::cerr << "Note: template file not specified. Using template_file.cc." << std::endl;
    template_filename = "template_file.cc";
  }
  if (!just_print && var_equivalence_filename=="") {
    if (quietLevel < 1)
      std::cerr << "Note: variable equivalence file not specified. Using var_equivalences.txt." << std::endl;
    var_equivalence_filename = "var_equivalences.txt";
  }
  if (!just_print && var_prefix=="") {
    var_prefix = "local_arg";
  }
  if (!just_print && basic_definitions_filename=="") {
    basic_definitions_filename = "BasicDefinitions.cc";
  }

  if (!checkFileExists(input_filename)) return 1;
  if (!just_print) {
    if (!checkFileExists(var_equivalence_filename)) return 1;
    //if (!checkFileExists(basic_definitions_filename)) return 1;
    if (!checkFileExists(template_filename)) return 1;
  }
  
  list<string> template_file;
  if (!just_print) {
    // read the input template file
    std::ifstream ifs(template_filename.c_str());
    while (!ifs.eof()) {
      string s;
      std::getline(ifs,s);
      template_file.push_back(s);
    }
    ifs.close();
  }
  
  replace_map_type replace_map;
  
  if (!just_print) {
    // read the variable equivalence file
    std::ifstream ifs(var_equivalence_filename.c_str());
    int lc=1;
    while (!ifs.eof()) {
      string s1,s2,s3,s4,s5;
      if (!read_one_line(ifs,s1,lc)) break;
      if (s1 == "EOF") break;
      if (!read_one_line(ifs,s2,lc)) {
        std::cout << "error in " << var_equivalence_filename
                  << ": reached end of file with incomplete information" << std::endl;
        std::cout << s1 << std::endl;
        return 1;
      }
      if (!read_one_line(ifs,s3,lc)) {
        std::cout << "error in " << var_equivalence_filename
                  << ": reached end of file with incomplete information" << std::endl;
        std::cout << s1 << std::endl;
        std::cout << s2 << std::endl;
        return 1;
      }
      if (!read_one_line(ifs,s4,lc)) {
        std::cout << "error in " << var_equivalence_filename
                  << ": reached end of file with incomplete information" << std::endl;
        std::cout << s1 << std::endl;
        std::cout << s2 << std::endl;
        std::cout << s3 << std::endl;
        return 1;
      }
      if (!read_one_line(ifs,s5,lc)) {
        std::cout << "error in " << var_equivalence_filename
                  << ": reached end of file with incomplete information" << std::endl;
        std::cout << s1 << std::endl;
        std::cout << s2 << std::endl;
        std::cout << s3 << std::endl;
        std::cout << s4 << std::endl;
        return 1;
      }
      simple_type st;
      read_simple_type(s1,st);
      
      replace_information ri;
      ri.decl_upcast          =  s2;
      ri.decl_strict          =  s3;
      ri.checker              =  s4;
      if ((s5 != "true") && (s5 != "false")) {
	std::cout << "Error in " << var_equivalence_filename
		  << " line: " << lc << ": it should be true or false and not '"
		  << s5 << "'" << std::endl;
	return 1;
      }
      ri.must_have_same_space = (s5 == "true");

      
      if (ri.must_have_same_space) {
        if (!read_one_line(ifs,s1,lc)) {
          std::cout << "error in " << var_equivalence_filename
                    << ": reached end of file with incomplete information" << std::endl;
	  return 1;
        }
        if (!read_one_line(ifs,s2,lc)) {
          std::cout << "error in " << var_equivalence_filename
                    << ": reached end of file with incomplete information" << std::endl;
	  return 1;
        }
        ri.to_obtain_space = s1;
        ri.to_obtain_real_var = s2;
      }
      
      string key = st.to_string();
      replace_map[key] = ri;
    }
    ifs.close();
  }

  std::ofstream output_stream;
  
  if (output_filename=="") use_cout = true;
  else {
    output_stream.open(output_filename.c_str());
  }
  std::ifstream input_file(input_filename.c_str());

  main_function(
    quietLevel,
    template_file,
    var_prefix,
    input_file,
    replace_map,
    just_print,
    basic_definitions_filename,
    use_cout ? std::cout : output_stream );

  if (!use_cout) output_stream.close();

  return 0;
}
