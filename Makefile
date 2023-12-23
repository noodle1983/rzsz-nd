PROJBASE=$(CURDIR)

#########################################
#                Target                 #
#########################################
SUBDIR= fsm zmodem io osc base64 lz \
		ftxui/ \
		ftxui/src/ftxui/screen/ \
		ftxui/src/ftxui/dom/ \
		ftxui/src/ftxui/component/ \
		fmt/src/ \


#########################################
#      header and lib dependancy        #
#########################################
INC_DIR= -I. \
     -I$(PROJBASE)/fsm \
     -I$(PROJBASE)/zmodem \
     -I$(PROJBASE)/osc \
     -I$(PROJBASE)/io \
     -I$(PROJBASE)/idl \
     -I$(PROJBASE)/base64 \
     -I$(PROJBASE)/lz \
     -I$(PROJBASE)/ftxui/ \
     -I$(PROJBASE)/ftxui/include \
     -I$(PROJBASE)/ftxui/src \
     -I$(PROJBASE)/fmt/include \
     -I$(PROJBASE)/fmt/src \

STATIC_LIB= \

SHARED_LIB_DIR=
SHARED_LIB=

#########################################
#              compiler                 #
#########################################
include $(PROJBASE)/build/makefile.compiler

#########################################
#              rules                    #
#########################################
include $(PROJBASE)/build/makefile.compile.rules

