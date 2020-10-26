PROJBASE=$(CURDIR)

#########################################
#                Target                 #
#########################################
SUBDIR= fsm \
    zmodem \

#########################################
#      header and lib dependancy        #
#########################################
INC_DIR= -I. \
     -I$(PROJBASE)/fsm \
     -I$(PROJBASE)/zmodem \

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

