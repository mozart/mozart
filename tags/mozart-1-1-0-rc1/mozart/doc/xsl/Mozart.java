// Copyright © by Denys Duchier, Aug 1999, Universität des Saarlandes

import java.util.Hashtable;
import com.jclark.xsl.*;
import org.xml.sax.*;
import com.jclark.xsl.expr.*;
import com.jclark.xsl.om.*;

public class Mozart {

    static void lerror(String msg) throws SAXException
    {
	 throw new SAXException(msg);
    }

    // map[key][val] --> node

    private static Hashtable map = new Hashtable();
    public static boolean haskey(String key)
    {
	return (map.get(key)!=null);
    }
    public static boolean hasentry(String key,String val)
    {
	Hashtable tbl = (Hashtable) map.get(key);
	return (tbl!=null) && tbl.containsKey(val);
    }
    public static boolean put(String key,String val,NodeIterator iter)
	throws SAXException, XSLException
    {
	Hashtable tbl = (Hashtable) map.get(key);
	if (tbl==null) {
	    tbl = new Hashtable();
	    map.put(key,tbl);
	}
	Node node = (Node) tbl.get(val);
	if (node!=null) lerror("*** key/val clash: sorry!!! key="+key+" val="+val);
	node = iter.next();
	tbl.put(val,node);
	return true;
    }
    public static Node get(String key,String val)
	throws SAXException
    {
	Hashtable tbl = (Hashtable) map.get(key);
	if (tbl==null) lerror("no such key: "+key);
	Node node = (Node) tbl.get(val);
	if (node==null) lerror("no such key/val: key="+key+" val="+val);
	return node;
    }

    // debug and error messages

    public static boolean say(String s)
    {
	System.err.print(s);
	return true;
    }
    public static boolean saynl(String s)
    {
	System.err.println(s);
	return true;
    }
    public static boolean nl()
    {
	System.err.println();
	return true;
    }
    public static boolean error(String s)
	throws SAXException
    {
	saynl(s);
	throw new SAXException("Mozart Error");
    }

    // smap[key] --> string

    private static Hashtable smap = new Hashtable();
    public static boolean shaskey(String key)
    {
	return (smap.get(key)!=null);
    }
    public static boolean sset(String key,String val)
    {
	smap.put(key,val);
	return true;
    }
    public static boolean sput(String key,String val)
	throws SAXException
    {
	if (smap.get(key)!=null) lerror("*** skey class!!! skey="+key+" val="+val);
	return sset(key,val);
    }
    public static String sget(String key)
	throws SAXException
    {
	String val = (String) smap.get(key);
	if (val==null) lerror("no such skey: "+key);
	return val;
    }

    // ssmap[key][val] --> string

    private static Hashtable ssmap = new Hashtable();
    public static boolean sshaskey(String key)
    {
	return ssmap.get(key)!=null;
    }
    public static boolean sshaskeyval(String key,String val)
    {
	Hashtable tbl = (Hashtable) ssmap.get(key);
	return (tbl!=null) && tbl.containsKey(val);
    }
    public static boolean ssput(String key,String arg1,String arg2)
	throws SAXException
    {
	Hashtable tbl = (Hashtable) ssmap.get(key);
	if (tbl==null) {
	    tbl = new Hashtable();
	    ssmap.put(key,tbl);
	}
	if (tbl.containsKey(arg1)) lerror("ssput: key/arg1="+key+"/"+arg1);
	tbl.put(arg1,arg2);
	return true;
    }
    public static String ssget(String key,String arg1)
	throws SAXException
    {
	Hashtable tbl = (Hashtable) ssmap.get(key);
	if (tbl==null) lerror("ssmap: no key="+key);
	String arg2 = (String) tbl.get(arg1);
	if (arg2==null) lerror("ssmap: no key/val="+key+"/"+arg1);
	return arg2;
    }
}
