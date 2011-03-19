include ../MODULE_CONFIG

CXX_CFLAGS 		=
CXX_LDFLAGS 	=
LIBRARIES 		= $(CXX_SYSLIBS)

MODULE_NAME		= Znc
OBJECTS			= $(MODULE_NAME).o
OUTPUT			= $(MODULE_NAME).so

default: $(OUTPUT)
Debug: all
Release: all
all: default


$(OUTPUT): $(SRC_DIR)$(OBJECTS)
	$(CXX_LINK) -o $(LIB_DIR)$@ $(OBJECTS) $(LIBRARIES)

update:
	git pull

cleanDebug: clean
cleanRelease: clean
clean:
	rm -f $(OBJECTS) $(LIB_DIR)$(OUTPUT)

cleanall: clean
