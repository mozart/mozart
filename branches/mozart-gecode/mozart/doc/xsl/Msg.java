public class Msg {
    public static boolean say(String msg) {
	System.err.print(msg);
	return true;
    }
    public static boolean saynl(String msg) {
	System.err.println(msg);
	return true;
    }
    public static boolean nl() {
	System.err.println("");
	return true;
    }
}
