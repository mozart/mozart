# Author: David Goodger
# Contact: goodger@users.sourceforge.net
# Revision: $Revision$
# Date: $Date$
# Copyright: This module has been placed in the public domain.

"""
Mozart Enhancement Proposal (MEP) Reader.
"""

__docformat__ = 'reStructuredText'


from docutils.readers import standalone
import meps
from docutils.transforms import references
from docutils.parsers import rst


class Inliner(rst.states.Inliner):

    """
    Extend `rst.Inliner` for local MEP references.
    """

    mep_url = rst.states.Inliner.pep_url


class Reader(standalone.Reader):

    supported = ('mep',)
    """Contexts this reader supports."""

    settings_spec = (
        'MEP Reader Option Defaults',
        'The --mep-references and --rfc-references options (for the '
        'reStructuredText parser) are on by default.',
        ())

    default_transforms = (references.Substitutions,
                          meps.Headers,
                          meps.Contents,
                          references.ChainedTargets,
                          references.AnonymousHyperlinks,
                          references.IndirectHyperlinks,
                          meps.TargetNotes,
                          references.Footnotes,
                          references.ExternalTargets,
                          references.InternalTargets,)

    settings_default_overrides = {'mep_references': 1, 'rfc_references': 1}

    inliner_class = Inliner

    def __init__(self, parser=None, parser_name=None):
        """`parser` should be ``None``."""
        if parser is None:
            parser = rst.Parser(rfc2822=1, inliner=self.inliner_class())
        standalone.Reader.__init__(self, parser, '')
