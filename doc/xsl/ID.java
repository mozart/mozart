import java.util.Hashtable;
import com.jclark.xsl.om.NodeIterator;

public class ID {
    private static Hashtable map = new Hashtable();
    public static boolean put(String id,NodeIterator n) {
        map.put(id,n);
        return false;
    }
    public static NodeIterator get(String id) {
        return (NodeIterator) map.get(id);
    }
}
