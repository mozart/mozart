import java.util.Hashtable;

public class Meta {
    private static Hashtable table_map = new Hashtable();
    public static boolean latexTableSpecPut(String id,String spec) {
        table_map.put(id.toUpperCase(),spec);
        return true;
    }
    public static String latexTableSpecGet(String id) {
        return (String) table_map.get(id);
    }
    public static boolean latexTableSpecExists(String id) {
        return table_map.containsKey(id);
    }
}
