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

    private static Hashtable mode_map = new Hashtable();
    public static boolean emacsModePut(String mode,String elisp) {
	mode_map.put(mode.toUpperCase(),elisp);
	return true;
    }
    public static String emacsModeGet(String mode) {
	if (mode_map.containsKey(mode))
	    return (String) mode_map.get(mode);
	else
	    return mode.toLowerCase();
    }
    private static Hashtable picwid_map = new Hashtable();
    public static boolean pictureWidthPut(String id,String wd) {
	picwid_map.put(id.toUpperCase(),wd);
	return true;
    }
    public static boolean pictureWidthExists(String id) {
	return picwid_map.containsKey(id.toUpperCase());
    }
    public static String pictureWidthGet(String id) {
	return (String) picwid_map.get(id.toUpperCase());
    }
  private static Hashtable category_map = new Hashtable();
  public static boolean entryCategoryPut(String cls,String nam) {
    category_map.put(cls.toUpperCase(),nam);
    return true;
  }
  public static boolean entryCategoryExists(String cls) {
    return category_map.containsKey(cls.toUpperCase());
  }
  public static String entryCategoryGet(String cls) {
    return (String) category_map.get(cls.toUpperCase());
  }
  public static boolean equal(String s1,String s2) {
    return s1.compareToIgnoreCase(s2)==0;
  }
  static String empty = "";
  private static Hashtable fullwidth_map = new Hashtable();
  public static boolean fullwidthPut(String id) {
    fullwidth_map.put(id.toUpperCase(),empty);
    return true;
  }
  public static boolean fullwidthGet(String id) {
    return fullwidth_map.containsKey(id.toUpperCase());
  }
  private static Hashtable chunk_map = new Hashtable();
  public static class ChunkInfo {
    String s;
    int i=0;
    public ChunkInfo(String s) { this.s=s; }
  }
  public static boolean chunkExists(String title) {
    return chunk_map.containsKey(title);
  }
  public static boolean chunkRegister(String id,String title) {
    ChunkInfo info = new ChunkInfo(id);
    chunk_map.put(title,info);
    return true;
  }
  public static String chunkGetID(String title) {
    return ((ChunkInfo)chunk_map.get(title)).s;
  }
  public static String chunkGetNUMInc(String title) {
    ChunkInfo info =  (ChunkInfo) chunk_map.get(title);
    info.i += 1;
    return String.valueOf(info.i);
  }
}
