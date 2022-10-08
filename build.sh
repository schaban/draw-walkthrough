#!/bin/sh

PROG_NAME=draw_prog
SYS_NAME="`uname -s`"

PROG_DIR=prog
CORE_DIR=core
DATA_DIR=data

INC_DIR=inc
SRC_DIR=src

SHADERS_SRC_DIR=$SRC_DIR/shaders
SHADERS_TGT_DIR=$DATA_DIR/simple_ogl

PROG_PATH=$PROG_DIR/$PROG_NAME
RUN_PATH=run.sh

if [ "$SYS_NAME" = "FreeBSD" ]; then
	NO_FMT="\0"
fi

FMT_BOLD=${NO_FMT:-"\e[1m"}
FMT_UNDER=${NO_FMT:-"\e[4m"}
FMT_BLACK=${NO_FMT:-"\e[30m"}
FMT_BLACK_BG=${NO_FMT:-"\e[30m"}
FMT_RED=${NO_FMT:-"\e[31m"}
FMT_RED_BG=${NO_FMT:-"\e[41m"}
FMT_GREEN=${NO_FMT:-"\e[32m"}
FMT_GREEN_BG=${NO_FMT:-"\e[42m"}
FMT_YELLOW=${NO_FMT:-"\e[33m"}
FMT_YELLOW_BG=${NO_FMT:-"\e[43m"}
FMT_BLUE=${NO_FMT:-"\e[34m"}
FMT_BLUE_BG=${NO_FMT:-"\e[44m"}
FMT_MAGENTA=${NO_FMT:-"\e[35m"}
FMT_MAGENTA_BG=${NO_FMT:-"\e[45m"}
FMT_CYAN=${NO_FMT:-"\e[36m"}
FMT_CYAN_BG=${NO_FMT:-"\e[46m"}
FMT_WHITE=${NO_FMT:-"\e[37m"}
FMT_WHITE_BG=${NO_FMT:-"\e[47m"}
FMT_GRAY=${NO_FMT:-"\e[90m"}
FMT_GRAY_BG=${NO_FMT:-"\e[100m"}
FMT_B_RED=${NO_FMT:-"\e[91m"}
FMT_B_RED_BG=${NO_FMT:-"\e[101m"}
FMT_B_GREEN=${NO_FMT:-"\e[92m"}
FMT_B_GREEN_BG=${NO_FMT:-"\e[102m"}
FMT_B_YELLOW=${NO_FMT:-"\e[93m"}
FMT_B_YELLOW_BG=${NO_FMT:-"\e[103m"}
FMT_B_BLUE=${NO_FMT:-"\e[94m"}
FMT_B_BLUE_BG=${NO_FMT:-"\e[104m"}
FMT_B_MAGENTA=${NO_FMT:-"\e[95m"}
FMT_B_MAGENTA_BG=${NO_FMT:-"\e[105m"}
FMT_B_CYAN=${NO_FMT:-"\e[96m"}
FMT_B_CYAN_BG=${NO_FMT:-"\e[106m"}
FMT_B_WHITE=${NO_FMT:-"\e[97m"}
FMT_B_WHITE_BG=${NO_FMT:-"\e[107m"}
FMT_OFF=${NO_FMT:-"\e[0m"}

printf "$FMT_MAGENTA$FMT_BOLD""/------------------------------------------\\ ""$FMT_OFF\n"
printf " ""$FMT_BLUE_BG$FMT_B_YELLOW"" .: Compiling draw-walkthrough project :. ""$FMT_OFF\n"
printf "$FMT_MAGENTA$FMT_BOLD""\\------------------------------------------/ ""$FMT_OFF\n"

if [ ! -d $PROG_DIR ]; then
	mkdir -p $PROG_DIR
fi

CORE_SRC_URL="https://raw.githubusercontent.com/schaban/crosscore_dev/main/src"
CORE_SRCS="crosscore.hpp crosscore.cpp demo.hpp demo.cpp draw.hpp oglsys.hpp oglsys.cpp oglsys.inc scene.hpp scene.cpp smprig.hpp smprig.cpp draw_ogl.cpp main.cpp"
CORE_OGL_SRCS="gpu_defs.h progs.inc shaders.inc"

DL_MODE="NONE"
DL_CMD=""

if [ -x "`command -v curl`" ]; then
	DL_MODE="CURL"
	DL_CMD="curl -o"
elif [ -x "`command -v wget`" ]; then
	DL_MODE="WGET"
	DL_CMD="wget -O"
fi

NEED_CORE=0
if [ ! -d $CORE_DIR ]; then
	mkdir -p $CORE_DIR
	mkdir -p $CORE_DIR/ogl
	NEED_CORE=1
