// Copyright © by Denys Duchier, Aug 1999, Universität des Saarlandes
// ===================================================================
//
// class MapOutputHandler implements an output handler for outputing
// textual data (not XML) where various characters may be variously
// escaped in various situations. oh hum...
//
// the basic idea is to be able to define named escape maps and to
// permit selection of a map to control escaping of characters for
// a particular scope.
//
// <!ELEMENT document (define*,(#PCDATA|use|alias)*)>
// <!ELEMENT define (escape|include)*>
// <!ELEMENT escape (#PCDATA)>
// <!ATTLIST escape char CDATA #REQUIRED>
// <!ELEMENT include EMPTY>
// <!ELEMENT (use|alias) (#PCDATA|use|alias)*>
// <!ATTLIST (define|include|use) name CDATA #REQUIRED>
// <!ATTLIST alias name CDATA #REQUIRED to CDATA #REQUIRED>
//
// <document>
//
//	is the outermost element and begins with a sequence of
// <define> elements: this is the only place where <define> elements
// may occur.
//
// <define name="foo"> ... </define>
//
//	defines a new escape map named "foo".  it consists of a
// sequence of <escape> and/or <include> elements which define
// substitution strings for certain characters.
//
// <escape char="$">\bslash </escape>
//
//	defines a substitution string for a character.  here it says
// that the dollar sign `$' must be replaced by `\bslash '.  if there
// are multiple <escape> elements for the same character, the last
// one wins.
//
// <include name="baz"/>
//
//	enters in the map being defined all the entries of the map
// named "baz".
//
// <use name="foo"> ... </use>
//
//	for the scope of this element, the escape map named "foo" will
// be in effect, unless overriden by a nested <use> element.
//
// <use> ... </use>
//
//	selects the default map which performs no escaping at all.
//
// <alias name="foo" to="baz"> ... </alias>
//
//	for the scope of this element, the name "foo" will denote
// the map that is currently denoted by name "baz" in the present
// scope.
//
// <foo> ... </foo>
//
//	for convenience <foo> is allowed instead of <use name="foo">
//
// <alias> is very useful in those situations where you want to
// reuse the same templates, but change the character escaping rules.
// For example this happens when converting to LaTeX: when
// generating \index{...} commands, it is necessary to additionally
// escape certain characters meaningful to makeindex, but you still
// want to use the same templates to process the contents.
//
// Why not allow to temporarily modify the current map? 2 reasons:
// first, this is hard to control predictably.  second, you may need
// modifications to more than one map.
// ===================================================================

import com.jclark.xsl.sax.*;
import org.xml.sax.*;
import java.io.Writer;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Hashtable;
import java.util.Iterator;

