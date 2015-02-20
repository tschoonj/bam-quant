bamdir = $(datadir)/bam-quant

dist_bam_DATA = bam-quant-app1.dtd \
		bam-quant-rxi.dtd
bam_DATA = bam-quant-cat.xml

bam-quant-cat.xml:
	$(XMLCATALOG) --noout --create bam-quant-cat.xml 
	$(XMLCATALOG) --noout --add rewriteURI \
	"http://www.bam.de/xml/" \
	"file:///$(bamdir)/" bam-quant-cat.xml

clean-local:
	rm -rf bam-quant-cat.xml
