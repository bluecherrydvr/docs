ifeq ($(GLABALCOMPILE),yes)
	override TOPDIR := ./../
	sinclude	$(TOPDIR)rules.make	
else
	sinclude	./rules.make
endif

export



SUBDIRS = src 
                    
		    
		  


#GLABALCOMPILE = yes

#display:
#	@echo $(TOPHPATH)
#	@echo $(TOPLPATH)
#	@echo $(TOPBINPATH)
#	@echo $(LIBPATH)
#	@echo $(CC)
#	@echo $(AR)
#	@echo $(LINK)
#	@echo $(LIBS)
#	@echo $(LIBPATH)
#	@echo $(INCPATH)
#	@echo $(CFLAGS)
#	@echo $(DVMLOGINC)
#	@echo $(DVMBUFFINC)
#	@echo $(DVMEVENTINC)
#	@echo $(DVMIOINC)
#	@echo $(DVMAVHANDLEINC)
#	@echo $(DVMSBASEINC)
#	@echo $(DVMTIMERINC)
#	@echo $(DVMSDKPATH)
#	@echo $(DVMSERVEPATH)
#	@echo $(DVMFILESERVEPATH)
#	@echo $(DVMNETSERVEPATH)
#	@echo $(DVMDEVICESERVERPATH)
#	@echo $(DVMALARMSERVERPATH)
#	@echo $(DVMFILESDKPATH)
#	@echo $(DVMNETSDKPATH)
#	@echo $(DVMNETSDKINC)
#	@echo $(DVMPELSDKPATH)
#	@echo $(DVMPELSDKINC)
	
.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
	@echo -e "CC : $(CC) \n\t"
	$(MAKE) -w -C $@
	

	
.PHONY: clean
clean:
	@rm -f ./bin/*
	@rm -f ./lib/*
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean;done

