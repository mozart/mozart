import java.util.Hashtable;
import com.jclark.xsl.sax.ResultTreeFragment;
import com.jclark.xsl.expr.*;
import com.jclark.xsl.om.*;

public class ID {
    private static Hashtable map = new Hashtable();
    public static boolean put(String id,NodeIterator n) throws XSLException {
	map.put(id,n.next());
	return true;
    }
    public static Node get(String id) {
	return (Node) map.get(id);
    }
    public static boolean putResult(String id,ResultTreeFragment n) {
	map.put(id,n);
	return true;
    }
    public static ResultTreeFragment getResult(String id) {
	return (ResultTreeFragment) map.get(id);
    }
}
