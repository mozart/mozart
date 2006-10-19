\insert '/home/ps-home/tmueller/Programming/Oz/RealIntervals/ri.oz'
\insert 'mks_prob.oz'
\insert 'mks_fd.oz'

/*

{ExploreBest {KnapsackFD Problem} proc {$ O N} O.maxprofit <: N.maxprofit end}

declare S = {SearchBest {KnapsackFD Problem} proc {$ O N} O.maxprofit <: N.maxprofit end}
{Show S}

sol(
   maxprofit:121 
   products:[0 0 3 3 0 0 0 0 0 2 1 5 0])
5270/44/5227

*/

\insert 'mks_lp_a.oz'
\insert 'mks_lp_b.oz'
\insert 'mks_lp_c.oz'
\insert 'mks_lp_d.oz'

/*

{ExploreBest {KnapsackLP Problem} proc {$ O N} {RI.lessEq O.maxprofit+1.0 N.maxprofit} end}

declare S = {SearchBest {KnapsackLP Problem} proc {$ O N} {RI.lessEq O.maxprofit+1.0 N.maxprofit} end}
{Show S}

sol(
   maxprofit:121.0 
   products:[0.0 0.0 3.0 3.0 0.0 0.0 0.0 0.0 0.0 
             2.0 1.0 5.0 0.0])

490/12/479

*/

\insert 'mks_fdlp.oz'

/*

{ExploreBest {KnapsackFDLP Problem} proc {$ O N} O.maxprofit <: N.maxprofit end}

declare S = {SearchBest {KnapsackFDLP Problem} proc {$ O N} O.maxprofit <: N.maxprofit end}
{Show S}


sol(
   maxprofit:121 
   products:[0 0 3 3 0 0 0 0 0 2 1 5 0])

52/8/45
*/