else
	for src in $CORE_SRCS; do
		if [ $NEED_CORE -ne 1 ]; then
			if [ ! -f $CORE_DIR/$src ]; then
				NEED_CORE=1
			fi
		fi
	done
	if [ ! -d $CORE_DIR/ogl ]; then
		mkdir -p $CORE_DIR/ogl
		NEED_CORE=1
	fi
	if [ $NEED_CORE -ne 1 ]; then
		for src in $CORE_OGL_SRCS; do
			if [ ! -f $CORE_DIR/ogl/$src ]; then
				NEED_CORE=1
			fi
		done
	fi
fi

if [ $NEED_CORE -ne 0 ]; then
	printf "$FMT_B_RED""-> Downloading crosscore sources...""$FMT_OFF\n"
	for src in $CORE_SRCS; do
		if [ ! -f $CORE_DIR/$src ]; then
			printf "$FMT_B_BLUE""     $src""$FMT_OFF\n"
			$DL_CMD $CORE_DIR/$src $CORE_SRC_URL/$src
		fi
	done
	for src in $CORE_OGL_SRCS; do
		if [ ! -f $CORE_DIR/ogl/$src ]; then
			printf "$FMT_CYAN""     $src""$FMT_OFF\n"
			$DL_CMD $CORE_DIR/ogl/$src $CORE_SRC_URL/ogl/$src
		fi
	done
fi

NEED_OGL_INC=0
case $SYS_NAME in
	Linux)
		NEED_OGL_INC=1
	;;
esac

if [ $NEED_OGL_INC -ne 0 ]; then
	KHR_REG_URL=https://registry.khronos.org
	INC_OGL=$INC_DIR/GL
	if [ ! -d $INC_OGL ]; then
		mkdir -p $INC_OGL
	fi
	OGL_INC_DL=0
	for kh in glext.h glcorearb.h; do
		if [ ! -f $INC_OGL/$kh ]; then
			if [ $OGL_INC_DL -ne 1 ]; then
				printf "$FMT_OFF""-> Downloading OpenGL headers...\n"
				OGL_INC_DL=1
			fi
			printf "$FMT_B_GREEN""     $kh""$FMT_OFF\n"
			$DL_CMD $INC_OGL/$kh $KHR_REG_URL/OpenGL/api/GL/$kh
		fi
	done
	INC_KHR=$INC_DIR/KHR
	if [ ! -d $INC_KHR ]; then
		mkdir -p $INC_KHR
	fi
	for kh in khrplatform.h; do
		if [ ! -f $INC_KHR/$kh ]; then
			if [ $OGL_INC_DL -ne 1 ]; then
				printf "$FMT_OFF""-> Downloading OpenGL headers...\n"
				OGL_INC_DL=1
			fi
			printf "$FMT_B_GREEN""     $kh""$FMT_OFF\n"
			$DL_CMD $INC_KHR/$kh $KHR_REG_URL/EGL/api/KHR/$kh
		fi
	done
fi

INCS="-I $CORE_DIR -I $INC_DIR "
SRCS="`ls $SRC_DIR/*.cpp` `ls $CORE_DIR/*.cpp`"

DEFS="-DX11"
LIBS="-lX11"

DEF_CXX="g++"
case $SYS_NAME in
	OpenBSD)
		INCS="$INCS -I/usr/X11R6/include"
		LIBS="$LIBS -L/usr/X11R6/lib"
		DEF_CXX="clang++"
	;;
	FreeBSD)
		INCS="$INCS -I/usr/local/include"
		LIBS="$LIBS -L/usr/local/lib"
		DEF_CXX="clang++"
	;;
	Linux)
		LIBS="$LIBS -ldl"
	;;
esac
CXX=${CXX:-$DEF_CXX}

printf "Compiling \"$FMT_BOLD$FMT_B_MAGENTA$PROG_PATH$FMT_OFF\" with $FMT_BOLD$CXX$FMT_OFF.\n"
rm -f $PROG_PATH
rm -f $RUN_PATH
$CXX -std=c++11 -ggdb -pthread $DEFS $INCS $SRCS -o $PROG_PATH $LIBS $*
if [ -f "$PROG_PATH" ]; then
	if [ ! -d $SHADERS_TGT_DIR ]; then
		mkdir -p $SHADERS_TGT_DIR
	fi
	printf "$FMT_OFF""Copying GLSL code:\n""$FMT_GRAY"
	for glsl in symbol.vert symbol.frag batch_static.vert batch_deform.vert batch.frag; do
		cp -v $SHADERS_SRC_DIR/$glsl $SHADERS_TGT_DIR/$glsl
	done
	printf "$FMT_OFF""Done!\n"
	printf "#!/bin/sh\n\n" > $RUN_PATH
	printf "./$PROG_PATH -nwrk:0 \$*" >> $RUN_PATH
	chmod +x $RUN_PATH
	printf "$FMT_B_GREEN""Success""$FMT_OFF""$FMT_BOLD""!!""$FMT_OFF\n"
else
	printf "$FMT_B_RED""Failure""$FMT_OFF...\n"
fi

