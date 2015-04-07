

 
all: $(OBJ_FILES)
	@echo -e "building: $(notdir $^) \n\t please wait ...\n"
	$(AR) x $(EVENTCORESLIBNAME)
	$(AR) x $(MEDIASDKSLIBNAME)
	$(AR) x $(ISILSDKSLIBNAME)
	$(AR) cru $(ISILMEDIASLIBNAME) *.o
	$(RANLIB) $(ISILMEDIASLIBNAME)
	-rm -f *.o $(EVENTCORESLIBNAME) $(MEDIASDKSLIBNAME) $(ISILSDKSLIBNAME)
	
.PHONY:clean
clean:
	-rm -f *.a *.o
	
	
	
