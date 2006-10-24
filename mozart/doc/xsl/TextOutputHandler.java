package com.jclark.xsl.sax;

import org.xml.sax.*;

import java.io.Writer;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

public class TextOutputHandler implements DocumentHandler, OutputStreamConverter {

    private Writer writer;
    private OutputStream out;

    class CharacterHandler {
	// default is not to allow characters
	void characters(char[] ch,int off,int len) throws IOException {
	    throw new IOException("text not allowed here");
	}
    }

    class DefaultCharacterHandler extends CharacterHandler {
	// default document handler just passes through all characters unchanged
	void characters(char[] ch,int off,int len) throws IOException {
	    writer.write(ch,off,len);
	}
    }

    class EnterCharacterHandler extends CharacterHandler {
	private char c;
	private StringBuffer buf = new StringBuffer();
	// <enter char="x">...</enter> records characters in a string buffer
	void characters(char[] ch,int off,int len) throws IOException {
	    buf.append(ch,off,len);
	}
	void beginDefinition(char ch) { c=ch; }
	void endDefinition() {
	    defmap.enter(c,buf.toString());
	    buf.setLength(0);
	}
    }

    private UsemapCharacterHandler first_map = null;
    private UsemapCharacterHandler lookup(String name) throws SAXException {
	for(UsemapCharacterHandler h=first_map;h!=null;h=h.next)
	    if (h.name==name) return h.deref();
	throw new SAXException("no such map: "+name);
    }

    class UsemapCharacterHandler extends CharacterHandler {
	private String name;
	private UsemapCharacterHandler next;
	private UsemapCharacterHandler alias;

	UsemapCharacterHandler(String n) { name=n; }

	UsemapCharacterHandler deref() {
	    return (alias==null)?this:alias;
	}

	void record() throws SAXException {
	    for(UsemapCharacterHandler h=first_map;h!=null;h=h.next)
		if (h.name==name)
		    throw new SAXException("map already exists: "+name);
	    next=first_map; first_map=this;
	}

	private String table[] = new String[128];

	void enter(char c,String s) {
	    if (c>=table.length) {
		int len = table.length;
		do { len *= 2; } while (len<=c);
		String[] new_table = new String[len];
		System.arraycopy(table,0,new_table,0,table.length);
		table=new_table;
	    }
	    table[c] = s;
	}

	// <usemap name="...">...</usemap> installs the named map which
	// translates characters which have been entered into the map
	void characters(char[] ch,int off,int len) throws IOException {
	    for(;len>0;off++,len--) {
		char c = ch[off];
		if (c < table.length) {
		    final String s = table[c];
		    if (s==null) writer.write(c);
		    else writer.write(s);
		} else {
		    writer.write(c);
		}
	    }
	}
    }

    private CharacterHandler characterHandler;
    private final CharacterHandler noCharacterHandler = new CharacterHandler();
    private final CharacterHandler defaultCharacterHandler = new DefaultCharacterHandler();
    private final EnterCharacterHandler enterCharacterHandler = new EnterCharacterHandler();

    private static final int ContextNone     = 0;
    private static final int ContextDocument = 1;
    private static final int ContextDefmap   = 2;
    private static final int ContextEnter    = 3;

    private int context;
    private UsemapCharacterHandler defmap;

    public TextOutputHandler() {
	characterHandler = noCharacterHandler;
    }

    public void startDocument() {
	writer = new BufferedWriter(new OutputStreamWriter(out));
	context = ContextNone;
    }

    public void endDocument() throws SAXException {
	try { writer.close(); }
	catch (IOException e) { throw new SAXException(e); }
    }

    public void processingInstruction(String name,String data) throws SAXException {
	throw new SAXException("processing instructions not allowed: <?"+name+" "+data+"?>");
    }

    public void setDocumentLocator(org.xml.sax.Locator loc) { }

    public void characters(char[] ch,int off,int len) throws SAXException {
	try { characterHandler.characters(ch,off,len); }
	catch (IOException e) { throw new SAXException(e); }
    }

    public void ignorableWhitespace(char[] ch,int off,int len) throws SAXException {
	characters(ch,off,len);
    }

    public void startElement(String name,AttributeList atts) throws SAXException {
	if (name.equals("document")) {
	    if (context!=ContextNone)
		throw new SAXException("element not allowed here: document");
	    context = ContextDocument;
	    push(defaultCharacterHandler);
	}
	else if (name.equals("defmap")) {
	    if (context!=ContextDocument)
		throw new SAXException("element not allowed here: defmap");
	    String mapname = atts.getValue("name");
	    if (name==null)
		throw new SAXException("defmap: name attribute mandatory");
	    context = ContextDefmap;
	    defmap = new UsemapCharacterHandler(mapname);
	    defmap.record();
	    push(noCharacterHandler);
	}
	else if (name.equals("enter")) {
	    if (context!=ContextDefmap)
		throw new SAXException("element not allowed here: enter");
	    String str = atts.getValue("char");
	    if (str==null)
		throw new SAXException("enter: char attribute mandatory");
	    if (str.length()!=1)
		throw new SAXException("enter: char attribute value must be string of length 1");
	    enterCharacterHandler.beginDefinition(str.charAt(0));
	    push(enterCharacterHandler);
	}
	else if (name.equals("usemap")) {
	    if (context!=ContextDocument)
		throw new SAXException("element not allowed here: usemap");
	    String mapname = atts.getValue("name");
	    push((mapname==null || mapname.length()==0)
		 ? defaultCharacterHandler : lookup(mapname));
	}
	else if (name.equals("alias")) {
	    if (context!=ContextDocument)
		throw new SAXException("element not allowed here: alias");
	    String fromname = atts.getValue("from");
	    if (fromname==null)
		throw new SAXException("alias: from attribute mandatory");
	    String toname = atts.getValue("to");
	    if (toname==null)
		throw new SAXException("alias: to attribute mandatory");
	    UsemapCharacterHandler frommap = lookup(fromname);
	    UsemapCharacterHandler tomap = lookup(toname);
	    trail(frommap);
	    frommap.alias = tomap;
	}
	else
	    throw new SAXException("unknown element: "+name);
    }

    public void endElement(String name) throws SAXException {
	if (name.equals("document")) context=ContextNone;
	else if (name.equals("enter")) {
	    enterCharacterHandler.endDefinition();
	    context=ContextDefmap;
	}
	else context=ContextDocument;
	if (name.equals("alias")) untrail();
	else pop();
    }

    private CharacterHandler[] stack = new CharacterHandler[100];
    private int depth=0;

    void pop() { characterHandler = stack[--depth]; }

    void push(CharacterHandler h) {
	if (depth>=stack.length) {
	    CharacterHandler[] new_stack = new CharacterHandler[2*stack.length];
	    System.arraycopy(stack,0,new_stack,0,stack.length);
	    stack=new_stack;
	}
	stack[depth++] = characterHandler;
	characterHandler = h;
    }

    public void setOutputStream(OutputStream out) { this.out = out; }

    private TrailEntry trail_top;

    class TrailEntry {
	private UsemapCharacterHandler map;
	private UsemapCharacterHandler was;
	private TrailEntry next;
	TrailEntry(UsemapCharacterHandler m) {
	    map = m;
	    was = m.alias;
	}
	void link() {
	    next = trail_top;
	    trail_top = this;
	}
	void redo() {
	    map.alias = was;
	    trail_top = next;
	}
    }

    void trail(UsemapCharacterHandler m) {
	(new TrailEntry(m)).link();
    }
    void untrail() { trail_top.redo(); }
}
