ifeq ($(HOSTNAME),speedy)
OZLIB	= /home/denys/Mozart/ozlib
else
OZLIB	= /home/duchier/ozlib
endif
JAR	= $(OZLIB)/OzMake2/XT.jar
STYLE	= $(OZLIB)/OzMake2/ozmake.xsl

.PHONY: doc
doc: index-0.2.1.html
index.html: index.xml $(STYLE)
	case `java -version 2>&1` in \
	Kaffe*) JAVACP=-addclasspath;; \
	*) JAVACP=-classpath;; \
	esac; \
	java $$JAVACP $(JAR) com.jclark.xsl.sax.Driver index.xml $(STYLE) index-0.2.1.html