public class MapOutputHandler
    implements OutputDocumentHandler
{
    private Writer writer;
    private boolean keepOpen;

    // ===============================================================
    // CHARACTER HANDLERS -- output of textual data by means of the
    // SAX characters method is delegated to a handler object.  In
    // particular, there will be a distinct handler object for each
    // map created by a <define> element: it will replace certain
    // characters by strings according to its escape table.
    // ===============================================================

    // this is the base class for all character handlers.
    // it does not allow any character data and signals
    // an exception if any is encountered.

    class Handler {
	void characters(char[] ch,int off,int len)
	    throws IOException
	{
	    throw new IOException("text not allowed here");
	}
    }

    Handler noHandler = new Handler();

    // the default handler passes through all characters
    // unchanged. the reason for having a specialized default
    // handler instead of a MapHandler with and empty map
    // is just efficiency.

    class DefaultHandler extends Handler {
	// passes all characters unchanged
	void characters(char[] ch, int off,int len)
	    throws IOException
	{
	    writer.write(ch,off,len);
	}
    }

    Handler defaultHandler = new DefaultHandler();

    // the escape handler is used within the <escape> element
    // to record the translation of a character.  this is
    // why it simply appends characters to its buffer.  the
    // finish() method returns the contents of this buffer
    // and also resets it to empty.

    class EscapeHandler extends Handler {
	private StringBuffer buf = new StringBuffer();
	// records all characters into a buffer
	void characters(char[] ch,int off,int len)
	    throws IOException
	{
	    buf.append(ch,off,len);
	}
	String finish() {
	    String s = buf.toString();
	    buf.setLength(0);
	    return s;
	}
    }

    EscapeHandler escapeHandler = new EscapeHandler();

    // a MapHandler has a table that indicates how certain
    // characters must be translated into given strings.
    // other characters remain unchanged.

    class MapHandler extends Handler {
	private String[] table;
	MapHandler(String[] t) { table=t; }
	void characters(char[] ch,int off,int len)
	    throws IOException
	{
	    for(;len>0;off++,len--) {
		char c = ch[off];
		if (c < table.length) {
		    final String s = table[c];
		    if (s==null) writer.write(c);
		    else writer.write(s);
		} else
		    writer.write(c);
	    }
	}
    }

    // ===============================================================
    // ALIASES -- names are mapped to handlers through aliases
    // ===============================================================

    Hashtable alias_table;

    Handler lookupAlias(String n)
    {
	return (Handler) alias_table.get(n);
    }

    Handler getAlias(String n) throws SAXException
    {
	Handler h = lookupAlias(n);
	if (h==null) error("no such map: "+n);
	return h;
    }

    void setAlias(String n,Handler h) {
	alias_table.put(n,h);
    }

    // ===============================================================
    // TRAIL -- bindings of aliases maybe temporarily changed, then
    // need to be reverted to their previous value.  thus whenever
    // aliasing is performed, we trail the previous binding of the
    // name in order to restore it on exit.
    // ===============================================================

    class TrailEntry {
	String name;
	Handler handler;
	TrailEntry next;
	TrailEntry(String s,Handler h,TrailEntry n)
	{
	    name=s; handler=h; next=n;
	}
    }

    TrailEntry trail_top;

    void trail(String n) {
	trail_top = new TrailEntry(n,lookupAlias(n),trail_top);
    }

    void untrail() {
	setAlias(trail_top.name,trail_top.handler);
	trail_top = trail_top.next;
    }

    // ===============================================================
    // ESCAPE MAPS -- escape maps are built while processing <define>
    // elements at the front of the document.  when we switch to
    // processing the body, escape maps are then `evaled' to convert
    // them into handlers and install initial bindings of aliases.
    // ===============================================================

    Hashtable escape_table;

    EscapeMap lookupMap(String n) {
	return (EscapeMap) escape_table.get(n);
    }

    EscapeMap getMap(String n) throws SAXException
    {
	EscapeMap map = lookupMap(n);
	if (map==null) error("no such map: "+n);
	return map;
    }

    void setMap(EscapeMap map) {
	escape_table.put(map.name,map);
    }

    void enterMap(EscapeMap map) throws SAXException {
	if (escape_table.containsKey(map.name))
	    error("attempt to redefine map: "+map.name);
	escape_table.put(map.name,map);
    }

    // due to the <include> element, one map may include another and
    // they may do so in arbitrary order.  this means that the
    // computation of the actual escape map must be delayed until
    // all <define>s at the front of the document have been processed.
    // at that point, the maps are `evaled' using eval() which
    // computes the complete table of a map.  this may involve
    // recursively computing other maps, due to <include>s. we check
    // for circularities by setting a map's status to EVAL_RUNNING
    // while it is being evaled.  If we ever encounter a map whose
    // status is EVAL_RUNNING, then we know that we have hit a
    // circularity.  the real work is being done by internalEval()
    // which does not check for circularity but must call eval()
    // to recurse.

    final static int EVAL_NONE    = 0;
    final static int EVAL_RUNNING = 1;
    final static int EVAL_DONE    = 2;

    class EscapeMap {
	protected String name;
	protected int status = EVAL_NONE;

	SimpleEscapeMap eval() throws SAXException
	{
	    if (status==EVAL_DONE) return (SimpleEscapeMap) this;
	    if (status==EVAL_RUNNING) error("map circularity: "+name);
	    status = EVAL_RUNNING;
	    SimpleEscapeMap map = internalEval();
	    status = EVAL_DONE;
	    return map;
	}

	SimpleEscapeMap internalEval() throws SAXException
	{
	    throw new SAXException("this should never happen (1)");
	}

	// this is a version of eval() that also updates the
	// hash table mapping names to escape maps

	SimpleEscapeMap topEval() throws SAXException
	{
	    if (status==EVAL_DONE) return (SimpleEscapeMap) this;
	    SimpleEscapeMap map = eval();
	    setMap(map);
	    return map;
	}
    }

    // a SimpleEscapeMap is just a table mapping characters to
    // strings (or null, if no escape string was entered for that
    // character).

    class SimpleEscapeMap extends EscapeMap {
	protected String table[] = new String[128];

	public SimpleEscapeMap(String n) { name=n; }

	void enter(char c,String s)
	{
	    if (c>=table.length) {
		int len = table.length;
		do { len*=2; } while (len<=c);
		String[] new_table = new String[len];
		System.arraycopy(table,0,new_table,0,table.length);
		table=new_table;
	    }
	    table[c] = s;
	}

	SimpleEscapeMap internalEval() throws SAXException
	{ return this; }
    }

    // an IncludeEscapeMap allows to delay the inclusion of another
    // map.  The map that was being accumulated is saved in previous.
    // the name of the map to be include is saved in included.
    // local escape elements enter info in the IncludeEscapeMap's
    // local table. calling internalEval() merges the tables, where
    // later definitions override.

    class IncludeEscapeMap extends SimpleEscapeMap {
	private EscapeMap previous;
	private String included;
	public IncludeEscapeMap(EscapeMap prev,String inc) {
	    super(prev.name);
	    previous = prev;
	    included = inc;
	}
	SimpleEscapeMap internalEval()
	    throws SAXException
	{
	    SimpleEscapeMap map = previous.eval();
	    String[] tbl = getMap(included).topEval().table;
	    for (int i=tbl.length-1;i>=0;i--)
		if (tbl[i]!=null) map.enter((char)i,tbl[i]);
	    tbl = table;
	    for (int i=tbl.length-1;i>=0;i--)
		if (tbl[i]!=null) map.enter((char)i,tbl[i]);
	    return map;
	}
    }

    // ===============================================================
    // create handlers and install initial bindings for aliases
    // by going through the known maps and evaling them.
    // ===============================================================

    void installHandlers() throws SAXException
    {
	Iterator i = escape_table.values().iterator();
	while (i.hasNext()) {
	    EscapeMap map = (EscapeMap) i.next();
	    setAlias(map.name,new MapHandler(map.topEval().table));
	}
	escape_table = null;
    }

    // ===============================================================
    // SAX API -- the output handler is written to be very tighed
    // assed about the DTD.  if an element appears in the wrong place,
    // an exception is raised.  there are 4 types of contexts: (1)
    // CTX_TOP is outside the <document> element.  nothing is allowed
    // at the top, except the <document> element. (2) CTX_FRONT
    // consists of the front children of the <document> element.  Only
    // children of type <define> are allowed.  (3) CTX_BODY for
    // anything that appears after the front children.  (4) CTX_DEFINE
    // is for <define> elements: nothing is allowed except <escape>
    // elements.  (5) CTX_ESCAPE is for <escape> elements. nothing but
    // characters is allowed here.
    // ===============================================================

    private static final int CTX_TOP	= 0;
    private static final int CTX_FRONT	= 1;
    private static final int CTX_BODY	= 2;
    private static final int CTX_DEFINE	= 3;
    private static final int CTX_ESCAPE	= 4;

    private int context;
    private Handler handler;

    void enterBody() throws SAXException
    {
	context = CTX_BODY;
	installHandlers();
	handler = defaultHandler;
    }

    public MapOutputHandler() {}

    public void startDocument()
    {
	pushHandler(noHandler);
	context      = CTX_TOP;
	alias_table  = new Hashtable(20);
	escape_table = new Hashtable(20);
	trail_top    = null;
    }

    public void endDocument() throws SAXException
    {
	popHandler();
	try {
	    if (writer!=null) {
		if (keepOpen)
		    writer.flush();
		else
		    writer.close();
		writer=null;
	    }
	}
	catch (IOException e) { throw new SAXException(e); }
	alias_table  = null;
	escape_table = null;
	trail_top    = null;	// should be null anyway
    }

    public void processingInstruction(String name,String data)
	throws SAXException
    {
	error("processing instructions not allowed: <?"+name+" "+data+"?>");
    }

    public void characters(char[] ch,int off,int len)
	throws SAXException
    {
	if (context==CTX_FRONT) enterBody();
	try { handler.characters(ch,off,len); }
	catch (IOException e) { throw new SAXException(e); }
    }

    public void ignorableWhitespace(char[] ch,int off,int len)
	throws SAXException
    {
	characters(ch,off,len);
    }

    void debug(String s){System.err.println(s);}
    public void startElement(String name,AttributeList atts)
	throws SAXException
    {
	// SAX requires the namespace prefix to be still
	// attached.  This is completely inappropriate for
	// output handlers and extension elements it general
	// since the namespace prefix is chosen by the style
	// sheet and cannot be known to the implementation
	// of the handler or extension element.  Thus we
	// must skip it again here.
	int off = name.indexOf(':');
	if (off>=0) name = name.substring(off+1);
	if      (name.equals("document")) stagDocument(atts);
	else if (name.equals("define"  )) stagDefine  (atts);
	else if (name.equals("include" )) stagInclude (atts);
	else if (name.equals("escape"  )) stagEscape  (atts);
	else if (name.equals("use"     )) stagUse     (atts);
	else if (name.equals("alias"   )) stagAlias   (atts);
	else stag(name,atts);
    }

    public void endElement(String name)
	throws SAXException
    {
	int off = name.indexOf(':');
	if (off>=0) name = name.substring(off+1);
	if      (name.equals("document")) etagDocument();
	else if (name.equals("define"  )) etagDefine  ();
	else if (name.equals("include" )) etagInclude ();
	else if (name.equals("escape"  )) etagEscape  ();
	else if (name.equals("use"     )) etagUse     ();
	else if (name.equals("alias"   )) etagAlias   ();
	else etag(name);
    }

    void error(String msg) throws SAXException
    {
	throw new SAXException(msg);
    }

    void notHere(String name) throws SAXException
    {
	error("element not allowed here: "+name);
    }

    String getReq(String elt,String name,AttributeList atts)
	throws SAXException
    {
	String val = atts.getValue(name);
	if (val==null) error(elt+": `"+name+"' attribute mandatory");
	return val;
    }
    
    void stag(String name,AttributeList atts)
	throws SAXException
    {
	if (context==CTX_FRONT) enterBody();
	else if (context!=CTX_BODY) notHere(name);
	pushHandler(getAlias(name));
    }
	
    void etag(String name) { popHandler(); }

    void stagDocument(AttributeList atts)
	throws SAXException
    {
	if (context!=CTX_TOP) notHere("document");
	context=CTX_FRONT;
	pushHandler(defaultHandler);
    }

    void etagDocument() { popHandler(); }

    SimpleEscapeMap defmap;

    void stagDefine(AttributeList atts)
	throws SAXException
    {
	if (context!=CTX_FRONT) notHere("define");
	String defname = getReq("define","name",atts);
	context = CTX_DEFINE;
	defmap  = new SimpleEscapeMap(defname);
	pushHandler(noHandler);
    }

    void etagDefine() throws SAXException {
	enterMap(defmap);
	defmap  = null;
	context = CTX_FRONT;
    }

    private char escapeChar;

    void stagEscape(AttributeList atts)
	throws SAXException
    {
	if (context!=CTX_DEFINE) notHere("escape");
	context = CTX_ESCAPE;
	String val = getReq("escape","char",atts);
	if (val.length()!=1)
	    error("escape: `char' attribute value must be string of length 1");
	escapeChar = val.charAt(0);
	pushHandler(escapeHandler);
    }

    void etagEscape()
    {
	defmap.enter(escapeChar,escapeHandler.finish());
	context = CTX_DEFINE;
	popHandler();
    }

    void stagInclude(AttributeList atts)
	throws SAXException
    {
	if (context!=CTX_DEFINE) notHere("include");
	String incname = getReq("include","name",atts);
	defmap = new IncludeEscapeMap(defmap,incname);
	pushHandler(noHandler);	// <include> element should be empty
    }

    void etagInclude() { popHandler(); }
    
    void stagUse(AttributeList atts)
	throws SAXException
    {
	if (context==CTX_FRONT) enterBody();
	else if (context!=CTX_BODY) notHere("use");
	String usename = atts.getValue("name");
	pushHandler((usename==null || usename.equals(""))
		    ? defaultHandler : getAlias(usename));
    }

    void etagUse() throws SAXException { popHandler(); }

    void stagAlias(AttributeList atts)
	throws SAXException
    {
	String aliasname = getReq("alias","name",atts);
	String toname    = getReq("alias","to"  ,atts);
	Handler tohandler= getAlias(toname);
	trail(aliasname);
	setAlias(aliasname,tohandler);
    }

    void etagAlias() { untrail(); }

    // ===============================================================
    // HANDLER STACK -- the stack represent the nesting of <use>
    // elements.  only the handler corresponding to the innermost
    // alias is in effect.
    // ===============================================================

    private Handler[] handler_stack = new Handler[20];
    private int handler_index = 0;

    void popHandler() {
	handler = handler_stack[--handler_index];
    }

    void pushHandler(Handler h) {
	if (handler_index>=handler_stack.length) {
	    Handler[] new_handler_stack = new Handler[2*handler_stack.length];
	    System.arraycopy(handler_stack,0,new_handler_stack,0,handler_stack.length);
	    handler_stack=new_handler_stack;
	}
	handler_stack[handler_index++] = handler;
	handler = h;
    }

    // ===============================================================
    // MAGIC STUFF
    // ===============================================================

    public DocumentHandler init(Destination dest,AttributeList atts)
	throws IOException
    {
	String mediaType = atts.getValue("media-type");
	if (mediaType==null) mediaType="text/plain";
	writer = new BufferedWriter
	    (dest.getWriter(mediaType,atts.getValue("encoding")));
	keepOpen = dest.keepOpen();
	return this;
    }

    public void setDocumentLocator(org.xml.sax.Locator loc) { }
}
