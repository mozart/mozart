JAR	= /home/denys/Mozart/ozlib/OzMake2/XT.jar
STYLE	= /home/denys/Mozart/ozlib/OzMake2/ozmake.xsl

doc: index.xml $(STYLE)
	case `java -version 2>&1` in \
	Kaffe*) JAVACP=-addclasspath;; \
	*) JAVACP=-classpath;; \
	esac; \
	java $$JAVACP $(JAR) com.jclark.xsl.sax.Driver index.xml $(STYLE) index.html
