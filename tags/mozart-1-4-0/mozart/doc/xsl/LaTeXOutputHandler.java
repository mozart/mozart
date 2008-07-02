package com.jclark.xsl.sax;

import org.xml.sax.*;

import java.io.Writer;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

public class LaTeXOutputHandler
    implements DocumentHandler, OutputStreamConverter
{
    private boolean skipeoln;

    class CharacterHandler {
	void characters(char[] ch, int off, int len) throws IOException { }
    }

    class EolnCharacterHandler extends CharacterHandler {
	void characters(char[] ch, int off, int len) throws IOException	{
	    if (len > 0) throw new IOException("chars in eoln element");
	    skipeoln = true;
	}
    }

    class CodeCharacterHandler extends CharacterHandler {
	void characters(char[] ch, int off, int len) throws IOException {
	    writer.write(ch,off,len);
	    skipeoln = false;
	}
    }

    class TextCharacterHandler extends CharacterHandler {
	private StringBuffer buf = new StringBuffer();
	void characters(char[] ch, int off, int len) throws IOException {
	    for(int i=0;i<len;i++) {
		char c = ch[i];
		if (c == '\n') {
		    if (! skipeoln) {
			skipeoln = true;
			buf.append(c);
		    }
		} else if (c=='#' || c=='$' || c=='%' ||
			   c=='&' || c=='~' || c=='_' ||
			   c=='^' || c=='\\'|| c=='{' ||
			   c=='}' || c=='<' || c=='>' ||
			   c=='|' ) {
		    buf.append("\\usechar{\\").append(c).append("}");
		    skipeoln = false;
		}
		else {
		    buf.append(c);
		    skipeoln = false;
		}
	    }
	    writer.write(buf.toString(),0,buf.length());
	    buf.setLength(0);
	}
    }

    class VerbCharacterHandler extends CharacterHandler {
	private StringBuffer buf = new StringBuffer();
	void characters(char[] ch, int off, int len) throws IOException {
	    for(int i=0;i<len;i++) {
		char c = ch[i];
		if (c=='#' || c=='$' || c=='%' ||
		    c=='&' || c=='~' || c=='_' ||
		    c=='^' || c=='\\'|| c=='{' ||
		    c=='}' || c=='<' || c=='>' ||
		    c=='|' ) {
		    buf.append("\\usevchar{\\").append(c).append("}");
		}
		else buf.append(c);
	    }
	    writer.write(buf.toString(),0,buf.length());
	    buf.setLength(0);
	    skipeoln = false;
	}
    }


    private Writer writer;
    private OutputStream out;

    private final CharacterHandler eolnCharacterHandler
	= new EolnCharacterHandler();
    private final CharacterHandler textCharacterHandler
	= new TextCharacterHandler();
    private final CharacterHandler verbCharacterHandler
	= new VerbCharacterHandler();
    private final CharacterHandler codeCharacterHandler
	= new CodeCharacterHandler();

    private CharacterHandler characterHandler;
    private int depth = 0;
    private CharacterHandler[] characterHandlers = new CharacterHandler[1];

    public LaTeXOutputHandler() {
	characterHandler = textCharacterHandler;
	skipeoln = true;
    }

    public void setOutputStream(OutputStream out) {
	this.out = out;
    }

    public void startDocument() { }

    public void characters(char[] ch, int off, int len)
	throws SAXException
    {
	if (writer == null)
	    writer = new BufferedWriter(new OutputStreamWriter(out));
	try {
	    characterHandler.characters(ch, off, len);
	}
	catch (IOException e) {
	    throw new SAXException(e);
	}
    }

    public void ignorableWhitespace(char[] ch, int off, int len)
	throws SAXException
    {
	characters(ch, off, len);
    }

    public void startElement(String name, AttributeList atts)
	throws SAXException
    {
	if (name.equals("document"))
	    push(textCharacterHandler);
	else if (name.equals("text"))
	    push(textCharacterHandler);
	else if (name.equals("verb"))
	    push(verbCharacterHandler);
	else if (name.equals("eoln")) {
	    try { writer.write('\n'); }
	    catch (IOException e) {
		throw new SAXException(e);
	    }
	    skipeoln = true;
	    push(eolnCharacterHandler);
	}
	else if (name.equals("code") || name.equals("latex"))
	    push(codeCharacterHandler);
	else
	    throw new SAXException("unexpected element: "+name);
    }

    public void endElement(String name) throws SAXException {
	pop();
    }

    public void pop() {
	characterHandler = characterHandlers[--depth];
    }

    public void push(CharacterHandler handler) {
	if (depth >= characterHandlers.length) {
	    CharacterHandler[] oldHandlers = characterHandlers;
	    characterHandlers = new CharacterHandler[oldHandlers.length * 2];
	    System.arraycopy(oldHandlers, 0, characterHandlers, 0,
			     oldHandlers.length);
	}
	characterHandlers[depth++] = characterHandler;
	characterHandler = handler;
    }

    public void processingInstruction(String target, String data) { }

    public void endDocument() throws SAXException {
	try {
	    if (writer != null)
		writer.close();
	    else
		out.close();
	}
	catch (IOException e) {
	    throw new SAXException(e);
	}
    }

    public void setDocumentLocator(org.xml.sax.Locator loc) { }
}
