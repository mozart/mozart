package com.jclark.xsl.sax;

import org.xml.sax.*;

import java.io.Writer;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

public class NXMLOutputHandler implements DocumentHandler, OutputStreamConverter {

  class CharacterHandler {
    void characters(char[] ch, int off, int len) throws IOException { }
  }

  class EscapeCharacterHandler extends CharacterHandler {
    private int c;
    private StringBuffer buf = new StringBuffer();

    void characters(char[] ch, int off, int len) {
      for (; len > 0; off++, len--) {
	if (ch[off] == '\n')
	  buf.append(lineSeparator);
	else
	  buf.append(ch[off]);
      }
    }

    void setChar(String str) {
      buf.setLength(0);
      if (str.length() == 0)
	c = -1;
      else
	c = str.charAt(0);
    }

    void endChar() {
      if (c >= 0) {
	if (c >= escapes.length) {
	  int newLen = escapes.length;
	  do {
	    newLen *= 2;
	  } while (newLen <= c);
	  String[] oldEscapes = escapes;
	  escapes = new String[newLen];
	  System.arraycopy(oldEscapes, 0, escapes, 0, oldEscapes.length);
	}
	escapes[c] = buf.toString();
      }
    }
  }

  class DataCharacterHandler extends CharacterHandler {
    void characters(char[] ch, int off, int len) throws IOException {
      final String[] escapes = NXMLOutputHandler.this.escapes;
      int start = off;
      for (; len > 0; off++, len--) {
	char c = ch[off];
	if (c < escapes.length) {
	  final String escaped = escapes[c];
	  if (escaped != null) {
	    if (start != off)
	      writer.write(ch, start, off - start);
	    writer.write(escaped);
	    start = off + 1;
	  }
	}
      }
      if (start != off)
	writer.write(ch, start, off - start);
    }
  }
  
  class ControlCharacterHandler extends CharacterHandler {
    void characters(char[] ch, int off, int len) throws IOException {
      writer.write(ch, off, len);
    }
  }

  class LineControlCharacterHandler extends CharacterHandler {
    void characters(char[] ch, int off, int len) throws IOException {
      int start = off;
      for (; len > 0; off++, len--) {
	if (ch[off] == '\n') {
	  if (start != off)
	    writer.write(ch, start, off - start);
	  writer.write(lineSeparator);
	  start = off + 1;
	}
      }
      if (start != off)
	writer.write(ch, start, off - start);
    }
  }

  private Writer writer;
  private OutputStream out;
  private final char[] chBuf = new char[1];
  private final String lineSeparator = System.getProperty("line.separator");
  private String[] escapes = new String[128];
  private final EscapeCharacterHandler escapeCharacterHandler = new EscapeCharacterHandler();
  private final CharacterHandler ignoreCharacterHandler = new CharacterHandler();
  private final CharacterHandler dataCharacterHandler = new DataCharacterHandler();
  private CharacterHandler controlCharacterHandler;

  private CharacterHandler characterHandler;
  private int depth = 0;
  private CharacterHandler[] characterHandlers = new CharacterHandler[1];

  public NXMLOutputHandler() {
    characterHandler = ignoreCharacterHandler;
    if (!lineSeparator.equals("\n")) {
      escapes['\n'] = lineSeparator;
      controlCharacterHandler = new LineControlCharacterHandler();
    }
    else
      controlCharacterHandler = new ControlCharacterHandler();
  }

  public void setOutputStream(OutputStream out) {
    this.out = out;
  }

    public void startDocument() { }

  public void characters(char[] ch, int off, int len) throws SAXException {
    if (writer == null)
      writer = new BufferedWriter(new OutputStreamWriter(out));
    try {
      characterHandler.characters(ch, off, len);
    }
    catch (IOException e) {
      throw new SAXException(e);
    }
  }

  public void ignorableWhitespace(char[] ch, int off, int len) throws SAXException {
    characters(ch, off, len);
  }

  public void startElement(String name, AttributeList atts) throws SAXException {
    if (name.equals("control"))
      push(controlCharacterHandler);
    else if (name.equals("data"))
      push(dataCharacterHandler);
    else if (name.equals("escape")) {
      escapeCharacterHandler.setChar(atts.getValue("char"));
      push(escapeCharacterHandler);
    }
    else if (name.equals("char")) {
      String num = atts.getValue("number");
      if (num != null) {
	try {
	  int c = Integer.parseInt(num);
	  if (c >= 0 && c < 65536) {
	      writer.write(c);
	      //chBuf[0] = (char)c;
	      //characters(chBuf, 0, 1);
	  }
	}
	catch (NumberFormatException e) { }
	catch (IOException e) {
	    throw new SAXException(e);
	}
      }
    }
    else if (name.equals("nxml")) {
      String encoding = atts.getValue("encoding");
      if (encoding != null && writer == null) {
	try {
	  writer = new BufferedWriter(new OutputStreamWriter(out, encoding));
	}
	catch (UnsupportedEncodingException e) {
	  throw new SAXException("unsupported encoding");
	}
      }
    }
  }

  public void endElement(String name) throws SAXException {
    if (name.equals("control") || name.equals("data"))
      pop();
    else if (name.equals("escape")) {
      escapeCharacterHandler.endChar();
      pop();
    }
  }

  public void pop() {
    characterHandler = characterHandlers[--depth];
  }

  public void push(CharacterHandler handler) {
    if (depth >= characterHandlers.length) {
      CharacterHandler[] oldHandlers = characterHandlers;
      characterHandlers = new CharacterHandler[oldHandlers.length * 2];
      System.arraycopy(oldHandlers, 0, characterHandlers, 0, oldHandlers.length);
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
