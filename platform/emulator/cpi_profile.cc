/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
  Author: tmueller
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/


#include "oz_cpi.hh"


OZ_CFunHeader * OZ_CFunHeader::_all_headers = NULL;


void OZ_CFunHeader::profileReset()
{
  OZ_CFunHeader *aux = getFirst();
  while(aux) {
    aux->_calls   = 0;
    aux->_samples = 0;
    aux = aux->getNext();
  }
}
