PROJBASE=$(CURDIR)

#########################################
#                Target                 #
#########################################
SUBDIR= fsm zmodem io

#########################################
#      header and lib dependancy        #
#########################################
INC_DIR= -I. \
     -I$(PROJBASE)/fsm \
     -I$(PROJBASE)/zmodem \
     -I$(PROJBASE)/io \

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

