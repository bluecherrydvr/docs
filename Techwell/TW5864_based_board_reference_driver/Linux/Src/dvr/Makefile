sinclude	./rules.make

export


SUBDIRS = $(ISILSDKDIRNAME) $(FILESDKDIRNAME) $(NETSDKDIRNAME) $(NETAPPDIRNAME) 
                    
		    



GLABALCOMPILE = yes

	
.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -w -C $@
	

	
.PHONY: clean
clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean;done

