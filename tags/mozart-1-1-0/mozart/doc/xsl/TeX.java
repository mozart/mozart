public class TeX {

    public static String quote(String s) {
	StringBuffer b = new StringBuffer();
	for(int i=0; i<s.length(); i++) {
	    char c = s.charAt(i);
	    if (c=='%'  || c=='#' || c=='&' ||
		c=='\\' || c=='{' || c=='}' ||
		c=='$'  || c=='_' || c=='^' ||
		c=='|'  ) {
		b.append("\\ZCHAR{\\");
		b.append(c);
		b.append("}");
	    } else {
		b.append(c);
	    }
	}
	return b.toString();
    }

    public static String noeol(String s) {
	StringBuffer b = new StringBuffer();
	for(int i=0; i<s.length(); i++) {
	    char c = s.charAt(i);
	    if (c=='\n') { b.append(' '); }
	    else { b.append(c); }
	}
	return b.toString();
    }

}
