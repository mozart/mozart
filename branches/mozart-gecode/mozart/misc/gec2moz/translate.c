/*
 *  Main authors:
 *     Diana Lorena Velasco <dlvelasco@puj.edu.co>
 *     Juan Gabriel Torres  <juantorres@puj.edu.co>
 *
 *  Contributing authors:
 *     
 *
 *  Copyright:
 *     Diana Lorena Velasco, 2007
 *     Juan Gabriel Torres, 2007
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include "lexer.h"

typedef map<string,string> mapss;

sp getE(vector<sp> *v, int pos){
  return (*v)[pos];
}


string Get_FName(pg C, mapss Initials){
  string tmp;
  for(unsigned int i=1;i<C->parlist->size();i++){
    tmp += Initials.find(getE(C->parlist,i)->tvar)->second;
  }
  return tmp;
}

void CGen(const char *outFile, vector<pg> Code, mapss Check, mapss Dec, mapss Space, mapss Initials){
  fstream file(outFile,ios::out);
  for(int i=0;i<(int)Code.size();i++){
    string Params = "";
    string SubN = Get_FName(Code[i],Initials);
    file << "OZ_BI_define(" << Code[i]->nppg << SubN << "," << Code[i]->n_param-1 << ",0)\n{\n";	
    file << "\t" << Dec.find(getE(Code[i]->parlist,0)->tvar)->second << "(" << getE(Code[i]->parlist,0)->var << ");\n";
    if(getE(Code[i]->parlist,0)->vdef == NULL){
      Params.assign(getE(Code[i]->parlist,0)->var,strlen(getE(Code[i]->parlist,0)->var));
    }
    else{
      Params.assign(getE(Code[i]->parlist,0)->vdef,strlen(getE(Code[i]->parlist,0)->vdef));
    }
    for(int j=1;j<(int)Code[i]->parlist->size();j++){
      string Paraux = "";
      if(j != (int)Code[i]->parlist->size()){
	Params += ", ";
      }
						
      if(Space.find(getE(Code[i]->parlist,j)->tvar)->second != "true"){
	if(getE(Code[i]->parlist,j)->vdef == NULL){
	  file << "\t" << Dec.find(getE(Code[i]->parlist,j)->tvar)->second << "(" << j-1 << 
	    ", __" << getE(Code[i]->parlist,j)->var << ");\n";
	  Paraux.assign(getE(Code[i]->parlist,j)->var,strlen(getE(Code[i]->parlist,j)->var));
	  Paraux = "__" + Paraux;
	  Params += Paraux;
	}
	else{
	  file << "\t" << Dec.find(getE(Code[i]->parlist,j)->tvar)->second << "(" << j-1 <<
	    ", __" << getE(Code[i]->parlist,j)->vdef << ");\n";
	  Paraux.assign(getE(Code[i]->parlist,j)->vdef,strlen(getE(Code[i]->parlist,j)->vdef));
	  Paraux = "__" + Paraux;
	  Params += Paraux;
	}
      }
      else{
	if(getE(Code[i]->parlist,j)->vdef == NULL){
	  file << "\t" << Dec.find(getE(Code[i]->parlist,j)->tvar)->second << "(" << j-1 << 
	    ", __" << getE(Code[i]->parlist,j)->var << ", " << getE(Code[i]->parlist,0)->var << ");\n";
	  Paraux.assign(getE(Code[i]->parlist,j)->var,strlen(getE(Code[i]->parlist,j)->var));
	  Paraux = "__" + Paraux;
	  Params += Paraux;
	}
	else{
	  file << "\t" << Dec.find(getE(Code[i]->parlist,j)->tvar)->second << "(" << j-1 <<
	    ", __" << getE(Code[i]->parlist,j)->var << ", " << getE(Code[i]->parlist,0)->vdef << ");\n";
	  Paraux.assign(getE(Code[i]->parlist,j)->vdef,strlen(getE(Code[i]->parlist,j)->vdef));
	  Paraux = "__" + Paraux;
	  Params += Paraux;
	}
      }
    }
    file << "\ttry{\n\t\t" << Code[i]->nspace << Code[i]->nppg << "(" << Params <<
      ");\n\t}\n\tcatch(Exception e){\n\t\tRAISE_GE_EXCEPTION(e);\n\t}\n" ;
    file << "\tGZ_RETURN(" << getE(Code[i]->parlist,0)->var << ");\n}OZ_BI_end\n\n";
				
		
  }
  file.close();
}



/**
   \brief The program is caled ./translate inputfile.h outputfile.c
 */
int main (int argc,char *argv[]){
  extern vector<pg> C;	
  extern mapss Check;
  extern mapss Initials;
  extern mapss Dec;
  extern mapss Space;
	
  if (argc == 3){
    if((yyin=fopen(argv[1],"r")) !=NULL){	 
      yyparse();
      fclose(yyin);
      CGen(argv[2],C,Check,Dec,Space,Initials);
    }
    else cout << "Couldn't read input file." << endl; 
  } 
  else cout << "Wrong arguments number" << endl << "Hint: ./translate inputfile.h outputfile.c" << endl; 
  return (0);
}
