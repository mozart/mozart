# Author: David Goodger
# Contact: goodger@users.sourceforge.net
# Revision: $Revision$
# Date: $Date$
# Copyright: This module has been placed in the public domain.

"""
Transforms for MEP processing.

- `Headers`: Used to transform a MEP's initial RFC-2822 header.  It remains a
  field list, but some entries get processed.
- `Contents`: Auto-inserts a table of contents.
- `MEPZero`: Special processing for MEP 0.
"""

__docformat__ = 'reStructuredText'

import sys
import os
import re
import time
from docutils import nodes, utils, languages
from docutils import ApplicationError, DataError
from docutils.transforms import Transform, TransformError
from docutils.transforms import parts, references, misc


class Headers(Transform):

    """
    Process fields in a MEP's initial RFC-2822 header.
    """

    default_priority = 360

    mep_url = 'mep-%04d.html'
    mep_cvs_url = ('http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/python/'
                   'python/nondist/meps/mep-%04d.txt')
    rcs_keyword_substitutions = (
          (re.compile(r'\$' r'RCSfile: (.+),v \$$', re.IGNORECASE), r'\1'),
          (re.compile(r'\$[a-zA-Z]+: (.+) \$$'), r'\1'),)

    def apply(self):
        if not len(self.document):
            # @@@ replace these DataErrors with proper system messages
            raise DataError('Document tree is empty.')
        header = self.document[0]
        if not isinstance(header, nodes.field_list) or \
              header.get('class') != 'rfc2822':
            raise DataError('Document does not begin with an RFC-2822 '
                            'header; it is not a MEP.')
        mep = None
        for field in header:
            if field[0].astext().lower() == 'mep': # should be the first field
                value = field[1].astext()
                try:
                    mep = int(value)
                    cvs_url = self.mep_cvs_url % mep
                except ValueError:
                    mep = value
                    cvs_url = None
                    msg = self.document.reporter.warning(
                        '"MEP" header must contain an integer; "%s" is an '
                        'invalid value.' % mep, base_node=field)
                    msgid = self.document.set_id(msg)
                    prb = nodes.problematic(value, value or '(none)',
                                            refid=msgid)
                    prbid = self.document.set_id(prb)
                    msg.add_backref(prbid)
                    if len(field[1]):
                        field[1][0][:] = [prb]
                    else:
                        field[1] += nodes.paragraph('', '', prb)
                break
        if mep is None:
            raise DataError('Document does not contain an RFC-2822 "MEP" '
                            'header.')
        if mep == 0:
            # Special processing for MEP 0.
            pending = nodes.pending(MEPZero)
            self.document.insert(1, pending)
            self.document.note_pending(pending)
        if len(header) < 2 or header[1][0].astext().lower() != 'title':
            raise DataError('No title!')
        for field in header:
            name = field[0].astext().lower()
            body = field[1]
            if len(body) > 1:
                raise DataError('MEP header field body contains multiple '
                                'elements:\n%s' % field.pformat(level=1))
            elif len(body) == 1:
                if not isinstance(body[0], nodes.paragraph):
                    raise DataError('MEP header field body may only contain '
                                    'a single paragraph:\n%s'
                                    % field.pformat(level=1))
            elif name == 'last-modified':
                date = time.strftime(
                      '%d-%b-%Y',
                      time.localtime(os.stat(self.document['source'])[8]))
                if cvs_url:
                    body += nodes.paragraph(
                        '', '', nodes.reference('', date, refuri=cvs_url))
            else:
                # empty
                continue
            para = body[0]
            if name == 'author':
                for node in para:
                    if isinstance(node, nodes.reference):
                        node.parent.replace(node, mask_email(node))
            elif name == 'discussions-to':
                for node in para:
                    if isinstance(node, nodes.reference):
                        node.parent.replace(node, mask_email(node, mep))
            elif name in ('replaces', 'replaced-by', 'requires'):
                newbody = []
                space = nodes.Text(' ')
                for refmep in re.split(',?\s+', body.astext()):
                    mepno = int(refmep)
                    newbody.append(nodes.reference(
                          refmep, refmep, refuri=self.mep_url % mepno))
                    newbody.append(space)
                para[:] = newbody[:-1] # drop trailing space
            elif name == 'last-modified':
                utils.clean_rcs_keywords(para, self.rcs_keyword_substitutions)
                if cvs_url:
                    date = para.astext()
                    para[:] = [nodes.reference('', date, refuri=cvs_url)]
            elif name == 'content-type':
                mep_type = para.astext()
                uri = self.mep_url % 2
                para[:] = [nodes.reference('', mep_type, refuri=uri)]
            elif name == 'version' and len(body):
                utils.clean_rcs_keywords(para, self.rcs_keyword_substitutions)


