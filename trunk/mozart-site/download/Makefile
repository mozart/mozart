# convenience, when I edit the site remotely
REMOTE_DIR = duchier@grizzly.ps.uni-sb.de:/services/mozart/httpd/html/download

install.py:
	scp view.py $(REMOTE_DIR)/
install:
	scp view.cgi view.py view.css $(REMOTE_DIR)/
install.cgi:
	scp view.cgi $(REMOTE_DIR)/
install.css:
	scp view.css $(REMOTE_DIR)/
