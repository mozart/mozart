// Copyright © by Denys Duchier, Aug 1999, Universität des Saarlandes

import java.util.Hashtable;
import com.jclark.xsl.*;
import org.xml.sax.*;
import com.jclark.xsl.expr.*;
import com.jclark.xsl.om.*;

public class Key {
    private static Hashtable table = new Hashtable();
    private static boolean putOK = true;
    static class EntryIterator extends CloneableNodeIteratorImpl
    {
	EntryIterator(NodeIterator iter) { super(iter); }
    }
    public static boolean put(String key,String val,NodeIterator iter)
	throws SAXException, XSLException
    {
	Hashtable tbl = (Hashtable) table.get(key);
	if (tbl==null) {
	    tbl = new Hashtable();
	    table.put(key,tbl);
	}
	NodeIterator entry = (NodeIterator) tbl.get(val);
	if (entry==null) tbl.put(val,iter);
	else if (!putOK)
	    throw new SAXException("Key: you cannot put after get'ting once");
	else tbl.put(val,new UnionNodeIterator(entry,iter));
	return true;
    }
    static NodeIterator nullIterator = new NullNodeIterator();
    public static NodeIterator get(String key,String val)
    {
	if (putOK) putOK = false;
	Hashtable tbl = (Hashtable) table.get(key);
	if (tbl==null) return nullIterator;
	NodeIterator entry = (NodeIterator) tbl.get(val);
	if (entry==null) return nullIterator;
	if (!(entry instanceof EntryIterator)) {
	    entry = new EntryIterator(entry);
	    tbl.put(val,entry);
	}
	return (NodeIterator) ((EntryIterator)entry).clone();
    }
}
