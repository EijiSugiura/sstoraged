TARGETS = sstoraged.html sstoraged.conf.html
man5_MANS = sstoraged.5 sstoraged.conf.5

all: $(TARGETS)

SUFFIXIES = .html .pod

.pod.html:
	pod2html --css=sstorage.css --infile=$< --outfile=$@

.pod.5:
	pod2man --section=5 --center="Software Storage" \
	--release="Software Storage" $< $@

clean-local :
	rm -f $(TARGETS) *~ $(man5_MANS) *.tmp

