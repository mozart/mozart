/* ssort - implementation of the subset sort rule */

#include "flexdef.h"


/* test_subset - test whether a set of states is a subset of another
 *
 * synopsis
 *     is_subset = test_subset( int ss1[num1+1], int num1,
 *				int ss2[num2+1], int num2 );
 *
 * ss1 and ss2 are the sets of states in increasing order of length num1
 * and num2 respectively.  is_subset is true if and only if all elements
 * of ss1 are contained in ss2.
 */

static int test_subset( ss1, num1, ss2, num2 )
int ss1[], num1;
int ss2[], num2;
	{
	int i, j;

	j = 1;
	for ( i = 1; i <= num1; ++i )
		{
		while ( ss2[j] < ss1[i] )
			{
			j++;
			if ( j > num2 )
				return 0;
			}
		if ( ss2[j] != ss1[i] )
			return 0;
		}

	return 1;
	}


/* prefer - return whether one rule is to be preferred over another
 *	    as indicated by the subset rule.
 *
 * synopsis
 *     result = prefer( int i, int j );
 *
 * result is true if and only if the rule with number i is to be preferred
 * over the rule with number j, i.e, if rule i is a proper subset of rule j
 * or they are equivalent but rule i is listed before rule j in the input
 * file.
 */

static int prefer( i, j )
int i, j;
	{
	return (dassnum[i] < dassnum[j] || dassnum[i] == dassnum[j] && i < j )
	       && test_subset( dass[i], dassnum[i], dass[j], dassnum[j] );
	}


/* set_remove - remove all elements of one set from another set
 *
 * synopsis
 *     void set_remove( int ss1[num1+1], int num1,
 *			int ss2[num2+1], int *num2_addr );
 *
 * ss1 and ss2 are the sets of states in increasing order of length num1
 * and num2 respectively.  The elements from ss1 are removed from ss2
 * destructively if present.
 */

static void set_remove( ss1, num1, ss2, num2 )
int ss1[], num1;
int ss2[], *num2;
	{
	int i, j, k;

	/* read index in ss2 */
	j = 1;
	/* overwrite index in ss2 */
	k = 1;
	for ( i = 1; i <= num1; ++i )
		{
		/* copy intermediate elements */
		while ( ss2[j] < ss1[i] )
			{
			ss2[k++] = ss2[j++];
			if ( j > *num2 )
				{
				*num2 = k - 1;
				return;
				}
			}

		if ( ss2[j] == ss1[i] )
			{
	 		j++;   /* remove element ss1[i] from ss2 */
			if ( j > *num2 )
				{
				*num2 = k - 1;
				return;
				}
			}
		}

	/* copy leftover elements */
	while ( j <= *num2 )
		ss2[k++] = ss2[j++];

	*num2 = k - 1;
	}


/* do_subset_sort_state - calculate the single rule to be accepted by a state
 *			  as indicated by the subset rule.
 *
 * synopsis
 *     void do_subset_sort_state();
 *
 * Let P1, ..., Pn be the sets of accepting states:
 *   n = num_rules,
 *   Pi = { dass[i][j] | 0 < j <= dassnum[i] }.
 * The following algorithm is executed:
 *
 * for each i = 1, ..., n, do
 *   Pi' := Pi;
 *   for each j = 1, ..., n, i != j, do
 *     if rule j is preferred over rule i then
 *       let Pi' := Pi' - Pj;
 *   for each state j in Pi' do
 *     if dfaacc[j].dfaacc_state is set to k then
 *       error: Pi and Pk are in conflict;
 *     else
 *       set dfaacc[j].dfaacc_state to i;
 *
 * Special handling is provided for the default rule so that appropriate
 * warnings can be issued or suppressed as needed.
 */

static void do_subset_sort_state()
	{
	int i, j, *dnew, num;

	dnew = allocate_integer_array( current_max_dfas );

	for ( i = 1; i <= num_rules; ++i )
		{
		num = dassnum[i];
		for ( j = 1; j <= num; ++j )
			dnew[j] = dass[i][j];

		for ( j = 1; j <= num_rules; ++j )
			if ( i != j && prefer( j, i ) )
				set_remove( dass[j], dassnum[j], dnew, &num );

		for ( j = 1; j <= num; ++j )
			if ( dfaacc[dnew[j]].dfaacc_state == 0 )
				{
				dfaacc[dnew[j]].dfaacc_state = i;
				rule_useful[i] = true;
				}
			else if ( i != default_rule )
				{
				line_warning(
					_( "rule is in conflict with another rule" ),
					rule_linenum[i] );
				line_warning(
					_( "this is the conflicting rule" ),
					rule_linenum[dfaacc[dnew[j]].dfaacc_state] );
				}
		}

	flex_free( dnew );
	}


/* do_subset_sort_set - calculate the set of rules to be accepted by a state
 *			as indicated by the subset rule.
 *
 * synopsis
 *     void do_subset_sort_set();
 *
 * Special handling is provided for the default rule so that appropriate
 * warnings can be issued or suppressed as needed.
 */

static void do_subset_sort_set()
	{
	int i, j, k, inserted, *rules;

	rules = allocate_integer_array( num_rules + 1 );

	/* By using an insertion sort to build up the rules array,
	 * we can determine the last position at which a rule can
	 * appear without disobeying the subset rule.
	 */
	for ( i = 1; i <= num_rules; ++i )
		{
		inserted = false;
		for ( j = 1; j < i; ++j )
			if ( prefer( i, rules[j] ) )
				{
				for ( k = i; k > j; --k )
					rules[k] = rules[k - 1];
				rules[j] = i;
				inserted = true;
				break;
				}

		if ( ! inserted )
			rules[i] = i;
		}

	if ( trace )
		{
		fprintf( stderr,
			"\nSorted the rules into the following order:\n\t" );
		for ( i = 1; i <= num_rules; ++i )
			fprintf( stderr, "%d ", rules[i] );
		fprintf( stderr, "\n" );
		}

	for ( i = 1; i <= num_rules; ++i )
		{
		k = rules[i];
		if ( spprdflt && k == default_rule )
			continue;

		for ( j = 1; j <= dassnum[k]; ++j )
			dfaacc[dass[k][j]].dfaacc_set[++accsiz[dass[k][j]]] = k;
		}

	flex_free( rules );
	}


/* do_subset_sort - calculate the rule or set of rules (depending on whether
 *		    REJECT is used) to be accepted by a state as indicated by
 *		    the subset rule.
 *
 * synopsis
 *     void do_subset_sort();
 */

void do_subset_sort()
	{
	if ( reject )
		do_subset_sort_set();
	else
		do_subset_sort_state();
	}