class Contents(Transform):

    """
    Insert an empty table of contents topic and a transform placeholder into
    the document after the RFC 2822 header.
    """

    default_priority = 380

    def apply(self):
        language = languages.get_language(self.document.settings.language_code)
        name = language.labels['contents']
        title = nodes.title('', name)
        topic = nodes.topic('', title, CLASS='contents')
        name = nodes.fully_normalize_name(name)
        if not self.document.has_name(name):
            topic['name'] = name
        self.document.note_implicit_target(topic)
        pending = nodes.pending(parts.Contents)
        topic += pending
        self.document.insert(1, topic)
        self.document.note_pending(pending)


class TargetNotes(Transform):

    """
    Locate the "References" section, insert a placeholder for an external
    target footnote insertion transform at the end, and schedule the
    transform to run immediately.
    """

    default_priority = 520

    def apply(self):
        doc = self.document
        i = len(doc) - 1
        refsect = copyright = None
        while i >= 0 and isinstance(doc[i], nodes.section):
            title_words = doc[i][0].astext().lower().split()
            if 'references' in title_words:
                refsect = doc[i]
                break
            elif 'copyright' in title_words:
                copyright = i
            i -= 1
        if not refsect:
            refsect = nodes.section()
            refsect += nodes.title('', 'References')
            doc.set_id(refsect)
            if copyright:
                # Put the new "References" section before "Copyright":
                doc.insert(copyright, refsect)
            else:
                # Put the new "References" section at end of doc:
                doc.append(refsect)
        pending = nodes.pending(references.TargetNotes)
        refsect.append(pending)
        self.document.note_pending(pending, 0)
        pending = nodes.pending(misc.CallBack,
                                details={'callback': self.cleanup_callback})
        refsect.append(pending)
        self.document.note_pending(pending, 1)

    def cleanup_callback(self, pending):
        """
        Remove an empty "References" section.

        Called after the `references.TargetNotes` transform is complete.
        """
        if len(pending.parent) == 2:    # <title> and <pending>
            pending.parent.parent.remove(pending.parent)


class MEPZero(Transform):

    """
    Special processing for MEP 0.
    """

    default_priority =760

    def apply(self):
        visitor = MEPZeroSpecial(self.document)
        self.document.walk(visitor)
        self.startnode.parent.remove(self.startnode)


class MEPZeroSpecial(nodes.SparseNodeVisitor):

    """
    Perform the special processing needed by MEP 0:

    - Mask email addresses.

    - Link MEP numbers in the second column of 4-column tables to the MEPs
      themselves.
    """

    mep_url = Headers.mep_url

    def unknown_visit(self, node):
        pass

    def visit_reference(self, node):
        node.parent.replace(node, mask_email(node))

    def visit_field_list(self, node):
        if node.hasattr('class') and node['class'] == 'rfc2822':
            raise nodes.SkipNode

    def visit_tgroup(self, node):
        self.mep_table = node['cols'] == 4
        self.entry = 0

    def visit_colspec(self, node):
        self.entry += 1
        if self.mep_table and self.entry == 2:
            node['class'] = 'num'

    def visit_row(self, node):
        self.entry = 0

    def visit_entry(self, node):
        self.entry += 1
        if self.mep_table and self.entry == 2 and len(node) == 1:
            node['class'] = 'num'
            p = node[0]
            if isinstance(p, nodes.paragraph) and len(p) == 1:
                text = p.astext()
                try:
                    mep = int(text)
                    ref = self.mep_url % mep
                    p[0] = nodes.reference(text, text, refuri=ref)
                except ValueError:
                    pass


non_masked_addresses = ('meps@python.org',
                        'python-list@python.org',
                        'python-dev@python.org')

def mask_email(ref, mepno=None):
    """
    Mask the email address in `ref` and return a replacement node.

    `ref` is returned unchanged if it contains no email address.

    For email addresses such as "user@host", mask the address as "user at
    host" (text) to thwart simple email address harvesters (except for those
    listed in `non_masked_addresses`).  If a MEP number (`mepno`) is given,
    return a reference including a default email subject.
    """
    if ref.hasattr('refuri') and ref['refuri'].startswith('mailto:'):
        if ref['refuri'][8:] in non_masked_addresses:
            replacement = ref[0]
        else:
            replacement_text = ref.astext().replace('@', '&#32;&#97;t&#32;')
            replacement = nodes.raw('', replacement_text, format='html')
        if mepno is None:
            return replacement
        else:
            ref['refuri'] += '?subject=MEP%%20%s' % mepno
            ref[:] = [replacement]
            return ref
    else:
        return ref
