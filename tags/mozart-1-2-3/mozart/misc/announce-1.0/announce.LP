Release of Mozart 1.0.0
-----------------------

Available at: http://www.mozart-oz.org

Systems supported: Many Unix-based platforms, Windows 95/98/NT

The Mozart consortium (DFKI & UdS & SFB 378, SICS, UCL) is pleased to
announce the release of Mozart 1.0, a major development platform for
logic programming, constraint programming, and distributed
programming.  Mozart implements the Oz language and is the fruit of a
decade of research into concurrent constraint, logic, and distributed
programming.  The platform is released with a very liberal license
(X11-style) that allows both commercial and non-commercial use.  Full
source code is available.  There is extensive documentation including
a full set of tutorials and reference manuals, and many demos.
Performance is competitive with commercial Prolog and Lisp systems.

We compare the logic and constraint programming abilities of Mozart
with Prolog-like languages.  In our experience, there are two
essentially different styles of declarative logic programming, which
can be called the 'algorithmic' style and the 'free' style, depending
on whether the programmer knows the control flow or not.  Prolog
provides a single resolution strategy to support both, which makes
programming awkward in each one.  Oz explicitly distinguishes between
the two and provides powerful support for each.  In addition, Oz
provides four constraint systems, higher-orderness, concurrency, and
state.  While Prolog uses a reflective operator-based syntax (same
syntax for program and data), Oz has powerful scanner and parser
generator tools.

In the algorithmic style, the programmer knows the control flow
completely.  Much Prolog code is written in this style.  Mercury is
targeted for this style only.  The programmer specifies both the
declarative semantics and the complete operational semantics of the
program.  In Prolog, the latter is determined by the order of clauses
and goals within clauses.  The programmer has to be careful to avoid
unwanted choice points, for example by using green cuts.  With
'freeze', a little bit of concurrency is possible, but there is no
preemptive concurrency.

In the free style, the programmer knows the inference rules and
logical constraints of the problem, but does not know much about the
control flow.  The free style is used often in constraint programming,
theorem proving, and computational linguistics.  Usually, the
programmer specifies the problem's logical constraints, purely
declaratively, and then separately specifies the inference (or search,
or resolution) strategy.  The strategy gives partial information about
the operational semantics.  Prolog is quite weak for specifying the
strategy; a logic programming language intended for the free style
usually extends Prolog in substantial ways.  Languages such as CHIP,
Prolog-IV, clp(FD), cc(FD), etc., fall in this category.

Oz supports well both styles of declarative programming.  For the
algorithmic style, a typical Oz construct is
'cond C then S1 else S2 end' (and its syntactic short-cuts, 'if' and
'case') where C is a constraint and S1, S2 are statements.  The 'cond'
has a precise logical meaning '(C /\ S1) \/ (not C /\ S2)' as well as
a precise operational meaning: wait until either C or not C can be
determined, then execute one alternative.  

For the free style, Oz uses just two basic concepts: first-class
computation spaces and determinacy-driven disjunctions (Andorra-style).
Using these concepts allows to efficiently program the resolution
strategy from within Oz.  The system provides powerful tools and
libraries based on this approach.  For example, the Oz Explorer, an
interactive tool that allows to visualize and control the search
strategy, is built in this way.

In addition to logic programming, Mozart provides:

- Four constraint systems: rational trees (as in modern Prologs),
  finite domains, record constraints, and finite sets of integers.

- Higher-orderness: procedures are first-class with lexical scoping,
  as in functional languages.  This allows programs to be compacter
  (less repetition of code) and supports data abstraction.

- Concurrency: lightweight preemptive threads.  Many thousands of
  threads can be easily created.

- State: a powerful object system with multiple inheritance and
  first-class classes, methods, and attributes.

- Distribution: full support for open fault-tolerant
  network-transparent distributed computing.  For example, it is
  trivial to create applets or CGI servlets that do constraint solving
  and logical inferencing.

- Tools: an extensive set of tools, libraries, and a powerful C++
  interface.

References

1. Seif Haridi, Peter Van Roy, Per Brand, Christian Schulte, "Programming
   Languages for Distributed Applications", New Generation Computing,
   May 1998, Vol. 16, No. 3, pp. 223-261. 
 
2. Gert Smolka, "The Oz Programming Model", Computer Science Today, Springer
   Verlag, Lecture Notes in Computer Science vol. 1000, 1995, pp. 324-343.
 
3. Peter Van Roy, Seif Haridi, Per Brand, Gert Smolka, Michael Mehl,
   Ralf Scheidhauer, "Mobile Objects in Distributed Oz", ACM Transactions
   on Programming Languages and Systems (TOPLAS), Sept. 1997, pp. 804-851. 
 
4. Seif Haridi, Peter Van Roy, Per Brand, Michael Mehl, Ralf Scheidhauer,
   Gert Smolka, "Efficient Logic Variables for Distributed Computing",
   ACM Transactions on Programming Languages and Systems (TOPLAS), 1999
   (to appear).
 
5. Per Brand, Peter Van Roy, Raphael Collet, Erik Klintskog, "A
   Fault-Tolerant Mobile State Protocol and Its Language Interface",
   Nov. 1998 (submitted for publication).
 
6. Joerg Wuertz, "Contraint-Based Scheduling in Oz", Operations Research
   Proceedings, Springer Verlag, 1996.
 
7. Christian Schulte, "Programming Constraint Inference Engines",
   Principles and Practice of Constraint Programming, Springer Verlag,
   1997.
 
Contact addresses:
 
   Seif Haridi
   +46 (8) 633-1500
   seif@sics.se
 
   Christian Schulte
   +49 (681) 302-5340
   schulte@ps.uni-sb.de
 
   Peter Van Roy
   +32 (10) 47.83.74
   pvr@info.ucl.ac.be
